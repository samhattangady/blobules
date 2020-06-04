#ifndef CHAPLIBOY_STRING_DEFINED
#define CHAPLIBOY_STRING_DEFINED

#include "cb_types.h"

typedef struct string {
    char* text;
    uint memory_allotted;
} string;

int append_chars(string* base, char* chars);
string empty_string();
string string_from(char* text);
string stringf(char* base, ...);
int clear_string(string* s);
int print_string(string* s);
int append_string(string* base, string* appendage);
int dispose_string(string* base);
int string_length(string* s);
int append_sprintf(string* s, char* base, ...);
string read_file(char* filename);

#endif
