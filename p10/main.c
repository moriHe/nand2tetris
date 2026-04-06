#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#define HASH_SIZE 101

typedef struct Node {
    char *key;
    struct Node *next;
} Node;

Node *hash_table[HASH_SIZE] = {NULL};
const char *keywords[] = {"class", "constructor", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return", "function"};
int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

unsigned long hash(const char *str) {
    unsigned long hash_num = 5381;
    int c;
    while (c = *str++) {
        hash_num = ((hash_num << 5) + hash_num) + c;
    }

    return hash_num;
}

void insert_node(const char *key)  {
    unsigned long hash_num = hash(key);
    int index = hash_num % HASH_SIZE;

    Node *new_node = malloc(sizeof(Node));

    new_node->key = strdup(key);
    new_node->next = hash_table[index];
    hash_table[index] = new_node;
}

Node *get_node(const char *key) {
    unsigned long hash_num = hash(key);
    int index = hash_num % HASH_SIZE;

    Node *node = hash_table[index];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}


bool is_line_overflow(char* buf, FILE *jack_file) {
    if (strchr(buf, '\n') == NULL && !feof(jack_file)) {
        return true;
    }

    return false;
}

void parse_file(FILE *jack_file) {
    //strchr("{}()[].,;+-*/&|<>=~", c) != NULL;
    char current_instr[2048];
    int i;
    int c;
    bool is_comment = false;
    // use ungetc?
    while ((c = fgetc(jack_file)) != EOF) {
        if (c == '/') {
            int next = fgetc(jack_file);
            if (next == '/' || next == '*') {
                is_comment = true;
            } else {
                ungetc(next, jack_file);
                // TODO: Handle division
            }
        }
    }

}

int main(int argc, char *argv[]) {
    for (int i = 0; i < num_keywords; i++) {
        insert_node(keywords[i]);
    }

    // TODO: Implement dir / single file argv. For now assuming it s a jack file
    FILE *jack_file = fopen(argv[1], "r");
    if (jack_file == NULL) {
        fprintf(stderr, "Error: Could not open file");
        return 1;
    }

    parse_file(jack_file);
    return 0;
}