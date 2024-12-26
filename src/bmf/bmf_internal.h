#include <stdint.h>

#include "bmf.h"

#define SAMPLE_RATE 48000
#define FPS 60
#define STEREO 2
#define SAMPLE_BUFSIZ (SAMPLE_RATE / FPS * STEREO)

typedef struct {
    int frequency;
    int volume;
    int duty;
    int timer;
} bmf_channel_properties;

struct bmf_note {
    int size;

    bmf_note_type type;
    int length;
    int start;
    int instrument;
    int value_from;
    int value_to;
};

struct bmf_pattern {
    int size;

    int num_notes;
    bmf_note* notes;
    bmf_song* parent;
};

struct bmf_song {
    int size;

    int length;
    int channels;
    int patterns;
    int bpm;
    int loop;
    uint8_t* pattern_table;
    uint8_t* channel_table;
    bmf_pattern* pattern_data;
    bmf* parent;
};

struct bmf_instrument {
    int size;

    int  num_duty_cycle;
    int  num_volume;
    int  num_pitch;
    uint16_t* duty_cycle;
    uint16_t* volume;
     int16_t* pitch;
    bmf* parent;
};

struct bmf {
    int size;

    short sample_buffer[SAMPLE_BUFSIZ];
    int sample_buffer_ptr;
    int curr_song;
    int num_songs;
    int num_instruments;
    int frame;
    int timer;
    float tempo;
    float master_volume;
    float channel_volumes[256];
    bmf_song* songs;
    bmf_instrument* instruments;
};

void bmf_synth_frame(bmf_song* song, short* buf_out, int* timer, float frame, float tempo, int channel);
void bmf_mix(short* dst, short* src, int samples);
void bmf_volume(short* samples, float volume, int num_samples);

typedef struct bmf_binary bmf_binary;
struct bmf_binary {
    char* data;
    int ptr;
    int capacity;
};

bmf_binary* bin_create();
bmf_binary* bin_alloc(int size);
bmf_binary* bin_wrap(char* data, int size);
void bin_read (bmf_binary* binary, void* dst, int size);
void bin_write(bmf_binary* binary, void* src, int size);