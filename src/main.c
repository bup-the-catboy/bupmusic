#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <unistd.h>

#define CIMGUI_USE_SDL2
#include "imgui/cimgui.h"
#include "imgui/cimgui_impl.h"

#include "audio.h"
#include "ui/editor.h"
#include "render/window.h"

bool imgui_config_exists = false;
SDL_Window* window;
SDL_Renderer* renderer;
int click_state = 0;
int scroll_state = 0;

void create_dockspace_layout(ImGuiID dockspace) {
    if (imgui_config_exists) return;
    imgui_config_exists = true;
    ImGuiID patterns, piano_roll;
    igDockBuilderRemoveNode(dockspace);
    igDockBuilderAddNode(dockspace, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoTabBar);
    igDockBuilderSetNodeSize(dockspace, igGetWindowViewport()->Size);
    igDockBuilderSplitNode(dockspace, ImGuiDir_Up, 0.4f, &patterns, &piano_roll);
    igDockBuilderDockWindow("Patterns", patterns);
    igDockBuilderDockWindow("Piano Roll", piano_roll);
}

void setup_dockspace() {
    ImGuiViewport* viewport = igGetWindowViewport();
    igSetNextWindowPos((ImVec2){ 0, 0 }, ImGuiCond_None, (ImVec2){ 0, 0 });
    igSetNextWindowSize(viewport->Size, ImGuiCond_None);
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar;
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){ 0, 0 });
    igBegin("Dockspace", NULL, windowFlags);
    igPopStyleVar(1);
    ImGuiID dockspace = igDockSpace(igGetID_Str("Dockspace"), (ImVec2){ 0.0f, 0.0f }, ImGuiDockNodeFlags_PassthruCentralNode, NULL);
    igEnd();
    create_dockspace_layout(dockspace);
}

int main() {
    window = SDL_CreateWindow("BupMusic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool running = true;
    igCreateContext(NULL);
    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->IniFilename = NULL;
    io->LogFilename = NULL;
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    imgui_config_exists = access("imgui.ini", F_OK) == 0;
    audio_init();
    while (running) {
        uint64_t start = SDL_GetTicks64();
        SDL_Event event;
        click_state = 0;
        scroll_state = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                click_state |= event.button.button;
                window_begin_drag(event.button.button);
            }
            if (event.type == SDL_MOUSEBUTTONUP) window_end_drag();
            if (event.type == SDL_MOUSEWHEEL) scroll_state = event.wheel.y;
            ImGui_ImplSDL2_ProcessEvent(&event);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        igNewFrame();

        setup_dockspace();
        editor_update();
        audio_update();

        igRender();
        ImGui_ImplSDLRenderer2_RenderDrawData(igGetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        uint64_t end = SDL_GetTicks64();
        int wait = 16 - (int)(end - start);
        if (wait <= 0) continue;
        SDL_Delay(wait);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(igGetCurrentContext());
    audio_deinit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}