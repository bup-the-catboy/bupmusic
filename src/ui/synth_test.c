#include <SDL2/SDL.h>

#include <pthread.h>

#include "audio.h"
#include "imgui/cimgui.h"
#include "bmf/bmf.h"

static int synth_test_wave_type = 0;
static int synth_test_frequency = 500;
static bool synth_test_playing = false;
static pthread_t synth_test_thread_id;
static int synth_test_timer = 0;
static float synth_test_duty_cycle = 0.5;

void synth_test() {
    igCombo_Str("Wave Type", &synth_test_wave_type, "Square\0Triangle\0Sine\0Sawtooth\0Noise\0", 0);
    int prev_freq = synth_test_frequency;
    if (igSliderInt("Frequency", &synth_test_frequency, 1, 1000, "%d", ImGuiSliderFlags_None)) {
        synth_test_timer *= (prev_freq / (float)synth_test_frequency);
    }
    igSliderFloat("Duty Cycle", &synth_test_duty_cycle, 0, 1, "%.3f", ImGuiSliderFlags_None);
    if (igButton(synth_test_playing ? "Stop" : "Play", (ImVec2){ 0, 0 })) synth_test_playing ^= 1;

    short* samples_buffer = audio_create_sample_buffer();
    if (synth_test_playing) {
        bmf_play_instrument(NULL, synth_test_wave_type, samples_buffer, audio_num_samples(), synth_test_frequency, synth_test_duty_cycle, 0, &synth_test_timer);
        synth_test_timer++;
    }
    else synth_test_timer = 0;

    float samples[800];
    for (int i = 0; i < 800; i++) {
        samples[i] = samples_buffer[i * 2] / 65536.f + 0.5;
    }
    igText("Sample Graph");
    igPlotLines_FloatPtr("##sample_graph", samples, 800, 0, "", 0.4, 0.6, (ImVec2){ 256, 64 }, sizeof(float));
}