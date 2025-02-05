#include "bmf_internal.h"

#include <stdlib.h>
#include <string.h>

bmf_song* bmf_get_songs(bmf* data) {
    return data->songs;
}

bmf_song* bmf_add_song(bmf* data, uint8_t length, uint8_t bpm) {
    if (data->num_songs == 0xFF) return NULL;
    data->num_songs++;
    data->songs = realloc(data->songs, sizeof(bmf_song) * data->num_songs);
    bmf_song* song = &data->songs[data->num_songs - 1];
    memset(song, 0, sizeof(bmf_song));
    song->size = sizeof(bmf_song);
    song->bpm = bpm;
    song->length = length;
    song->pattern_data  = malloc(1);
    song->channel_table = malloc(1);
    song->pattern_table = malloc(1);
    song->parent = data;
    return song;
}

uint8_t bmf_get_bpm(bmf_song* song) {
    return song->bpm;
}

uint8_t bmf_get_length(bmf_song* song) {
    return song->length;
}

uint8_t bmf_get_loop(bmf_song* song) {
    return song->loop;
}

void bmf_set_bpm(bmf_song* song, uint8_t bpm) {
    song->bpm = bpm;
}

void bmf_set_length(bmf_song* song, uint8_t length) {
    uint8_t* new_pattern_table = malloc(song->channels * length);
    int min_length = song->length < length ? song->length : length;
    for (int i = 0; i < min_length; i++) {
        for (int j = 0; j < song->channels; j++) {
            new_pattern_table[j * length + i] = song->pattern_table[j * song->length + i];
        }
    }
    for (int i = 0; i < (int)length - song->length; i++) {
        for (int j = 0; j < song->channels; j++) {
            new_pattern_table[j * length + i] = 0xFF;
        }
    }
    free(song->pattern_table);
    song->length = length;
    song->pattern_table = new_pattern_table;
}

void bmf_set_loop(bmf_song* song, uint8_t loop) {
    song->loop = loop;
}

void bmf_add_channel(bmf_song* song, bmf_channel_type type) {
    bmf_add_channel_at(song, type, song->channels);
}

void bmf_add_channel_at(bmf_song* song, bmf_channel_type type, uint8_t index) {
    song->channels++;
    uint8_t* new_channel_table = malloc(song->channels);
    uint8_t* new_pattern_table = malloc(song->length * song->channels);
    int ptr = 0;
    for (int i = 0; i < song->channels; i++) {
        if (i == index) {
            new_channel_table[i] = type;
            memset(new_pattern_table + song->length * i, 0, song->length);
        }
        else {
            new_channel_table[i] = song->channel_table[ptr];
            memcpy(new_pattern_table + song->length * i, song->pattern_table + song->length * ptr, song->length);
            ptr++;
        }
    }
    free(song->channel_table);
    free(song->pattern_table);
    song->channel_table = new_channel_table;
    song->pattern_table = new_pattern_table;
    for (int i = 0; i < song->length; i++) {
        bmf_clear_pattern_table_at(song, index, i);
    }
}

uint8_t* bmf_get_pattern_table(bmf_song* song) {
    uint8_t* pattern_table = malloc(song->channels * song->length);
    memcpy(pattern_table, song->pattern_table, song->channels * song->length);
    return pattern_table;
}

uint8_t bmf_get_pattern_table_at(bmf_song* song, uint8_t channel, uint8_t position) {
    if (channel >= song->channels || position >= song->length) return 0xFF;
    return song->pattern_table[channel * song->length + position];;
}

void bmf_set_pattern_table_at(bmf_song* song, uint8_t channel, uint8_t position, uint8_t index) {
    if (channel >= song->channels || position >= song->length) return;
    song->pattern_table[channel * song->length + position] = index;
}

void bmf_clear_pattern_table_at(bmf_song* song, uint8_t channel, uint8_t position) {
    bmf_set_pattern_table_at(song, channel, position, 0xFF);
}

void bmf_set_pattern_table(bmf_song* song, uint8_t* pattern_table) {
    memcpy(song->pattern_table, pattern_table, song->channels * song->length);
}

uint8_t bmf_num_channels(bmf_song* song) {
    return song->channels;
}

uint8_t bmf_num_patterns(bmf_song* song) {
    return song->patterns;
}

bmf_channel_type* bmf_get_channels(bmf_song* song) {
    bmf_channel_type* types = malloc(sizeof(bmf_channel_type) * song->channels);
    for (int i = 0; i < song->channels; i++) {
        types[i] = song->channel_table[i];
    }
    return types;
}

bmf_pattern* bmf_get_pattern(bmf_song* song, uint8_t index) {
    if (index == 0xFF || index >= song->patterns) return NULL;
    return &song->pattern_data[index];
}

void bmf_remove_song(bmf_song* song) {
    bmf_song* new_songs = malloc(sizeof(bmf_song) * (song->parent->num_songs - 1));
    int ptr = 0;
    for (int i = 0; i < song->parent->num_songs; i++) {
        if (song == &song->parent->songs[i]) continue;
        new_songs[ptr++] = song->parent->songs[i];
    }
    free(song->parent->songs);
    song->parent->songs = new_songs;
    free(song->pattern_data);
    free(song->pattern_table);
    free(song->channel_table);
    free(song);
}