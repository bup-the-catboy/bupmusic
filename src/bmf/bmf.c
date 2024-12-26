#include "bmf.h"
#include "bmf_internal.h"

#include <string.h>
#include <stdlib.h>

bmf* bmf_create() {
    bmf* data = malloc(sizeof(bmf));
    data->size = sizeof(bmf);
    data->master_volume = 1;
    data->tempo = 1;
    for (int i = 0; i < 256; i++) {
        data->channel_volumes[i] = 0;
    }
    memset(data->sample_buffer, 0, sizeof(data->sample_buffer));
    data->curr_song = -1;
    data->num_songs = 0;
    data->num_instruments = 0;
    data->sample_buffer_ptr = 0;
    data->songs = malloc(1);
    data->instruments = malloc(1);
    return data;
}

uint8_t bmf_num_songs(bmf* data) {
    return data->num_songs;
}

uint8_t bmf_num_instruments(bmf* data) {
    return data->num_instruments;
}

uint8_t bmf_curr_song_num_channels(bmf* data) {
    if (data->curr_song == -1) return 0;
    return data->songs[data->curr_song].channels;
}

uint8_t bmf_current_song(bmf* data) {
    return data->curr_song;
}

float bmf_current_tempo(bmf* data) {
    return data->tempo;
}

float bmf_current_time(bmf* data) {
    return data->frame / 60.f;
}

float bmf_get_master_volume(bmf* data) {
    return data->master_volume;
}

float bmf_get_channel_volume(bmf* data, uint8_t channel) {
    if (data->curr_song == -1) return 0;
    if (channel < 0 || channel >= data->songs[data->curr_song].channels) return 0;
    return data->channel_volumes[channel];
}

void bmf_select_song(bmf* data, uint8_t song) {
    if (song < 0 || song >= data->num_songs) song = -1;
    data->curr_song = song;
    data->frame = 0;
}

void bmf_seek(bmf* data, float secs) {
    data->frame = secs * FPS;
}

void bmf_tempo(bmf* data, float tempo) {
    data->tempo = tempo;
}

void bmf_master_volume(bmf* data, float volume) {
    if (volume < 0) volume = 0;
    if (volume > 1) volume = 1;
    data->master_volume = volume;
}

void bmf_channel_volume(bmf* data, float volume, uint8_t channel) {
    if (data->curr_song == -1) return;
    if (channel < 0 || channel >= data->songs[data->curr_song].channels) return;
    if (volume < 0) volume = 0;
    if (volume > 1) volume = 1;
    data->channel_volumes[channel] = volume;
}

void bmf_play(bmf* data, short* samples_out, int num_out) {
    if (data->curr_song == -1) return;
    int remaining = num_out;
    while (remaining != 0) {
        // check if there are any uncopied samples from the previous frame
        int frame_remaining = SAMPLE_BUFSIZ - data->sample_buffer_ptr;
        if (frame_remaining > remaining) frame_remaining = remaining;

        // copy them
        memcpy(samples_out, data->sample_buffer + data->sample_buffer_ptr, frame_remaining * sizeof(short));
        data->sample_buffer_ptr += frame_remaining;
        remaining -= frame_remaining;

        // nothing to do
        if (remaining == 0) break;

        // synth next frame
        data->sample_buffer_ptr = 0;
        short channel_out[SAMPLE_BUFSIZ];
        memset(data->sample_buffer, 0, sizeof(data->sample_buffer));
        for (int i = 0; i < data->songs[data->curr_song].channels; i++) {
            bmf_synth_frame(&data->songs[data->curr_song], channel_out, &data->timer, data->frame, data->tempo, i);
            bmf_volume(channel_out, data->channel_volumes[i], SAMPLE_BUFSIZ);
            bmf_mix(data->sample_buffer, channel_out, SAMPLE_BUFSIZ);
        }
        bmf_volume(data->sample_buffer, data->master_volume, SAMPLE_BUFSIZ);
        data->frame += data->tempo;
    }
}

void bmf_destroy(bmf* data) {
    for (int i = 0; i < data->num_songs; i++) {
        for (int j = 0; j < data->songs[i].patterns; j++) {
            free(data->songs[i].pattern_data[j].notes);
        }
        free(data->songs[i].channel_table);
        free(data->songs[i].pattern_table);
        free(data->songs[i].pattern_data);
    }
    free(data->songs);
    free(data->instruments);
    free(data);
}

void* bmf_get(void* bmf_struct_array, int index) {
    int size = *(int*)bmf_struct_array;
    return (char*)bmf_struct_array + index * size;
}