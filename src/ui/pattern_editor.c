#include "render/renderer.h"
#include "render/window.h"
#include "render/icons.h"
#include "imgui/cimgui.h"
#include "bmf/bmf.h"

void pattern_editor(bmf_song* curr_song, int* curr_channel) {
    if (!curr_song) return;
    bmf_channel_type* types = bmf_get_channels(curr_song);
    window_scroll_limit(0, 0, (bmf_get_length(curr_song) + 10) * 74 + 40, bmf_num_channels(curr_song) * 74 + 20);
    window_scroll_mode(false, false);
    render_set_color(0x1F1F1FFF);
    render_rect_fill(2, 2, 36, 16);
    window_scroll_mode(true, true);
    int selected_pattern = cursor_rel_x() <= 39 ? -1 : floor((cursor_x() - 39) / 74.);
    int selected_channel = cursor_rel_y() <= 19 ? -1 : floor((cursor_y() - 19) / 74.);
    if (!igIsWindowHovered(ImGuiHoveredFlags_None) ||
        selected_channel >= bmf_num_channels(curr_song) ||
        selected_pattern >= bmf_get_length  (curr_song)
    ) selected_channel = selected_pattern = -1;
    window_scroll_mode(true, false);
    render_scissor(40, 0, AUTO, 20);
    for (int i = 0; i < bmf_get_length(curr_song); i++) {
        if (selected_pattern == i) render_set_color(0x2F2F2FFF);
        else render_set_color(0x1F1F1FFF);
        render_rect_fill(40 + i * 74, 2, 72, 16);
        render_set_color(0xFFFFFFFF);
        if (i == bmf_get_loop(curr_song)) render_triangle(42 + i * 74, 4, 54 + i * 74, 4, 48 + i * 74, 16);
        render_text_anchored(4 + i * 74 + 72, 5, 0.5, 0, "%d", i + 1);
        if (clicked(39 + i * 74, 0, 74, 19, SDL_BUTTON_LEFT)) bmf_set_loop(curr_song, i);
    }
    render_set_color(hovered(39 + bmf_get_length(curr_song) * 74, 0, 19, 19) ? 0x2F2F2FFF : 0x1F1F1FFF);
    render_rect_fill(40 + bmf_get_length(curr_song) * 74, 2, 16, 16);
    render_set_color(0xFFFFFFFF);
    icon_draw_resize(40 + bmf_get_length(curr_song) * 74, 2);
    for (int i = 0; i < bmf_num_channels(curr_song); i++) {
        window_scroll_mode(false, true);
        render_scissor(0, 20, 40, AUTO);
        if (selected_channel == i) render_set_color(0x2F2F2FFF);
        else render_set_color(0x1F1F1FFF);
        render_rect_fill(2, 20 + i * 74, 36, 72);
        render_set_color(0xFFFFFFFF);
        switch (types[i]) {
            case bmf_channel_type_square:
                icon_draw_square(4, 22 + i * 74);
                break;
            case bmf_channel_type_triangle:
                icon_draw_triangle(4, 22 + i * 74);
                break;
            case bmf_channel_type_sine:
                icon_draw_sine(4, 22 + i * 74);
                break;
            case bmf_channel_type_sawtooth:
                icon_draw_sawtooth(4, 22 + i * 74);
                break;
            case bmf_channel_type_noise:
                icon_draw_noise(4, 22 + i * 74);
                break;
        }
        window_scroll_mode(true, true);
        render_scissor(40, 20, AUTO, AUTO);
        if (selected_channel == i) *curr_channel = selected_channel;
        for (int j = 0; j < bmf_get_length(curr_song); j++) {
            bool a = selected_channel == i;
            bool b = selected_pattern == j;
            if      (a && b) render_set_color(0x4F4F4FFF);
            else if (a || b) render_set_color(0x3F3F3FFF);
            else             render_set_color(0x2F2F2FFF);
            render_rect_fill(40 + j * 74, 20 + i * 74, 72, 72);
        }
    }
    render_unscissor();
    free(types);
}