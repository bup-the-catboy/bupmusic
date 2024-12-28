#ifndef BUPMUSIC_WINDOW_H
#define BUPMUSIC_WINDOW_H

#include <stdbool.h>

void window_scroll_mode(float x, float y);
void window_scroll_limit(int min_x, int min_y, int max_x, int max_y);
void window_handle_zoom(float modifier, bool modify_x, bool modify_y, float min, float max);
void window_begin_drag(int button);
void window_end_drag();
void window_begin(const char* name);
void window_end();
float window_zoom();

void tooltip(const char* fmt, ...);

int cursor_x();
int cursor_y();
int cursor_rel_x();
int cursor_rel_y();
int rel_x(int pos);
int rel_y(int pos);
bool clicked(int x, int y, int w, int h, int button);
bool hovered(int x, int y, int w, int h);
bool drag(int* start_x, int* start_y, int* end_x, int* end_y, int button);

#endif