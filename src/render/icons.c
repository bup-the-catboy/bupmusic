#include "icons.h"
#include "renderer.h"

void icon_draw_outline(int x, int y) {
    render_rect_fill( 0 + x,  0 + y, 32,  2);
    render_rect_fill( 0 + x, 30 + y, 32,  2);
    render_rect_fill( 0 + x,  2 + y,  2, 28);
    render_rect_fill(30 + x,  2 + y,  2, 28);
}

void icon_draw_square(int x, int y) {
    icon_draw_outline(x, y);
    render_rect_fill(4 + x, 4 + y, 24, 24);
}

void icon_draw_triangle(int x, int y) {
    icon_draw_outline(x, y);
    render_triangle(16 + x, 4 + y, 4 + x, 28 + y, 28 + x, 28 + y);
}

void icon_draw_sine(int x, int y) {
    icon_draw_outline(x, y);
    render_circle(16 + x, 16 + y, 12);
}

void icon_draw_sawtooth(int x, int y) {
    icon_draw_outline(x, y);
    render_triangle(28 + x, 4 + y, 4 + x, 28 + y, 28 + x, 28 + y);
}

void icon_draw_noise(int x, int y) {
    icon_draw_outline(x, y);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 2 == 1) continue;
            render_rect_fill(4 + x + i * 6, 4 + y + j * 6, 6, 6);
        }
    }
}

void icon_draw_resize(int x, int y) {
    render_triangle(x + 1,  y + 8, x + 5,  y + 4, x + 5,  y + 12);
    render_triangle(x + 16, y + 8, x + 11, y + 4, x + 11, y + 12);
    render_rect_fill(x + 4, y + 7, 8, 2);
}
