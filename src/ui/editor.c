#include "editor.h"

#include "render/window.h"
#include "imgui/cimgui.h"
#include "bmf/bmf.h"
#include "tinyfiledialogs.h"
#include "editors.h"

#include <stdlib.h>
#include <string.h>

bool playing = false;
bmf* project = NULL;
bmf_song* curr_song = NULL;
bmf_instrument* curr_instrument = NULL;
int curr_channel = 0;
bool show_synth_test = false;

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

char last_saved[1024] = "";

void save_project(bool force_pick) {
    if (last_saved[0] == 0 || force_pick) {
        const char* filename = tinyfd_saveFileDialog("Save File", "", 1, (const char*[]){ "*.bmf" }, "BMF Music File");
        if (filename) strcpy(last_saved, filename);
    }
    if (last_saved[0] != 0) {
        FILE* f = fopen(last_saved, "w" BINARY);
        int size;
        char* data = bmf_save(project, &size);
        fwrite(data, size, 1, f);
        fclose(f);
    }
}

void song_properties(bmf_song* song) {
    if (igBeginMenu("BPM", true)) {
        int value = bmf_get_bpm(song);
        igInputInt("##bpm_input", &value, 1, 10, ImGuiInputTextFlags_None);
        bmf_set_bpm(song, value);
        igEndMenu();
    }
}

void instrument_properties(bmf_instrument* song) {
    if (igMenuItem_Bool("Volume",     "", false, true)) {}
    if (igMenuItem_Bool("Pitch",      "", false, true)) {}
    if (igMenuItem_Bool("Duty Cycle", "", false, true)) {}
}

void editor_update() {
    if (!project) {
        project = bmf_create();
        curr_song = bmf_add_song(project, 16, 120);
        curr_instrument = bmf_add_instrument(project);
    }
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("New", "", false, true)) {
                bmf_destroy(project);
                project = bmf_create();
            }
            if (igMenuItem_Bool("Open", "", false, true)) {
                const char* filename = tinyfd_openFileDialog("Open File", "", 1, (const char*[]){ "*.bmf" }, "BMF Music File", false);
                if (filename) {
                    strcpy(last_saved, filename);
                    FILE* f = fopen(filename, "r" BINARY);
                    fseek(f, 0, SEEK_END);
                    size_t size = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    char* data = malloc(size);
                    fread(data, size, 1, f);
                    fclose(f);
                    bmf_destroy(project);
                    project = bmf_load(data, size);
                    free(data);
                }
            }
            if (igMenuItem_Bool("Save", "", false, true)) save_project(false);
            if (igMenuItem_Bool("Save As", "", false, true)) save_project(true);
            if (igMenuItem_Bool("Quit", "", false, true)) exit(0);
            igEndMenu();
        }
        if (igBeginMenu("Add", true)) {
            if (igMenuItem_Bool("Song", "", false, true)) curr_song = bmf_add_song(project, 16, 120);
            if (igMenuItem_Bool("Instrument", "", false, true)) curr_instrument = bmf_add_instrument(project);
            if (igBeginMenu("Channel", curr_song != NULL)) {
                if (igMenuItem_Bool("Square",   "", false, true)) bmf_add_channel(curr_song, bmf_channel_type_square);
                if (igMenuItem_Bool("Triangle", "", false, true)) bmf_add_channel(curr_song, bmf_channel_type_triangle);
                if (igMenuItem_Bool("Noise",    "", false, true)) bmf_add_channel(curr_song, bmf_channel_type_noise);
                igEndMenu();
            }
            igEndMenu();
        }
        if (igBeginMenu("Select", true)) {
            if (igBeginMenu("Song", true)) {
                for (int i = 0; i < bmf_num_songs(project); i++) {
                    char name[32];
                    sprintf(name, "Song %d", i + 1);
                    bmf_song* song = bmf_get(bmf_get_songs(project), i);
                    if (igMenuItem_Bool(name, "", curr_song == song, true)) {
                        curr_song = song;
                        curr_channel = 0;
                    }
                }
                igEndMenu();
            }
            if (igBeginMenu("Instrument", true)) {
                for (int i = 0; i < bmf_num_instruments(project); i++) {
                    char name[32];
                    sprintf(name, "Instrument %d", i + 1);
                    bmf_instrument* instrument = bmf_get(bmf_get_instruments(project), i);
                    if (igMenuItem_Bool(name, "", curr_instrument == instrument, true)) curr_instrument = instrument;
                }
                igEndMenu();
            }
            igEndMenu();
        }
        if (igBeginMenu("Edit", true)) {
            if (igBeginMenu("Current Song", curr_song != NULL)) {
                song_properties(curr_song);
                igEndMenu();
            }
            if (igBeginMenu("Current Instrument", curr_instrument != NULL)) {
                instrument_properties(curr_instrument);
                igEndMenu();
            }
            if (igBeginMenu("Song", true)) {
                for (int i = 0; i < bmf_num_songs(project); i++) {
                    char name[32];
                    sprintf(name, "Song %d", i + 1);
                    if (igBeginMenu(name, true)) {
                        bmf_song* song = bmf_get(bmf_get_songs(project), i);
                        song_properties(song);
                        igSeparator();
                        if (igMenuItem_Bool("Delete", "", false, bmf_num_songs(project) > 1)) {}
                        igEndMenu();
                    }
                }
                igEndMenu();
            }
            if (igBeginMenu("Instrument", true)) {
                for (int i = 0; i < bmf_num_instruments(project); i++) {
                    char name[32];
                    sprintf(name, "Instrument %d", i + 1);
                    if (igBeginMenu(name, true)) {
                        instrument_properties(bmf_get(bmf_get_instruments(project), i));
                        igSeparator();
                        if (igMenuItem_Bool("Delete", "", false, bmf_num_instruments(project) > 1)) {}
                        igEndMenu();
                    }
                }
                igEndMenu();
            }
            if (igMenuItem_Bool("Synth Test", "", false, true)) show_synth_test = true;
            igEndMenu();
        }
        igEndMainMenuBar();
    }

    if (show_synth_test) {
        igBegin("Synth Test", &show_synth_test, ImGuiWindowFlags_NoDocking);
        synth_test();
        igEnd();
    }

    window_begin("Patterns");
    pattern_editor(curr_song, &curr_channel);
    window_end();

    window_begin("Piano Roll");
    piano_roll(curr_song, curr_instrument, curr_channel);
    window_end();
}
