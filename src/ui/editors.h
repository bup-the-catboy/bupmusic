#ifndef BUPMUSIC_EDITORS_H
#define BUPMUSIC_EDITORS_H

#include "bmf/bmf.h"

void pattern_editor(bmf_song* curr_song, int* curr_channel);
void piano_roll(bmf_song* curr_song, int curr_channel);
void synth_test();

#endif