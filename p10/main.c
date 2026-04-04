#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

bool is_line_overflow(char* buf, FILE *jack_file) {
    if (strchr(buf, '\n') == NULL && !feof(jack_file)) {
        return true;
    }

    return false;
}

void parse_file(FILE *jack_file) {
    char buf[2048];
    bool is_ml_comment = false;
    char tmp[2048];
    while (fgets(buf, sizeof buf, jack_file) != NULL) {
        char* current = buf;
        if (is_line_overflow(current, jack_file)) {
            fprintf(stderr, "Error: Exceeding line limit of 2048 chars.");
            break;
        }
        if (!is_ml_comment) {
            char* sl_comment = strstr(current, "//");
            if (comment != NULL) {
                *comment = '\0';
            }
        }

        for (int i = 0; current[i] != '\0'; i++) {
            
            if (is_ml_comment) 
                continue;

        }
        char* ml_comment_start = strstr(current, "/*");
        if (!is_ml_comment && ml_comment_start != NULL) {
            *ml_comment_start = '\0';
            is_ml_comment = true;
        }

        char* ml_comment_end = strstr(current, "*/");
        if (ml_comment_end != NULL) {
            is_ml_comment = false;
        }
    }
}

int main(int argc, char *argv[]) {
    // TODO: Implement dir / single file argv. For now assuming it s a jack file
    FILE *jack_file = fopen(argv[1], "r");
    if (jack_file == NULL) {
        fprintf(stderr, "Error: Could not open file");
        return 1;
    }

    parse_file(jack_file);
    return 0;
}