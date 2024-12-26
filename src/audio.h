#ifndef BUPMUSIC_AUDIO_H
#define BUPMUSIC_AUDIO_H

short* audio_create_sample_buffer();
int audio_num_samples();

void audio_update();
void audio_init();
void audio_deinit();

#endif