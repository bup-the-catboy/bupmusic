#include "bmf/bmf.h"
#include "render/renderer.h"
#include "render/window.h"
#include "audio.h"

#include <SDL3/SDL.h>

#define NOTES_PER_OCTAVE 12
#define OCTAVES           9
#define NOTES (NOTES_PER_OCTAVE * OCTAVES)

#define NOTE(x) x + NOTES_PER_OCTAVE *
#define C  NOTE(0)
#define CX NOTE(1)
#define D  NOTE(2)
#define DX NOTE(3)
#define E  NOTE(4)
#define F  NOTE(5)
#define FX NOTE(6)
#define G  NOTE(7)
#define GX NOTE(8)
#define A  NOTE(9)
#define AX NOTE(10)
#define B  NOTE(11)

int frequency_table[] = {
// octave   C     C#    D     D#    E     F     F#    G     G#    A     A#    B
/*   0  */ 16,   17,   18,   19,   21,   22,   23,   25,   26,   28,   29,   31,
/*   1  */ 33,   35,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62,
/*   2  */ 65,   69,   73,   78,   82,   87,   93,   98,   104,  110,  117,  123,
/*   3  */ 131,  139,  147,  156,  165,  175,  185,  196,  208,  220,  233,  247,
/*   4  */ 262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,
/*   5  */ 523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,
/*   6  */ 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
/*   7  */ 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
/*   8  */ 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902
};

const char* note_name_table[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static uint64_t piano_roll_timer = 0;
static int piano_roll_frame = 0;

void piano_roll(bmf_song* curr_song, bmf_instrument* curr_instrument, int curr_channel) {
    if (!curr_song) return;
    if (bmf_num_channels(curr_song) == 0) return;
    window_handle_zoom(1.1, true, false, 1 / 32.f, 32);
    int pattern_size = 256 * window_zoom();
    window_scroll_mode(false, false);
    window_scroll_limit(0, 0, pattern_size * bmf_get_length(curr_song) + 48, 16 * NOTES + 16);
    render_set_color(0x1F1F1FFF);
    render_rect_fill(2, 2, 44, 12);
    window_scroll_mode(false, true);
    int start_x, start_y, end_x, end_y;
    int playing_piano_note = -1;
    if (drag(&start_x, &start_y, &end_x, &end_y, SDL_BUTTON_LEFT)) {
        if (start_x < 48) playing_piano_note = NOTES - (end_y - 16) / 16 - 1;
        if (rel_y(start_y) < 16) playing_piano_note = -1;
        if (playing_piano_note >= 0 && playing_piano_note < NOTES) bmf_play_instrument(
            curr_instrument,
            bmf_get_ro(bmf_get_channels(curr_song), curr_channel),
            audio_create_sample_buffer(), audio_num_samples(),
            frequency_table[playing_piano_note], 0,
            piano_roll_frame++, &piano_roll_timer
        );
    }
    else piano_roll_timer = piano_roll_frame = 0;
    render_scissor(0, 16, AUTO, AUTO);
    for (int i = 0; i < NOTES; i++) {
        int note = NOTES - i - 1;
        if (hovered(0, i * 16 + 16, 48, 16) || playing_piano_note == note) {
            tooltip("%d Hz", frequency_table[note]);
            render_set_color(0xFFFFFFFF);
        }
        else render_set_color(i % 2 == 0 ? 0xCFCFCFFF : 0xDFDFDFFF);
        render_rect_fill(0, i * 16 + 16, 48, 16);
        render_set_color(i % 2 == 0 ? 0x1F1F1FFF : 0x282828FF);
        render_rect_fill(48, i * 16 + 16, AUTO, 16);
        render_set_color(0x0F0F0FFF);
        render_text_anchored(46, i * 16 + 16 + 3, 1, 0, "%s%d", note_name_table[note % NOTES_PER_OCTAVE], note / NOTES_PER_OCTAVE);
        render_line(0, i * 16 + 16, 48 + bmf_get_length(curr_song) * pattern_size, AUTO);
        if (i % NOTES_PER_OCTAVE == NOTES_PER_OCTAVE - 1) render_line(0, i * 16 + 31, 48 + bmf_get_length(curr_song) * pattern_size, AUTO);
    }
    window_scroll_mode(true, false);
    render_scissor(48, 0, AUTO, AUTO);
    for (int i = 0; i < bmf_get_length(curr_song); i++) {
        render_set_color(0x1F1F1FFF);
        render_rect_fill(49 + i * pattern_size, 2, pattern_size - 2, 12);
        render_set_color(0xFFFFFFFF);
        render_text_anchored(48 + pattern_size / 2 + i * pattern_size, 3, 0.5, 0, "%d", i + 1);
        render_set_color(0x0F0F0FFF);
        render_line(48 + (i + 0) * pattern_size, 16, AUTO, NOTES * 16);
        render_line(47 + (i + 1) * pattern_size, 16, AUTO, NOTES * 16);
        render_line(48 + (pattern_size * 0.25) + i * pattern_size, 16, AUTO, NOTES * 16);
        render_line(48 + (pattern_size * 0.50) + i * pattern_size, 16, AUTO, NOTES * 16);
        render_line(48 + (pattern_size * 0.75) + i * pattern_size, 16, AUTO, NOTES * 16);
    }
    render_unscissor();
    render_set_color(0x0F0F0FFF);
    render_rect_fill(48 + bmf_get_length(curr_song) * pattern_size, 0, AUTO, AUTO);
}