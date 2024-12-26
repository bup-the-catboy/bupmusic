#include "bmf_internal.h"

#include <stdlib.h>
#include <string.h>

bmf_pattern* bmf_add_pattern(bmf_song* song) {
    if (song->patterns == 0xFF) return NULL;
    song->patterns++;
    song->pattern_data = realloc(song->pattern_data, sizeof(bmf_pattern) * song->patterns);
    bmf_pattern* pattern = &song->pattern_data[song->patterns - 1];
    memset(pattern, 0, sizeof(bmf_pattern));
    pattern->size = sizeof(bmf_pattern);
    pattern->parent = song;
    pattern->num_notes = 1;
    pattern->notes = calloc(sizeof(bmf_note), 1);
    pattern->notes->size = sizeof(bmf_note);
    return pattern;
}

bmf_note* bmf_pattern_get_notes(bmf_pattern* pattern) {
    return pattern->notes;
}

uint8_t bmf_pattern_num_notes(bmf_pattern* pattern) {
    return pattern->num_notes;
}

void bmf_set_note(bmf_pattern* pattern, uint8_t start, uint8_t length, bmf_note_type type, uint16_t value) {
    bmf_set_slide_note(pattern, start, length, type, value, value);
}

void bmf_set_slide_note(bmf_pattern* pattern, uint8_t start, uint8_t length, bmf_note_type type, uint16_t base, uint16_t slide) {

}

void bmf_remove_pattern(bmf_pattern* pattern) {
    bmf_pattern* new_patterns = malloc(sizeof(bmf_pattern) * (pattern->parent->patterns - 1));
    int ptr = 0;
    for (int i = 0; i < pattern->parent->patterns; i++) {
        if (pattern == &pattern->parent->pattern_data[i]) continue;
        new_patterns[ptr++] = pattern->parent->pattern_data[i];
    }
    free(pattern->parent->pattern_data);
    pattern->parent->pattern_data = new_patterns;
    free(pattern->notes);
    free(pattern);
}