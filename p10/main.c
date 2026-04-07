#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

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
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "UserList");
    xmlDocSetRootElement(doc, root_node);
    xmlNodePtr user_node = xmlNewChild(root_node, NULL, BAD_CAST "User", NULL);
    xmlNewProp(user_node, BAD_CAST "id", BAD_CAST "1");
    xmlNewChild(user_node, NULL, BAD_CAST "Name", BAD_CAST "John Doe");
    xmlNewChild(user_node, NULL, BAD_CAST "Role", BAD_CAST "Developer");
    xmlSaveFormatFileEnc("users.xml", doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    //strchr("{}()[].,;+-*/&|<>=~", c) != NULL;
    char current_instr[2048];
    int i;
    int c;
    bool is_single_line_comment = false;
    bool is_multi_line_comment = false;
    // use ungetc?
    while ((c = fgetc(jack_file)) != EOF) {
        if (is_multi_line_comment && c == '*') {
            int next = fgetc(jack_file);
            if (next == '/') {
                is_multi_line_comment = false;
                continue;
            } else {
                ungetc(next, jack_file);
                continue;
            }
        }
        if (is_single_line_comment) {
            if (c == '\n') {
                is_single_line_comment = false;
            }
            continue;
        }
        if (c == '/') {
            int next = fgetc(jack_file);
            if (next == '/') {
                is_single_line_comment = true;
                // TODO: Process current_instr. Reset current_instr
                continue;
            } else if (next == '*') {
                is_multi_line_comment = true;
                // TODO: Process current_isntr. Reset current_instr;
                continue;
            } else {
                ungetc(next, jack_file);
                // TODO: c = division. Handle it
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