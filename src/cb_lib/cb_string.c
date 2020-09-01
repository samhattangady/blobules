#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cb_string.h"

int BASE_STRING_LENGTH = 1024;

int append_chars(string* base, char* chars) {
    short should_realloc = 0;
    u32 memory_allotted = base->memory_allotted;
    while(memory_allotted < string_length(base) + strlen(chars)) {
        memory_allotted *= 2;
        should_realloc = 1;
    }
    if (should_realloc == 1) {
        printf("reallocing... append_chars\n");
        base->text = (char*) realloc(base->text, memory_allotted);
        base->memory_allotted = memory_allotted;
    }
    strcat(base->text, chars);
    return 0;
}

int va_append_sprintf(string* base, char* fbase, va_list args) {
    // TODO (24 Feb 2020 sam): PERFORMANCE. See if we can do this in a single
    // va loop. Currently, we use vsnprintf to get the length of the temp
    // buffer. Then, we use vsprintf to actually copy the required stuffs to the
    // string. There might be a way to do it in a single rep. Since we do it twice,
    // we also require an extra variable (args_copy);
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fbase, args) + 1;
	// FIXME (09 Jun 1010 sam): Should be size size. Was not working in windows
    char s[1000];
    s[0] = '\0';
    vsprintf(&s[0], fbase, args_copy);
    s[size-1] = '\0';
    append_chars(base, s);
    return 0;
}

int string_length(string* s) {
    return strlen(s->text);
}

int clear_string(string* s) {
    s->text[0] = '\0';
    return 0;
}

int print_string(string* s) {
    printf(s->text);
    return 0;
}

int dispose_string(string* base) {
    free(base->text);
    return 0;
}

string empty_string() {
    return string_from("");
}

string string_from(char* text) {
    unsigned int len = BASE_STRING_LENGTH;
    while (len < strlen(text))
        len *= 2;
    char* s = (char*) malloc(len * sizeof(char));
    s[0] = '\0';
    strcat(s, text);
    // TODO (31 Mar 2020 sam): for some reason, returning without allocating
    // doesn't work here. We can't just return {s, len}; for som reason...
    string result = {s, len};
    return result;
}

int append_string(string* base, string* appendage) {
    append_chars(base, appendage->text);
    return 0;
}

string stringf(char* base, ...) {
    va_list args;
    va_start(args, base);
    string result = empty_string();
    va_append_sprintf(&result, base, args);
    va_end(args);
    return result;
}

int append_sprintf(string* base, char* fbase, ...) {
    va_list args;
    va_start(args, fbase);
    va_append_sprintf(base, fbase, args);
    va_end(args);
    return 0;
}

string read_file(char* filename) {
    // https://stackoverflow.com/a/3464656/5453127
    char* buffer = NULL;
    int string_size, read_size;
    FILE* handler = fopen(filename, "r");
    if (handler) {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        printf("mallocing... read_file %s\n", filename);
        buffer = (char*) malloc(sizeof(char) * (string_size+1) );
        // TODO (12 Jun 2020 sam): Why do we have to memset here? Is the size wrong?
        // Without the memset, it is inconsistent. Does not work all the time. Wierd.
        memset(buffer, 0, sizeof(char)*(string_size+1));
        read_size = fread(buffer, sizeof(char), string_size, handler);
        buffer[string_size] = '\0';
        if (string_size != read_size) {
            // TODO (02 Apr 2020 sam): Raise some error here?
            // free(buffer);
            // buffer = NULL;
        }
        fclose(handler);
    }
    string result = string_from(buffer);
    free(buffer);
    return result;
}
