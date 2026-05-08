#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"

#define HASH_SIZE 101

static unsigned long hash(const char *str) {
    unsigned long hash_num = 5381;
    int c;
    while (c = *str++) {
        hash_num = ((hash_num << 5) + hash_num) + c;
    }

    return hash_num;
}

bool is_int(const char *str) {
    char *endptr;
    if (*str == '\0') {
        return false;
    }
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

Node *hash_table[HASH_SIZE] = {NULL};

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

void insert_node(const char *key)  {
    unsigned long hash_num = hash(key);
    int index = hash_num % HASH_SIZE;

    Node *new_node = malloc(sizeof(Node));

    new_node->key = strdup(key);
    new_node->next = hash_table[index];
    hash_table[index] = new_node;
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
                resp.value = strdup(tmp);
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
                    resp.value = strdup(tmp);
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
                    resp.value = strdup(tmp);
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
                resp.value = strdup(tmp);
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
                resp.value = strdup(tmp);
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
                resp.value = strdup(symbol_str);
            }
            return resp;
        }

        current_instr[i] = c;
        current_instr[i + 1] = '\0';
        i++;
    }
    return resp;
}