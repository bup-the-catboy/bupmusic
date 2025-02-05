#include "audio.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SDL3/SDL.h>

struct AudioBuffer {
    short* samples;
    struct AudioBuffer* next;
};

SDL_AudioStream* audio_stream;
struct AudioBuffer *audio_buffers, *audio_buffers_head;
short audio_final_samples[1600];

short* audio_create_sample_buffer() {
    if (!audio_buffers) audio_buffers = audio_buffers_head = calloc(sizeof(struct AudioBuffer), 1);
    else audio_buffers_head = audio_buffers_head->next;
    audio_buffers_head->next = calloc(sizeof(struct AudioBuffer), 1);
    audio_buffers_head->samples = calloc(sizeof(short), audio_num_samples());
    return audio_buffers_head->samples;
}

int audio_num_samples() {
    return 1600;
}

void audio_init() {
    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 48000 };
    audio_stream = SDL_CreateAudioStream(&spec, &spec);
}

void audio_update() {
    memset(audio_final_samples, 0, sizeof(audio_final_samples));
    struct AudioBuffer* curr = audio_buffers;
    while (curr) {
        if (curr->samples) {
            for (int i = 0; i < audio_num_samples(); i++) {
                audio_final_samples[i] += curr->samples[i];
            }
            free(curr->samples);
        }
        struct AudioBuffer* next = curr->next;
        free(curr);
        curr = next;
    }
    if (SDL_GetAudioStreamQueued(audio_stream) > 0) usleep(1);
    SDL_PutAudioStreamData(audio_stream, audio_final_samples, sizeof(audio_final_samples));
    audio_buffers = NULL;
}

void audio_deinit() {
    SDL_DestroyAudioStream(audio_stream);
}