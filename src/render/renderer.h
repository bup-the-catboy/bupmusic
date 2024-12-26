#ifndef BUPMUSIC_RENDERER_H
#define BUPMUSIC_RENDERER_H

#define AUTO -2147483648

extern int render_translate_x;
extern int render_translate_y;

void render_set_color    (unsigned int rgba);
void render_rect_fill    (int x,  int y,  int w,  int h);
void render_rect_outl    (int x,  int y,  int w,  int h);
void render_triangle     (int x1, int y1, int x2, int y2, int x3, int y3);
void render_circle       (int x,  int y,  int r);
void render_line         (int x1, int y1, int x2, int y2);
void render_text         (int x,  int y,  const char* fmt, ...);
void render_text_anchored(int x,  int y,  float ax, float ay, const char* fmt, ...);
void render_scissor      (int x,  int y,  int w, int h);
void render_unscissor    ();

int text_size(const char* fmt, ...);

#endif