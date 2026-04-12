#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <sys/stat.h>
#include <dirent.h>

#define HASH_SIZE 101

typedef struct CurrentInstr {
    char *type;
    char *value;
} CurrentInstr;

bool is_int(const char *str) {
    char *endptr;
    if (*str == '\0') {
        return false;
    }
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

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

void write_identifier_or_integer() {

}

CurrentInstr advance_parser(FILE *jack_file)
{
    CurrentInstr resp = {0};
    char current_instr[2048] = {0};
    char tmp[2050] = {0};
    int i = 0;
    int c;
    bool is_single_line_comment = false;
    bool is_multi_line_comment = false;
    bool is_string = false;
    while ((c = fgetc(jack_file)) != EOF)
    {
        if (is_string)
        {
            if (c == '"')
            {
                snprintf(tmp, sizeof(tmp), " %s ", current_instr);
                resp.type = "stringConstant";
                resp.value = tmp;
                return resp;
            }
            else
            {
                current_instr[i] = c;
                current_instr[i + 1] = '\0';
                i++;
            }

            continue;
        }
        if (c == '\n')
        {
            is_single_line_comment = false;
            current_instr[0] = '\0';
            i = 0;
            continue;
        }
        else
        {
            if (is_single_line_comment)
            {
                continue;
            }
        }
        if (is_multi_line_comment)
        {
            if (c == '*')
            {
                int next = fgetc(jack_file);
                if (next == '/')
                {
                    is_multi_line_comment = false;
                    current_instr[0] = '\0';
                    i = 0;
                    continue;
                }
                else
                {
                    ungetc(next, jack_file);
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        if (c == '"')
        {
            is_string = true;
            continue;
        }

        if (c == '/')
        {
            int next = fgetc(jack_file);
            if (next == '/')
            {
                is_single_line_comment = true;
                // TODO: Evaluate current_instr reset
                if (i > 0)
                {
                    Node *node = get_node(current_instr);
                    snprintf(tmp, sizeof(tmp), " %s ", current_instr);
                    if (node != NULL)
                    {
                        resp.type = "keyword";
                    }
                    else
                    {
                        if (is_int(current_instr))
                        {
                            resp.type = "integerConstant";
                        }
                        else
                        {
                            resp.type = "identifier";
                        }
                    }
                    ungetc(next, jack_file);
                    ungetc(c, jack_file);
                    resp.type = tmp;
                    return resp;
                }
                continue;
            }
            else if (next == '*')
            {
                is_multi_line_comment = true;
                // TODO: Evaluate current_instr reset
                if (i > 0)
                {
                    Node *node = get_node(current_instr);
                    snprintf(tmp, sizeof(tmp), " %s ", current_instr);
                    if (node != NULL)
                    {
                        resp.type = "keyword";
                    }
                    else
                    {
                        if (is_int(current_instr))
                        {
                            resp.type = "integerConstant";
                        }
                        else
                        {
                            resp.type = "identifier";
                        }
                    }
                    ungetc(next, jack_file);
                    ungetc(c, jack_file);
                    resp.value = tmp;
                    return resp;
                }
                continue;
            }
            else
            {
                ungetc(next, jack_file);
                resp.type = "symbol";
                resp.value = " / ";
                return resp;
            }
        }
        if (isspace(c))
        {
            // TODO: Evaluate current_instr reset
            if (i > 0)
            {
                Node *node = get_node(current_instr);
                snprintf(tmp, sizeof(tmp), " %s ", current_instr);
                if (node != NULL)
                {
                    resp.type = "keyword";
                }
                else
                {
                    if (is_int(current_instr))
                    {
                        resp.type = "integerConstant";
                    }
                    else
                    {
                        resp.type = "identifier";
                    }
                }
                resp.value = tmp;
                return resp;
            }
            continue;
        }

        if (strchr("{}()[].,;+-*/&|<>=~", c) != NULL)
        {
            if (i > 0)
            {
                // TODO: Evaluate current_isntr reset
                Node *node = get_node(current_instr);
                snprintf(tmp, sizeof(tmp), " %s ", current_instr);
                if (node != NULL)
                {
                    resp.type = "keyword";
                }
                else
                {
                    if (is_int(current_instr))
                    {
                        resp.type = "integerConstant";
                    }
                    else
                    {
                        resp.type = "identifier";
                    }
                }
                ungetc(c, jack_file);
                resp.value = tmp;
                return resp;
            }
            resp.type = "symbol";
            if (c == '&')
            {
                resp.value = " &amp; ";
            }
            else if (c == '<')
            {
                resp.value = " &lt; ";
            }
            else if (c == '>')
            {
                resp.value = " &gt; ";
            }
            else
            {
                char symbol_str[4] = {' ', (char)c, ' ', '\0'};
                resp.value = symbol_str;
            }
            return resp;
        }

        current_instr[i] = c;
        current_instr[i + 1] = '\0';
        i++;
    }
    return resp;
}

void parse_file(FILE *jack_file, char* xml_t_ident) {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "tokens");
    xmlDocSetRootElement(doc, root_node);
    // child to root /parent to other nodes xmlNodePtr user_node = xmlNewChild(root_node, NULL, BAD_CAST "User", NULL);
    // just a child xmlNewChild(user_node, NULL, BAD_CAST "Name", BAD_CAST "John Doe");

    //strchr("{}()[].,;+-*/&|<>=~", c) != NULL;
    bool has_tokens = true;
    while (has_tokens) {
        CurrentInstr current_instr = advance_parser(jack_file);
        if (current_instr.type != NULL)
            xmlNewChild(root_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        else 
            has_tokens = false;
    }

    xmlSaveCtxtPtr ctxt = xmlSaveToFilename(xml_t_ident, NULL, XML_SAVE_NO_DECL | XML_SAVE_FORMAT);
    if (ctxt) {
        xmlSaveDoc(ctxt, doc);
        xmlSaveClose(ctxt);
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();

}

void process_file(char *argv)
{
    FILE *jack_file = fopen(strdup(argv), "r");
    char *filename_identifier = strrchr(strdup(argv), '/');
    // If null, the arg is probably Basic.vm instead of ./Basic.vm. filename_identifier points to /. So we only need to advance if present.
    if (filename_identifier != NULL) {
        filename_identifier++;
    } else {
        filename_identifier = strdup(argv);
    }
    char *dot = strrchr(filename_identifier, '.');
    if (dot && (strcmp(dot, ".jack") == 0)) {
        *dot = '\0';
    } else {
        return;
    }
    char xml_t_ident[strlen(filename_identifier) + 6];
    snprintf(xml_t_ident, sizeof(xml_t_ident), "%sT.xml", filename_identifier);
    if (jack_file == NULL) {
        return;
    }
    parse_file(jack_file, xml_t_ident);
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < num_keywords; i++) {
        insert_node(keywords[i]);
    }

    int status;
    struct stat st_buf;
    const char *iptr = argv[1];
    status = stat(iptr, &st_buf);
    if (status != 0) {
        fprintf(stderr, "Error: Wrong status");
        return 1;
    }

    if (S_ISDIR(st_buf.st_mode)) {
        DIR *dir = opendir(argv[1]);
        if (dir == NULL) {
            return 1;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", argv[1], entry->d_name);
            process_file(full_path);
        }
    } else if (S_ISREG(st_buf.st_mode)) {
        process_file(argv[1]);
    }

    return 0;
}