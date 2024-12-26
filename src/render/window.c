#include "imgui/cimgui.h"

#include "main.h"
#include "renderer.h"

#include "window.h"

struct WindowData {
    ImGuiID id;
    int scroll_x, scroll_y;
    bool scrolling;
    int scroll_origin_x, scroll_origin_y;
    int scroll_limit_min_x, scroll_limit_min_y;
    int scroll_limit_max_x, scroll_limit_max_y;
    float zoom;
    SDL_Texture* texture;
    struct WindowData* next;
};

struct WindowData *window_data, *window_data_head;
int dragging, dragging_start_x, dragging_start_y;

void register_window(ImGuiID id) {
    if (window_data == NULL) window_data = window_data_head = calloc(sizeof(struct WindowData), 1);
    else window_data_head = window_data_head->next;
    ImVec2 window_size;
    igGetWindowSize(&window_size);
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    window_data_head->next = calloc(sizeof(struct WindowData), 1);
    window_data_head->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dm.w, dm.h);
    window_data_head->id = id;
    window_data_head->zoom = 1;
}

struct WindowData* find_window(ImGuiID id) {
    struct WindowData* curr = window_data;
    if (!curr) return NULL;
    while (curr->next) {
        if (curr->id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

void window_scroll_mode(float x, float y) {
    struct WindowData* window_data = find_window(igGetItemID());
    render_translate_x = window_data->scroll_x * x;
    render_translate_y = window_data->scroll_y * y;
    ImVec2 size;
    igGetWindowSize(&size);
    if (-render_translate_x + size.x >= window_data->scroll_limit_max_x) render_translate_x = -(window_data->scroll_limit_max_x - size.x);
    if (-render_translate_y + size.y >= window_data->scroll_limit_max_y) render_translate_y = -(window_data->scroll_limit_max_y - size.y);
    if (-render_translate_x          <  window_data->scroll_limit_min_x) render_translate_x = -(window_data->scroll_limit_min_x);
    if (-render_translate_y          <  window_data->scroll_limit_min_y) render_translate_y = -(window_data->scroll_limit_min_y);
}

void window_scroll_limit(int min_x, int min_y, int max_x, int max_y) {
    struct WindowData* window = find_window(igGetItemID());
    window->scroll_limit_min_x = min_x;
    window->scroll_limit_min_y = min_y;
    window->scroll_limit_max_x = max_x;
    window->scroll_limit_max_y = max_y;
}

void window_begin_drag(int button) {
    SDL_GetMouseState(&dragging_start_x, &dragging_start_y);
    dragging = button;
}

void window_end_drag() {
    dragging = 0;
}

void window_handle_scroll(struct WindowData* window) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    if ((click_state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) && igIsWindowHovered(ImGuiHoveredFlags_None)) {
        window->scrolling = true;
        window->scroll_origin_x = x;
        window->scroll_origin_y = y;
    }
    if (window->scrolling) {
        if (dragging & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
            window->scroll_x += x - window->scroll_origin_x;
            window->scroll_y += y - window->scroll_origin_y;
            window->scroll_origin_x = x;
            window->scroll_origin_y = y;
        }
        else {
            window_scroll_mode(true, true);
            window->scroll_x = render_translate_x;
            window->scroll_y = render_translate_y;
            window->scrolling = false;
        }
    }
}

void window_handle_zoom(float modifier, bool modify_x, bool modify_y, float min, float max) {
    struct WindowData* window = find_window(igGetItemID());
    float prev_zoom = window->zoom;
    if (scroll_state < 0) window->zoom /= modifier * -scroll_state;
    if (scroll_state > 0) window->zoom *= modifier *  scroll_state;
    if (window->zoom < min) window->zoom = min;
    if (window->zoom > max) window->zoom = max;
    float cursor_x = cursor_rel_x();
    float cursor_y = cursor_rel_y();
    if (modify_x) window->scroll_x = (window->scroll_x - cursor_x) * (window->zoom / prev_zoom) + cursor_x;
    if (modify_y) window->scroll_y = (window->scroll_y - cursor_y) * (window->zoom / prev_zoom) + cursor_y;
}

void window_begin(const char* name) {
    igBegin(name, NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGuiDockNode_SetLocalFlags(igGetWindowDockNode(), ImGuiDockNodeFlags_NoTabBar);
    struct WindowData* window = find_window(igGetItemID());
    if (!window) {
        register_window(igGetItemID());
        return;
    }
    SDL_SetRenderTarget(renderer, window->texture);
    SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    window_scroll_mode(true, true);
    window_handle_scroll(window);
}

void window_end() {
    SDL_Texture* texture = SDL_GetRenderTarget(renderer);
    if (texture) {
        int w, h;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        SDL_SetRenderTarget(renderer, NULL);
        igSetCursorPos((ImVec2){ 0, 0 });
        igImage((ImTextureID)texture, (ImVec2){ w, h }, (ImVec2){ 0, 0 }, (ImVec2){ 1, 1 }, (ImVec4){ 1, 1, 1, 1 }, (ImVec4){ 0, 0, 0, 0 });
    }
    igEnd();
}

float window_zoom() {
    return find_window(igGetItemID())->zoom;
}

void tooltip(const char* fmt, ...) {
    va_list list;
    va_start(list, fmt);
    igBeginTooltip();
    igTextV(fmt, list);
    igEndTooltip();
    va_end(list);
}

int cursor_rel_x() {
    int x;
    ImVec2 window_pos;
    igGetWindowPos(&window_pos);
    SDL_GetMouseState(&x, NULL);
    return x - window_pos.x;
}

int cursor_rel_y() {
    int y;
    ImVec2 window_pos;
    igGetWindowPos(&window_pos);
    SDL_GetMouseState(NULL, &y);
    return y - window_pos.y;
}

int cursor_x() {
    return cursor_rel_x() - render_translate_x;
}

int cursor_y() {
    return cursor_rel_y() - render_translate_y;
}

bool hovered(int x, int y, int w, int h) {
    return cursor_x() >= x && cursor_y() >= y && cursor_x() < x + w && cursor_y() < y + h && igIsWindowHovered(ImGuiHoveredFlags_None);
}

bool clicked(int x, int y, int w, int h, int button) {
    return hovered(x, y, w, h) && (click_state & SDL_BUTTON(button));
}

bool drag(int* start_x, int* start_y, int* end_x, int* end_y, int button) {
    if (!(dragging & SDL_BUTTON(button))) return false;
    ImVec2 window_pos;
    igGetWindowPos(&window_pos);
    if (start_x) *start_x = dragging_start_x - window_pos.x - render_translate_x;
    if (start_y) *start_y = dragging_start_y - window_pos.y - render_translate_y;
    if (end_x) *end_x = cursor_x();
    if (end_y) *end_y = cursor_y();
    return true;
}

