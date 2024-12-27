#include "bmf/bmf.h"
#include "bmf_internal.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct {
    int frame, time;
    bmf_note* note;
} bmf_note_instance;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int interpolate(bmf_note_instance* instance) {
    return (instance->note->value_to - instance->note->value_from) * (instance->time / (float)instance->note->length) + instance->note->value_from;
}

int interpolate_signed(bmf_note_instance* instance) {
    int16_t from = instance->note->value_from;
    int16_t to = instance->note->value_to;
    return (to - from) * (instance->time / (float)instance->note->length) + from;
}

double frac(double x) {
    return x - floor(x);
}

float bmf_sample(bmf_channel_type channel_type, uint16_t frequency, float duty_cycle, uint64_t sample_index) {
    double t = sample_index / (double)SAMPLE_RATE;
    float sample;
    switch (channel_type) {
        case bmf_channel_type_square:
            sample = frac(t * frequency / 2) < duty_cycle ? 0.5f : -0.5f;
            break;
        case bmf_channel_type_triangle:
            sample = fabs(frac(t * frequency / 2) * 2 - 1) * 2 - 1;
            break;
        case bmf_channel_type_sine:
            sample = sin((t * frequency / 2) * 2 * M_PI);
            break;
        case bmf_channel_type_sawtooth:
            sample = (frac(t * frequency / 2) * 2 - 1) * 0.75f;
            break;
        case bmf_channel_type_noise:
            // TODO
            break;
    }
    return sample * 0.2; // on its own its very loud
}

void bmf_play_instrument(bmf_instrument* instrument, bmf_channel_type channel_type, short* samples_out, int num_samples, uint16_t frequency, float duty_cycle, uint8_t frame, uint64_t* timer) {
    float volume = 1;
    int freq = frequency;
    if (instrument) {
        uint8_t idx_volume     = min(frame, instrument->num_volume     - 1);
        uint8_t idx_pitch      = min(frame, instrument->num_pitch      - 1);
        uint8_t idx_duty_cycle = min(frame, instrument->num_duty_cycle - 1);
        volume = instrument->volume[idx_volume] / 65536.f;
        duty_cycle = instrument->duty_cycle[idx_duty_cycle] / 65536.f;
        freq = (int16_t)instrument->pitch[idx_pitch] + frequency;
        if (freq >= 65536) freq = 65535;
    }
    for (int i = 0; i < num_samples; i++) {
        float sample = bmf_sample(channel_type, freq, duty_cycle, (*timer) / freq);
        if (sample < -1) sample = -1;
        if (sample >  1) sample =  1;
        samples_out[i] = sample * volume * 32767;
        (*timer) += freq;
    }
}

void bmf_synth_frame(bmf_song* song, short* buf_out, uint64_t* timer, float frame, float tempo, int channel) {
    bmf_note_instance note_instances[32];
    int num_note_instances = 0; //  vvvvv  60[sec in min] * 60[fps] / 64[units per beat]
    int frame_to_unit = song->bpm / 56.25f;
    int units_per_frame = frame_to_unit / tempo;
    int pattern_pos = (int)(frame * frame_to_unit) / 256; // 256 units per pattern
    int    note_pos = (int)(frame * frame_to_unit) % 256;
    int pattern_idx = song->pattern_table[channel * song->channels + pattern_pos];
    memset(buf_out, 0, sizeof(short) * SAMPLE_BUFSIZ);
    if (pattern_idx == 0xFF) return;
    bmf_pattern* pattern = &song->pattern_data[pattern_idx];
    bmf_note_instance volume_note = (bmf_note_instance){ 0, 0, NULL };
    bmf_note_instance pitch_note  = (bmf_note_instance){ 0, 0, NULL };
    for (int i = 0; i < pattern->num_notes; i++) {
        bmf_note* note = &pattern->notes[i];
        if (note->start >= note_pos && note->start + note->length < note_pos) {
            int time = note_pos - note->start;
            bmf_note_instance instance = (bmf_note_instance){
                .frame = time / units_per_frame,
                .time  = time,
                .note  = note
            };
            switch (note->type) {
                case bmf_note_type_audible:
                    if (num_note_instances == sizeof(note_instances) / sizeof(*note_instances)) break;
                    note_instances[num_note_instances++] = instance;
                    break;
                case bmf_note_type_volume:
                    volume_note = instance;
                    break;
                case bmf_note_type_pitch:
                    pitch_note = instance;
                    break;
            }
        }
    }
    float volume = interpolate(&volume_note) / 65536.f;
    int16_t freq_offset = interpolate_signed(&pitch_note);
    for (int i = 0; i < num_note_instances; i++) {
        short samples[SAMPLE_BUFSIZ / STEREO];
        int frequency = interpolate(&note_instances[i]) + freq_offset;
        if (frequency >= 65536) frequency = 65535;
        for (int j = 0; j < SAMPLE_BUFSIZ / STEREO; j++) {
            int sample = (int)samples[j] * volume + (int)buf_out[j * 2 + 0];
            if (sample < -32768) sample = -32768;
            if (sample >  32767) sample =  32767;
            buf_out[j * 2 + 0] = sample;
            buf_out[j * 2 + 1] = sample;
        }
        bmf_play_instrument(&song->parent->instruments[note_instances[i].note->instrument], song->channel_table[channel], samples, SAMPLE_BUFSIZ / STEREO, frequency, 0.5, note_instances[i].frame, timer);
    }
}

void bmf_volume(short* samples, float volume, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        samples[i] *= volume;
    }
}

void bmf_mix(short* dst, short* src, int samples) {
    for (int i = 0; i < samples; i++) {
        dst[i] += src[i];
    }
}