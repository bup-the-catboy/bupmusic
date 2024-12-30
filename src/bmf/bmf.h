#ifndef BUP_MUSIC_FORMAT_H
#define BUP_MUSIC_FORMAT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    bmf_note_type_audible,
    bmf_note_type_volume,
    bmf_note_type_pitch
} bmf_note_type;

typedef enum {
    bmf_channel_type_square,
    bmf_channel_type_triangle,
    bmf_channel_type_sine,
    bmf_channel_type_sawtooth,
    bmf_channel_type_noise,
} bmf_channel_type;

typedef struct bmf bmf;
typedef struct bmf_instrument bmf_instrument;
typedef struct bmf_song bmf_song;
typedef struct bmf_pattern bmf_pattern;
typedef struct bmf_note bmf_note;

#define bmf_get_ro(array, index) ({       \
    __typeof__(*(array)) value;            \
    __typeof__(array) temp_array = (array); \
    value = temp_array[(index)];             \
    free(temp_array);                         \
    value;                                     \
})

bmf*    bmf_create();
bmf*    bmf_load(const            char* data, int  length);
char*   bmf_save                  (bmf* data, int* out_len);
uint8_t bmf_num_songs             (bmf* data);
uint8_t bmf_num_instruments       (bmf* data);
uint8_t bmf_curr_song_num_channels(bmf* data);
uint8_t bmf_current_song          (bmf* data);
float   bmf_current_tempo         (bmf* data);
float   bmf_current_time          (bmf* data);
float   bmf_get_master_volume     (bmf* data);
float   bmf_get_channel_volume    (bmf* data, uint8_t channel);
void    bmf_select_song           (bmf* data, uint8_t song);
void    bmf_seek                  (bmf* data, float secs);
void    bmf_tempo                 (bmf* data, float tempo);
void    bmf_master_volume         (bmf* data, float vol);
void    bmf_channel_volume        (bmf* data, float vol, uint8_t channel);
void    bmf_play                  (bmf* data, short* samples_out, int num_out);
void    bmf_destroy               (bmf* data);
void    bmf_play_instrument       (bmf_instrument* instrument, bmf_channel_type channel_type, short* samples_out, int num_samples, uint16_t frequency, float duty_cycle, uint8_t frame, uint64_t* timer);
float   bmf_sample                (bmf_channel_type channel_type, uint16_t frequency, float duty_cycle, uint64_t sample_index);

bmf_instrument* bmf_get_instruments           (bmf* data);
bmf_instrument* bmf_add_instrument            (bmf* data);
uint16_t* bmf_instrument_get_volume           (bmf_instrument* instrument, uint8_t* length);
 int16_t* bmf_instrument_get_pitch            (bmf_instrument* instrument, uint8_t* length);
uint16_t* bmf_instrument_get_duty_cycle       (bmf_instrument* instrument, uint8_t* length);
void      bmf_instrument_set_volume_length    (bmf_instrument* instrument, uint8_t  length);
void      bmf_instrument_set_pitch_length     (bmf_instrument* instrument, uint8_t  length);
void      bmf_instrument_set_duty_cycle_length(bmf_instrument* instrument, uint8_t  length);
void      bmf_remove_instrument               (bmf_instrument* instrument);

bmf_song*         bmf_get_songs             (bmf* data);
bmf_song*         bmf_add_song              (bmf* data, uint8_t length, uint8_t bpm);
uint8_t           bmf_get_bpm               (bmf_song* song);
uint8_t           bmf_get_length            (bmf_song* song);
uint8_t           bmf_get_loop              (bmf_song* song);
void              bmf_set_bpm               (bmf_song* song, uint8_t bpm);
void              bmf_set_length            (bmf_song* song, uint8_t length);
void              bmf_set_loop              (bmf_song* song, uint8_t loop);
void              bmf_add_channel           (bmf_song* song, bmf_channel_type type);
void              bmf_add_channel_at        (bmf_song* song, bmf_channel_type type, uint8_t index);
uint8_t*          bmf_get_pattern_table     (bmf_song* song);
uint8_t           bmf_get_pattern_table_at  (bmf_song* song, uint8_t channel, uint8_t position);
void              bmf_set_pattern_table_at  (bmf_song* song, uint8_t channel, uint8_t position, uint8_t index);
void              bmf_clear_pattern_table_at(bmf_song* song, uint8_t channel, uint8_t position);
void              bmf_set_pattern_table     (bmf_song* song, uint8_t* pattern_table);
uint8_t           bmf_num_channels          (bmf_song* song);
uint8_t           bmf_num_patterns          (bmf_song* song);
bmf_channel_type* bmf_get_channels          (bmf_song* song);
bmf_pattern*      bmf_get_pattern           (bmf_song* song, uint8_t index);
void              bmf_remove_song           (bmf_song* song);

bmf_pattern* bmf_add_pattern      (bmf_song* song);
uint8_t      bmf_get_pattern_index(bmf_pattern* pattern);
bmf_note*    bmf_pattern_get_notes(bmf_pattern* pattern);
uint8_t      bmf_pattern_num_notes(bmf_pattern* pattern);
void         bmf_set_note         (bmf_pattern* pattern, uint8_t start, uint8_t length, bmf_note_type type, uint16_t value);
void         bmf_set_slide_note   (bmf_pattern* pattern, uint8_t start, uint8_t length, bmf_note_type type, uint16_t base, uint16_t slide);
void         bmf_remove_pattern   (bmf_pattern* pattern);

void* bmf_get(void* bmf_struct_array, int index);

#endif
