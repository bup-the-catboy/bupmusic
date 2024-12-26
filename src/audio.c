#include "audio.h"

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include <SDL2/SDL.h>

struct AudioBuffer {
    short* samples;
    struct AudioBuffer* next;
};

struct AudioBuffer *audio_buffers, *audio_buffers_head;
bool audio_running = false;
short audio_final_samples[1600];
pthread_t audio_thread_id;

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

void* audio_thread(void* _) {
    SDL_AudioSpec spec;
    spec.freq = 48000;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 800;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
    audio_running = true;
    FILE* f = fopen("test.pcm", "w");
    while (audio_running) {
        while (SDL_GetQueuedAudioSize(1) > sizeof(audio_final_samples)) SDL_Delay(1);
        fwrite(audio_final_samples, sizeof(audio_final_samples), 1, f);
        SDL_QueueAudio(1, audio_final_samples, sizeof(audio_final_samples));
    }
    fclose(f);
    SDL_CloseAudio();
    return NULL;
}

void audio_init() {
    pthread_create(&audio_thread_id, NULL, audio_thread, NULL);
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
    audio_buffers = NULL;
}

void audio_deinit() {
    audio_running = false;
    pthread_join(audio_thread_id, NULL);
}