#include "bmf_internal.h"

#include <stdlib.h>
#include <string.h>

bmf_instrument* bmf_get_instruments(bmf* data) {
    return data->instruments;
}

bmf_instrument* bmf_add_instrument(bmf* data) {
    if (data->num_instruments == 0xFF) return NULL;
    data->num_instruments++;
    data->instruments = realloc(data->instruments, sizeof(bmf_instrument) * data->num_instruments);
    bmf_instrument* instrument = &data->instruments[data->num_instruments - 1];
    memset(instrument, 0, sizeof(bmf_instrument));
    instrument->size = sizeof(bmf_instrument);
    instrument->parent = data;
    instrument->num_volume     = 1;
    instrument->num_pitch      = 1;
    instrument->num_duty_cycle = 1;
    instrument->volume     = malloc(1);
    instrument->pitch      = malloc(1);
    instrument->duty_cycle = malloc(1);
    instrument->volume    [0] = 0xFFFF;
    instrument->pitch     [0] = 0x0000;
    instrument->duty_cycle[0] = 0x8000;
    instrument->parent = data;
    return instrument;
}

uint16_t* bmf_instrument_get_volume(bmf_instrument* instrument, uint8_t* length) {
    if (length) *length = instrument->num_volume;
    return instrument->volume;
}

int16_t* bmf_instrument_get_pitch(bmf_instrument* instrument, uint8_t* length) {
    if (length) *length = instrument->num_pitch;
    return instrument->pitch;
}

uint16_t* bmf_instrument_get_duty_cycle(bmf_instrument* instrument, uint8_t* length) {
    if (length) *length = instrument->num_duty_cycle;
    return instrument->duty_cycle;
}

void bmf_instrument_set_volume_length(bmf_instrument* instrument, uint8_t length) {
    instrument->volume = realloc(instrument->volume, length * 2);
}

void bmf_instrument_set_pitch_length(bmf_instrument* instrument, uint8_t length) {
    instrument->pitch = realloc(instrument->pitch, length * 2);
}

void bmf_instrument_set_duty_cycle_length(bmf_instrument* instrument, uint8_t length) {
    instrument->duty_cycle = realloc(instrument->duty_cycle, length * 2);
}

void bmf_remove_instrument(bmf_instrument* instrument) {
    bmf_instrument* new_instruments = malloc(sizeof(bmf_instrument) * (instrument->parent->num_instruments - 1));
    int ptr = 0;
    for (int i = 0; i < instrument->parent->num_instruments; i++) {
        if (instrument == &instrument->parent->instruments[i]) continue;
        new_instruments[ptr++] = instrument->parent->instruments[i];
    }
    free(instrument->parent->instruments);
    instrument->parent->instruments = new_instruments;
    free(instrument->volume);
    free(instrument->pitch);
    free(instrument->duty_cycle);
    free(instrument);
}