#include "bmf_internal.h"

#include <stdlib.h>
#include <stdint.h>

#define read(type) ({              \
    type x;                         \
    bin_read(bin, &x, sizeof(type)); \
    x;                                \
})

bmf* bmf_load(const char* data, int length) {
    bmf* bmf = malloc(sizeof(*bmf));
    bmf_binary* bin = bin_wrap((char*)data, length);
    bmf->size = sizeof(*bmf);
    bmf->num_songs       = read(uint8_t);
    bmf->num_instruments = read(uint8_t);
    bmf->songs           = calloc(sizeof(bmf_song),       bmf->num_songs);
    bmf->instruments     = calloc(sizeof(bmf_instrument), bmf->num_instruments);
    for (int i = 0; i < bmf->num_songs; i++) {
        bmf_song* song = &bmf->songs[i];
        song->size          = sizeof(bmf_song);
        song->length        = read(uint8_t);
        song->loop          = read(uint8_t);
        song->bpm           = read(uint8_t);
        song->channels      = read(uint8_t);
        song->patterns      = read(uint8_t);
        song->pattern_table = calloc(song->patterns, song->channels);
        song->pattern_data  = calloc(song->patterns, sizeof(bmf_pattern));
        song->channel_table = calloc(song->channels, sizeof(bmf_channel_type));
        bin_read(bin, song->pattern_table, song->patterns * song->channels);
        for (int j = 0; j < song->channels; j++) {
            song->channel_table[j] = read(uint8_t);
        }
        for (int j = 0; j < song->patterns; j++) {
            bmf_pattern* pattern = &song->pattern_data[j];
            pattern->num_notes = read(uint8_t);
            pattern->notes     = calloc(pattern->num_notes, sizeof(bmf_note));
            for (int k = 0; k < pattern->num_notes; k++) {
                bmf_note* note = &pattern->notes[k];
                note->size       = sizeof(bmf_note);
                note->type       = read(uint8_t);
                note->start      = read(uint8_t);
                note->length     = read(uint8_t);
                note->instrument = read(uint8_t);
                note->value_from = read(uint16_t);
                note->value_to   = read(uint16_t);
            }
        }
    }
    for (int i = 0; i < bmf->num_instruments; i++) {
        bmf_instrument* instrument = &bmf->instruments[i];
        instrument->size           = sizeof(bmf_instrument);
        instrument->num_duty_cycle = read(uint8_t);
        instrument->num_volume     = read(uint8_t);
        instrument->num_pitch      = read(uint8_t);
        instrument->duty_cycle     = malloc(2 * instrument->num_duty_cycle);
        instrument->volume         = malloc(2 * instrument->num_volume);
        instrument->pitch          = malloc(2 * instrument->num_pitch);
        bin_read(bin, instrument->duty_cycle, instrument->num_duty_cycle);
        bin_read(bin, instrument->volume,     instrument->num_volume);
        bin_read(bin, instrument->pitch,      instrument->num_pitch);
    }
    return bmf;
}