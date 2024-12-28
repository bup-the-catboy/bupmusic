#ifndef BUPMUSIC_MAIN_H
#define BUPMUSIC_MAIN_H

#include <SDL2/SDL.h>
#include <stdbool.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int click_state;
extern int scroll_state;
extern bool just_started_dragging;

#endif