#include "bmf_internal.h"

#include <stdlib.h>

#define write(type, value) {          \
    type val = value;                  \
    bin_write(bin, &val, sizeof(type)); \
}

char* bmf_save(bmf* bmf, int* outlen) {
    bmf_binary* bin = bin_create();
    write(uint8_t, bmf->num_songs);
    write(uint8_t, bmf->num_instruments);
    for (int i = 0; i < bmf->num_songs; i++) {
        bmf_song* song = &bmf->songs[i];
        write(uint8_t, song->length);
        write(uint8_t, song->loop);
        write(uint8_t, song->bpm);
        write(uint8_t, song->channels);
        write(uint8_t, song->patterns);
        bin_write(bin, song->pattern_table, song->patterns * song->channels);
        for (int j = 0; j < song->channels; j++) {
            write(uint8_t, song->channel_table[j]);
        }
        for (int j = 0; j < song->patterns; j++) {
            bmf_pattern* pattern = &song->pattern_data[j];
            write(uint8_t, pattern->num_notes);
            for (int k = 0; k < pattern->num_notes; k++) {
                bmf_note* note = &pattern->notes[k];
                write(uint8_t,  note->type);
                write(uint8_t,  note->start);
                write(uint8_t,  note->length);
                write(uint8_t,  note->instrument);
                write(uint16_t, note->value_from);
                write(uint16_t, note->value_to);
            }
        }
    }
    for (int i = 0; i < bmf->num_instruments; i++) {
        bmf_instrument* instrument = &bmf->instruments[i];
        write(uint8_t, instrument->num_duty_cycle);
        write(uint8_t, instrument->num_volume);
        write(uint8_t, instrument->num_pitch);
        bin_write(bin, instrument->duty_cycle, instrument->num_duty_cycle);
        bin_write(bin, instrument->volume,     instrument->num_volume);
        bin_write(bin, instrument->pitch,      instrument->num_pitch);
    }
    *outlen = bin->ptr;
    char* out = bin->data;
    free(bin);
    return out;
}