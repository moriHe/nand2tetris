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
// TODO: free CurrentInstr when used
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

// TODO: field / static uses almost the same code. Can be united
void compile_parameter_list(FILE *jack_file, xmlNodePtr node, CurrentInstr current_instr) {
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    xmlNodePtr param_list_node = xmlNewChild(node, NULL, BAD_CAST "parameterList", BAD_CAST "");
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, " ) ") != 0) {
        if (
        strcmp(current_instr.type, "identifier") != 0 
        && strcmp(current_instr.value, " boolean ") != 0 
        && strcmp(current_instr.value, " int ") != 0 
        && strcmp(current_instr.value, " char ") != 0
        ) {
            fprintf(stderr, "Error: Parameter List not followed by correct type\n");
            return;
        }
        xmlNewChild(param_list_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.type, "identifier") != 0) {
            fprintf(stderr, "Error: Parm List no identifier after type.\n");
            return;
        }
        xmlNewChild(param_list_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));

        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.value, " , ") == 0) {
            xmlNewChild(param_list_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
            current_instr = advance_parser(jack_file);
        }
    }

    if (strcmp(current_instr.value, " ) ") != 0) {
        fprintf(stderr, "Error: Param List not closed\n");
        return;
    }

    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
}

void compile_subroutine_dec(FILE *jack_file, xmlNodePtr root_node, CurrentInstr current_instr) {
        char *subr_val = current_instr.value;
        xmlNodePtr subr_node = xmlNewChild(root_node, NULL, BAD_CAST "subroutineDec", BAD_CAST "");
        xmlNewChild(subr_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));

        current_instr = advance_parser(jack_file);
        if (strcmp(subr_val, " function ") == 0 || strcmp(subr_val, " method ") == 0) {
            if (strcmp(current_instr.type, "keyword") != 0) {
                fprintf(stderr, "Error: Missing keyword after fn or method.\n");
                return;
            }
            xmlNewChild(subr_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
            current_instr = advance_parser(jack_file);

        }

        if (strcmp(current_instr.type, "identifier") != 0) {
            printf("val=%s\n", current_instr.value);
            fprintf(stderr, "Error: Missing identifier in subroutine declaration.\n");
            return;
        }
        xmlNewChild(subr_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));

        current_instr = advance_parser(jack_file);
        if (strcmp(subr_val, " constructor ") == 0) {
            xmlNewChild(subr_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
            current_instr = advance_parser(jack_file);
        }

        if (strcmp(current_instr.value, " ( ") != 0) {
            fprintf(stderr, "Error: Missing opening paranthesis in subroutine decl.\n");
            return;
        }

        compile_parameter_list(jack_file, subr_node, current_instr);

}

void compile_class_var_dec(FILE *jack_file, xmlNodePtr root_node, CurrentInstr current_instr) {
        xmlNodePtr var_dec_node = xmlNewChild(root_node, NULL, BAD_CAST "classVarDec", BAD_CAST "");
        xmlNewChild(var_dec_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));

        current_instr = advance_parser(jack_file);
        if (
        strcmp(current_instr.type, "identifier") != 0 
        && strcmp(current_instr.value, " boolean ") != 0 
        && strcmp(current_instr.value, " int ") != 0 
        && strcmp(current_instr.value, " char ") != 0
        ) {
            fprintf(stderr, "Error: Field or static variable not followed by correct type");
            return;
        }

        xmlNewChild(var_dec_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));

        current_instr = advance_parser(jack_file);
        while (strcmp(current_instr.type, "identifier") == 0) {
            xmlNewChild(var_dec_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "symbol") == 0) {
                xmlNewChild(var_dec_node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
 
            } else {
                fprintf(stderr, "Error: missing symbol in field or static declaration.");
                return;
            }
            if (strcmp(current_instr.value, " , ") == 0) {
                current_instr = advance_parser(jack_file);
            }
        }

}

void compile_class(FILE *jack_file, xmlNodePtr node) {
    // Check for class keyword
    CurrentInstr current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, " class ") == 0) {
        xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    } else {
        fprintf(stderr, "Error: File does not start with class keyword");
        return;
    }

    // Check for class identifier
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.type, "identifier") != 0) {
        fprintf(stderr, "Error: No follow up identifier");
        return;
    } else {
        xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    }
    
    // Check for opening bracket
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, " { ") != 0) {
        fprintf(stderr, "Error: No opening bracket");
        return;
    } else {
        xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    }

    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, " static ") == 0 || strcmp(current_instr.value, " field ") == 0) {

        compile_class_var_dec(jack_file, node, current_instr);
        current_instr = advance_parser(jack_file);
    }

    while (strcmp(current_instr.value, " constructor ") == 0 ||strcmp(current_instr.value, " method ") == 0 || strcmp(current_instr.value, " function ") == 0) {
        compile_subroutine_dec(jack_file, node, current_instr);
        current_instr = advance_parser(jack_file);
    }

    // Done:
    // compile_class (kinda. I mean all other methods descent from class)
    // compile_class_var_dec
    // compile_subroutine
    // compile_parameter_list

    // TODOs: 
    // compile subroutineBody
    
    // compile_var_dec
    // compile_statements
        // compile_let, compile_if, compile_while, compile_do, compile_return

    // compile_expression (identifies a term and any subsquent operators)
    // compile_term (constants, vairables and array indexing)
    // compile_expression_list
    return;
}

void parse_file(FILE *jack_file, char* xml_t_ident) {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "class");
    if (root_node == NULL) {
        fprintf(stderr, "Failed to create class_node\n");
        return;
    }
    xmlDocSetRootElement(doc, root_node);
    compile_class(jack_file, root_node);

    /*
    bool has_tokens = true;
    while (has_tokens) {
        CurrentInstr current_instr = advance_parser(jack_file);
        if (current_instr.type != NULL)
            xmlNewChild(root_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        else 
            has_tokens = false;
    }
    */

    xmlSaveCtxtPtr ctxt = xmlSaveToFilename(xml_t_ident, NULL, XML_SAVE_NO_DECL | XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY);
    if (ctxt) {
        xmlSaveDoc(ctxt, doc);
        xmlSaveClose(ctxt);
    }
    xmlFreeDoc(doc);

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
    char xml_t_ident[strlen(filename_identifier) + 5];
    snprintf(xml_t_ident, sizeof(xml_t_ident), "%s.xml", filename_identifier);
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
    xmlCleanupParser();

    return 0;
}