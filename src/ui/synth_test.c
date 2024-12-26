#include <SDL2/SDL.h>

#include <pthread.h>

#include "audio.h"
#include "imgui/cimgui.h"
#include "bmf/bmf.h"

static int synth_test_wave_type = 0;
static int synth_test_frequency = 500;
static bool synth_test_playing = false;
static float synth_test_duty_cycle = 0.5;
static FILE* synth_test_file = NULL;
static uint64_t synth_test_timer = 0;
static short* synth_test_frozen_samples = NULL;
static int synth_test_sample_graph_offset = 0;
static int synth_test_sample_graph_count  = 800;

void synth_test() {
    bool frozen_flipped = false;
    igSeparatorText("Synth Settings");
    igCombo_Str("Wave Type", &synth_test_wave_type, "Square\0Triangle\0Sine\0Sawtooth\0Noise\0", 0);
    igSliderInt("Frequency", &synth_test_frequency, 1, 8000, "%d", ImGuiSliderFlags_None);
    igSliderFloat("Duty Cycle", &synth_test_duty_cycle, 0, 1, "%.3f", ImGuiSliderFlags_None);
    igSeparatorText("Controls");
    igBeginDisabled(synth_test_frozen_samples != NULL);
    if (igButton(synth_test_playing ? "Stop##gen_stop" : "Play", (ImVec2){ 0, 0 })) synth_test_playing ^= 1;
    igEndDisabled();
    igSameLine(0, -1);
    igBeginDisabled(!synth_test_playing);
    if (igButton(synth_test_frozen_samples == NULL ? "Pause" : "Unpause", (ImVec2){ 0, 0 })) frozen_flipped = true;
    igEndDisabled();
    igSameLine(0, -1);
    if (igButton(synth_test_file ? "Stop##record_stop" : "Record", (ImVec2){ 0, 0 })) {
        if (synth_test_file) {
            fclose(synth_test_file);
            synth_test_file = NULL;
        }
        else synth_test_file = fopen("synth.pcm", "w");
    }

    if (frozen_flipped && synth_test_frozen_samples) {
        free(synth_test_frozen_samples);
        synth_test_frozen_samples = NULL;
        frozen_flipped = false;
    }

    short* samples_buffer;
    if (synth_test_frozen_samples) samples_buffer = synth_test_frozen_samples;
    else {
        samples_buffer = audio_create_sample_buffer();
        if (synth_test_playing) {
            bmf_play_instrument(NULL, synth_test_wave_type, samples_buffer, audio_num_samples(), synth_test_frequency, synth_test_duty_cycle, 0, &synth_test_timer);
            synth_test_timer++;
        }
        else synth_test_timer = 0;
        if (synth_test_file) fwrite(samples_buffer, sizeof(short), audio_num_samples(), synth_test_file);
    }

    if (frozen_flipped && !synth_test_frozen_samples) {
        synth_test_frozen_samples = malloc(sizeof(short) * audio_num_samples());
        memcpy(synth_test_frozen_samples, samples_buffer, sizeof(short) * audio_num_samples());
        frozen_flipped = false;
    }

    float samples[800];
    for (int i = 0; i < 800; i++) {
        samples[i] = samples_buffer[i * 2] / 65536.f + 0.5;
    }
    igSeparatorText("Sample Graph");
    igSliderInt("Offset", &synth_test_sample_graph_offset, 0, 800 - synth_test_sample_graph_count, "%d", ImGuiSliderFlags_AlwaysClamp);
    igSliderInt("Count",  &synth_test_sample_graph_count,  1, 800,                                 "%d", ImGuiSliderFlags_AlwaysClamp);
    if (synth_test_sample_graph_offset + synth_test_sample_graph_count > 800) synth_test_sample_graph_offset = 800 - synth_test_sample_graph_count;
    igPlotLines_FloatPtr("##sample_graph", samples + synth_test_sample_graph_offset, synth_test_sample_graph_count, 0, "", 0.4, 0.6, (ImVec2){ 256, 64 }, sizeof(float));
}