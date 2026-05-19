#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <sys/stat.h>
#include <dirent.h>

#include "parser.h"
#include "symbol_table.h"

int static_index = 0;
int field_index = 0;
int var_index = 0;
int arg_index = 0;
char *class_name;
int while_index = 0;
char *current_subroutine_name = NULL;
int if_index = 0;

const char *keywords[] = {"class", "constructor", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return", "function"};
int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

bool is_line_overflow(char* buf, FILE *jack_file) {
    if (strchr(buf, '\n') == NULL && !feof(jack_file)) {
        return true;
    }

    return false;
}



bool is_valid_type(CurrentInstr current_instr) {
    if (strcmp(current_instr.type, "identifier") == 0
    || strcmp(current_instr.value, "boolean" ) == 0
    || strcmp(current_instr.value, "int") == 0
    || strcmp(current_instr.value, "char") == 0) {
        return true;
    }

    return false;
}

char *get_kind(Kind kind) {
    if (kind == K_ARG) {
        return "argument"; 
    } else if (kind == K_VAR) {
        return "local";
    } else if (kind == K_STATIC) {
        return "static";
    }
    // TODO What about Field?
    return "unknown"; 
}

CurrentInstr compile_parameter_list(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node, FILE *vm_file) {
    int param_n = 0;
    char buffer[256];
    xmlNodePtr paramlist_node = xmlNewChild(root_node, NULL, BAD_CAST "parameterList", BAD_CAST "");
    while (strcmp(current_instr.value, ")") != 0) {
        param_n++;
        if (is_valid_type(current_instr)) {
            char *type = current_instr.value;
            xmlNewChild(paramlist_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Error: Missing identifier after type in param list.\n");
                break;
            }
            Identifier *ident = get_ident(current_instr.value, subroutine_table);
            if (ident == NULL || ident->kind != K_ARG) {
                insert_ident(current_instr.value, type, arg_index, K_ARG, subroutine_table);
                arg_index++;
            }

            ident = get_ident(current_instr.value, subroutine_table);
            snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
                ident->name, ident->type, ident->kind, ident->kind_index);              
            xmlNewChild(paramlist_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            xmlNewChild(paramlist_node, NULL, BAD_CAST "p11-declaration", BAD_CAST buffer);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, ",") == 0) {
                xmlNewChild(paramlist_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
            }
        } else {
            fprintf(stderr, "Error: Paramlist invalid type.\n");
            break;
        }
    }
    if (paramlist_node->children == NULL) {
        xmlNodeSetContent(paramlist_node, BAD_CAST "\n");
    }

    return current_instr;
}

CurrentInstr compile_var_dec(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {
    char buffer[256];
    xmlNodePtr var_dec_node = xmlNewChild(root_node, NULL, BAD_CAST "varDec", BAD_CAST "");
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    if (!is_valid_type(current_instr)) {
        fprintf(stderr, "Error: Wrong type in vardec\n");
        return current_instr;
    }
    char *type = current_instr.value;
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.type, "identifier") == 0) {
        Identifier *ident = get_ident(current_instr.value, subroutine_table);
        if (ident == NULL || ident->kind != K_VAR) {
            insert_ident(current_instr.value, type, var_index, K_VAR, subroutine_table);
            var_index++;
        }

        ident = get_ident(current_instr.value, subroutine_table);
        snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
             ident->name, ident->type, ident->kind, ident->kind_index);        
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        xmlNewChild(var_dec_node, NULL, BAD_CAST "p11-declaration", BAD_CAST buffer);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.value, ",") == 0) {
            xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);           
        }
    }
    if (strcmp(current_instr.value, ";") != 0) {
        fprintf(stderr, "Error: Missing semicolon varDec\n");
        return current_instr;
    }
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    
    return current_instr;
}

bool is_statement(CurrentInstr current_instr) {
    if (strcmp(current_instr.value, "let") == 0 ||
    strcmp(current_instr.value, "if") == 0 ||
    strcmp(current_instr.value, "while") == 0 ||
    strcmp(current_instr.value, "do") == 0 ||
    strcmp(current_instr.value, "return") == 0) {
        return true;
    }
    return false;
}

bool is_operator(const char* val) {
    if (val == NULL) return false;

    if (strstr(val, "&lt;") || strstr(val, "&gt;") || strstr(val, "&amp;")) {
        return true;
    }

    if (strlen(val) >= 1 && strchr("+-*/|=", val[0])) {
        return true;
    }

    return false;
}

bool is_keyword_constant(CurrentInstr instr) {
    if (instr.type == NULL || instr.value == NULL) return false;
    
    if (strcmp(instr.type, "keyword") != 0) return false;

    if (strcmp(instr.value, "true") == 0 ||
        strcmp(instr.value, "false") == 0 ||
        strcmp(instr.value, "null") == 0 ||
        strcmp(instr.value, "this") == 0) {
        return true;
    }

    return false;
}

CurrentInstr compile_expression_node(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node, FILE *vm_file, bool is_return);
CurrentInstr compile_term(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node, FILE *vm_file) {
    char buffer[256];
    xmlNodePtr term_node = xmlNewChild(node, NULL, BAD_CAST "term", BAD_CAST "");
    if (strcmp(current_instr.value, "~") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        current_instr = compile_term(jack_file, current_instr, term_node, vm_file);
        fprintf(vm_file, "not\n");
        return current_instr;
    } else if (strcmp(current_instr.value, "-") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        current_instr = compile_term(jack_file, current_instr, term_node, vm_file);
        fprintf(vm_file, "neg\n");
        return current_instr;
    } 
    else if (strcmp(current_instr.value, "(") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        current_instr = compile_expression_node(jack_file, current_instr, term_node, vm_file, false);

        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        return advance_parser(jack_file); 
    }

    else if (strcmp(current_instr.type, "identifier") == 0) {
        char *base = current_instr.value;
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        Identifier *ident = get_ident(current_instr.value, subroutine_table);
        if (ident == NULL) {
            ident = get_ident(current_instr.value, class_table);
        }
        if (ident != NULL) {
            snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
            ident->name, ident->type, ident->kind, ident->kind_index);
            xmlNewChild(term_node, NULL, BAD_CAST "p11-usage", BAD_CAST buffer);
        }
        current_instr = advance_parser(jack_file);

        if (strcmp(current_instr.value, "[") == 0) {
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, term_node, vm_file, false);
            
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            return advance_parser(jack_file);
        } 
        else if (strcmp(current_instr.value, ".") == 0 || strcmp(current_instr.value, "(") == 0) {
            char *point_ext = NULL;
            if (strcmp(current_instr.value, ".") == 0) {
                xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.type, "identifier") != 0) {
                    fprintf(stderr, "Error: Missing identifier after . in compile term\n");
                    return current_instr;
                }
                point_ext = current_instr.value;

                xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.value, "(") != 0) {
                    fprintf(stderr, "Error: Missing ( in term . \n");
                    return current_instr;
                }

            }
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);

            current_instr = advance_parser(jack_file);
            xmlNodePtr expr_list_node = xmlNewChild(term_node, NULL, BAD_CAST "expressionList", BAD_CAST "");
            int args_n = 0;
            if (strcmp(current_instr.value, ")") != 0) {
                args_n++;
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node, vm_file, false);
            }
            while (strcmp(current_instr.value, ",") == 0) {
                xmlNewChild(expr_list_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node, vm_file, false);
            }
            fprintf(vm_file, "call %s", base);
            if (point_ext != NULL) {
                fprintf(vm_file, ".%s", point_ext);
            }
            fprintf(vm_file, " %d\n", args_n);
            if (expr_list_node->children == NULL) {
                xmlNodeSetContent(expr_list_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, ")") != 0) {
                fprintf(stderr, "Error: No closing ) for expr list in term\n");
                return current_instr;
            }
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);

            return current_instr;
            
        } else {
            if (ident != NULL) {
                fprintf(vm_file, "push %s %d\n", get_kind(ident->kind), ident->kind_index);
            }
            return current_instr;
        }
    }

    else {
        //@VM-OPERRATION
        if (strcmp(current_instr.type, "integerConstant") == 0) {
          fprintf(vm_file, "push constant %s\n", current_instr.value);

        }
        if (strcmp(current_instr.value, "true") == 0 || strcmp(current_instr.value, "false") == 0) {
            //@VM-OPERATION
            fprintf(vm_file, "push constant 0\n");
        }
        if (strcmp(current_instr.value, "true") == 0) {
            //@VM-OPERATION
            fprintf(vm_file, "not\n");
        }
        if (strcmp(current_instr.type, "stringConstant") == 0 ||
            strcmp(current_instr.type, "integerConstant") == 0 ||
            is_keyword_constant(current_instr)) {
            Identifier *ident = get_ident(current_instr.value, subroutine_table);
            if (ident != NULL) {
                snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
                ident->name, ident->type, ident->kind, ident->kind_index);
                xmlNewChild(term_node, NULL, BAD_CAST "p11-usage", BAD_CAST buffer);
            }    
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            return advance_parser(jack_file);
        }
    }

    return current_instr;
}

CurrentInstr compile_expression_node(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node, FILE *vm_file, bool is_return) {
    xmlNodePtr expr_node = xmlNewChild(node, NULL, BAD_CAST "expression", BAD_CAST "");  
    current_instr = compile_term(jack_file, current_instr, expr_node, vm_file);
    if (expr_node->children->children == NULL) {
        xmlUnlinkNode(expr_node);
        xmlFreeNode(expr_node);
        //@VM-OPERATION
        if (is_return) {
            fprintf(vm_file, "push constant 0\n");
        }
        return current_instr;
    }
    //@VM-OPERATION
    int i = 0;
    char *ops[256];
    while (is_operator(current_instr.value)) {
            ops[i] = current_instr.value;
            i++;
            xmlNewChild(expr_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_term(jack_file, current_instr, expr_node, vm_file);
    }
    //@VM-OPERATION
    // TODO: Handle others
    //@VM-OPERATION
        for (int j = 0; j < i; j++) {
            if (strcmp(ops[j], "+") == 0) {
                fprintf(vm_file, "add\n");
            } 
            else if (strcmp(ops[j], "-") == 0) {
                fprintf(vm_file, "sub\n");
            } 
            else if (strcmp(ops[j], "*") == 0) {
                fprintf(vm_file, "call Math.multiply 2\n");
            } 
            else if (strcmp(ops[j], "/") == 0) {
                fprintf(vm_file, "call Math.divide 2\n");
            } 
            else if (strstr(ops[j], "lt;") != NULL) {
                fprintf(vm_file, "lt\n");
            } 
            else if (strstr(ops[j], "gt;") != NULL) {
                fprintf(vm_file, "gt\n");
            } 
            else if (strcmp(ops[j], "=") == 0) {
                fprintf(vm_file, "eq\n");
            } 
            else if (strstr(ops[j], "amp;") != NULL) {
                fprintf(vm_file, "and\n");
            } 
            else if (strcmp(ops[j], "|") == 0) {
                fprintf(vm_file, "or\n");
            }
        }
    return current_instr;
}

CurrentInstr compile_statements(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node, FILE *vm_file) {
    char buffer[256];
    bool has_return = false;
    while(is_statement(current_instr)) {
        if (strcmp(current_instr.value, "let") == 0) {
            xmlNodePtr let_node = xmlNewChild(root_node, NULL, BAD_CAST "letStatement", BAD_CAST "");    
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Missing identifier let statement\n");
                return current_instr;
            }

            Identifier *ident = get_ident(current_instr.value, subroutine_table);
            if (ident == NULL) {
                ident = get_ident(current_instr.value, class_table);
            }
            if (ident != NULL) {
                snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
                ident->name, ident->type, ident->kind, ident->kind_index);
                xmlNewChild(let_node, NULL, BAD_CAST "p11-usage", BAD_CAST buffer);

            } else {
                fprintf(stderr, "Error: Missing table identifier let statement\n");
            }

            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, "[") == 0) {
                xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, let_node, vm_file, false);
                
                xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
            } 
            if (strcmp(current_instr.value, "=") != 0) {
                fprintf(stderr, "Error missing equal sign let statement\n");
                return current_instr;
            }            
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, let_node, vm_file, false);    
            if (strcmp(current_instr.value, ";") != 0) {
                fprintf(stderr, "Error: Missing semicolon end of let statement\n");
                return current_instr;
            }
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            fprintf(vm_file, "pop %s %d\n", get_kind(ident->kind), ident->kind_index);
            current_instr = advance_parser(jack_file);
        } else if (strcmp(current_instr.value, "do") == 0) {
            char *do_class_name = NULL;
            char *do_method_name = NULL;
            xmlNodePtr do_node = xmlNewChild(root_node, NULL, BAD_CAST "doStatement", BAD_CAST "");    
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Missing identifier do statement\n");
                return current_instr;
            }
            do_class_name = current_instr.value;
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);

            if (strcmp(current_instr.value, ".") == 0) {
                xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.type, "identifier") != 0) {
                    fprintf(stderr, "Missing identifier after point doStatement\n");
                    return current_instr;
                }
                do_method_name = current_instr.value;
                xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
            }

            if (strcmp(current_instr.value, "(") != 0) {
                fprintf(stderr, "Missing opening bracket do statement\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            xmlNodePtr expr_list_node = xmlNewChild(do_node, NULL, BAD_CAST "expressionList", BAD_CAST "");
            int args_n = 0;
            if (strcmp(current_instr.value, ")") != 0) {
                args_n++;
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node, vm_file, false);
            }
            while (strcmp(current_instr.value, ",") == 0) {
                args_n++;
                xmlNewChild(expr_list_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node, vm_file, false);

            }

            if (expr_list_node->children == NULL) {
                xmlNodeSetContent(expr_list_node, BAD_CAST "\n");
            }
            Identifier *ident = get_ident(do_class_name, subroutine_table);
            if (ident == NULL) {
                ident = get_ident(do_class_name, class_table);
            }
            if (ident != NULL) {
                snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
                ident->name, ident->type, ident->kind, ident->kind_index);
                xmlNewChild(do_node, NULL, BAD_CAST "p11-usage", BAD_CAST buffer);

            } else {
                // @VM-OPERATION
                if (do_method_name == NULL) {
                    fprintf(vm_file, "call %s %d\n", do_class_name, args_n);
                } else {
                    fprintf(vm_file, "call %s.%s %d\n", do_class_name, do_method_name, args_n);
                }
                fprintf(vm_file, "pop temp 0\n");
            }

            if (strcmp(current_instr.value, ")") != 0) {
                fprintf(stderr, "Error: No closing ) for expr list in dostatement\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, ";") != 0) {
                fprintf(stderr, "Error: Missing semicolon do statment\n");
                return current_instr;
            }


            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
        } else if(strcmp(current_instr.value, "return") == 0) {
            has_return = true;
            xmlNodePtr return_node = xmlNewChild(root_node, NULL, BAD_CAST "returnStatement", BAD_CAST "");
            xmlNewChild(return_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, return_node, vm_file, true);
            if (strcmp(current_instr.value, ";") != 0) {
                fprintf(stderr, "Error: Missing semicolon return statement\n");
                return current_instr;
            }
            fprintf(vm_file, "return\n");
            xmlNewChild(return_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
        } else if(strcmp(current_instr.value, "if") == 0) {
            xmlNodePtr if_node = xmlNewChild(root_node, NULL, BAD_CAST "ifStatement", BAD_CAST "");    
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);

            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, "(") != 0) {
                fprintf(stderr, "Error: Missing ( in ifStatement if block\n");
                return current_instr;
            }   
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);      
            current_instr = compile_expression_node(jack_file, current_instr, if_node, vm_file, false);
            if (strcmp(current_instr.value, ")") != 0) {
                fprintf(stderr, "Error: Missing ) in ifStatement if block\n");
                return current_instr;                
            }
            int local_if_index = if_index;
            if_index++;
            //@VM-OPERATION
            fprintf(vm_file, "if-goto IF_BRANCH_%d\n", local_if_index);
            fprintf(vm_file, "goto ELSE_BRANCH_%d\n", local_if_index);
            fprintf(vm_file, "label IF_BRANCH_%d\n", local_if_index);
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            
            if (strcmp(current_instr.value, "{") != 0) {
                fprintf(stderr, "Error: Missing { in ifStatement if block\n");
                return current_instr;                    
            }
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            xmlNodePtr if_statement_node = xmlNewChild(if_node, NULL, BAD_CAST "statements", BAD_CAST "");    
            current_instr = compile_statements(jack_file, current_instr, if_statement_node, vm_file);
            if (if_statement_node->children == NULL) {
                xmlNodeSetContent(if_statement_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, "}") != 0) {
                fprintf(stderr, "Error: Missing } in ifStatement if block\n");
                return current_instr;                  
            }
            //@VM-OPERATION
            fprintf(vm_file, "goto IF_END_%d\n", local_if_index);
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, "else") == 0) {

                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                //@VM-OPERATION
                fprintf(vm_file, "label ELSE_BRANCH_%d\n", local_if_index);
                if (strcmp(current_instr.value, "{") != 0) {
                    fprintf(stderr, "Error: Missing { else block\n");
                    return current_instr;
                }
                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                xmlNodePtr else_statement_node = xmlNewChild(if_node, NULL, BAD_CAST "statements", BAD_CAST "");    
                current_instr = compile_statements(jack_file, current_instr, else_statement_node, vm_file);
                if (else_statement_node->children == NULL) {
                    xmlNodeSetContent(else_statement_node, BAD_CAST "\n");
                }
                if (strcmp(current_instr.value, "}") != 0) {
                    fprintf(stderr, "Error: Missing } in ifStatement if block\n");
                    return current_instr;                  
                }  
                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);                              
            }     
            //@VM-OPERATION
            fprintf(vm_file, "label IF_END_%d\n", local_if_index);   
        } else if (strcmp(current_instr.value, "while") == 0) {
            // 'while' '(' expression ')' '{' statements '}'
            xmlNodePtr while_node = xmlNewChild(root_node, NULL, BAD_CAST "whileStatement", BAD_CAST "");    
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, "(") != 0) {
                fprintf(stderr, "Error: Missing ( in whileStatement\n");
                return current_instr;
            }  
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file); 
            //@VM-OPERATION  
            // TODO: L1 -> compiled expression -> not -> if-goto L2-> compiled statements -> goto L1 -> L2 
            fprintf(vm_file, "label WhileStart.%s.%s.%d\n", class_name, current_subroutine_name, while_index); 
            current_instr = compile_expression_node(jack_file, current_instr, while_node, vm_file, false);
            if (strcmp(current_instr.value, ")") != 0) {
                fprintf(stderr, "Error: Missing ) in whileStatement\n");
                return current_instr;                
            } 
            //@VM-OPERATION
            fprintf(vm_file, "not\nif-goto WhileEnd.%s.%s.%d\n", class_name, current_subroutine_name, while_index);
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  

            if (strcmp(current_instr.value, "{") != 0) {
                fprintf(stderr, "Error: Missing { in whileStatement\n");
                return current_instr;                    
            }
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            xmlNodePtr while_statements_node = xmlNewChild(while_node, NULL, BAD_CAST "statements", BAD_CAST "");    
            current_instr = compile_statements(jack_file, current_instr, while_statements_node, vm_file);
            if (while_statements_node->children == NULL) {
                xmlNodeSetContent(while_statements_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, "}") != 0) {
                fprintf(stderr, "Error: Missing } in while statement block\n");
                return current_instr;                  
            }
            fprintf(vm_file, "goto WhileStart.%s.%s.%d\n", class_name, current_subroutine_name, while_index); 
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            fprintf(vm_file, "label WhileEnd.%s.%s.%d\n", class_name, current_subroutine_name, while_index); 
            while_index++;
        } else {
            current_instr = advance_parser(jack_file);
        }
    }

    return current_instr;
}

CurrentInstr compile_subroutine_body(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node, FILE *vm_file) {  
    if (strcmp(current_instr.value, "{") != 0) {
        fprintf(stderr, "Error: Missing opening Braces subroutine body.\n");
        return current_instr;        
    }
    xmlNodePtr body_node = xmlNewChild(root_node, NULL, BAD_CAST "subroutineBody", BAD_CAST "");
    xmlNewChild(body_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, "var") == 0) {
    current_instr = compile_var_dec(jack_file, current_instr, body_node);
    }
    fprintf(vm_file, "%d\n", var_index);

    xmlNodePtr statement_node = xmlNewChild(body_node, NULL, BAD_CAST "statements", BAD_CAST "");
    current_instr = compile_statements(jack_file, current_instr, statement_node, vm_file);
    if (statement_node->children == NULL) {
        xmlNodeSetContent(statement_node, BAD_CAST "\n");
    }
    if (strcmp(current_instr.value, "}") != 0) {
        fprintf(stderr, "Error: Missing closing brace for subroutine body\n");
        return current_instr;
    }
    xmlNewChild(body_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    return advance_parser(jack_file);
}

CurrentInstr compile_subroutine(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node, FILE *vm_file) {
    var_index = 0;
    arg_index = 0;
    reset_ident_table(subroutine_table);
    while_index = 0;
    current_subroutine_name = NULL;
    bool is_constructor = strcmp(current_instr.value, "constructor") == 0;
    bool is_method = strcmp(current_instr.value, "method") == 0;
    xmlNodePtr subroutine_node = xmlNewChild(root_node, NULL, BAD_CAST "subroutineDec", BAD_CAST "");
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    // @VM-OPERATION
    fprintf(vm_file, "%s ", current_instr.value);
    current_instr = advance_parser(jack_file);
    if (is_method) {
        insert_ident("this", class_name, arg_index, K_ARG, subroutine_table);
        arg_index++;
        char buffer[256];
        Identifier *ident = get_ident("this", subroutine_table);
        snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
             ident->name, ident->type, ident->kind, ident->kind_index);
                 
        xmlNewChild(subroutine_node, NULL, BAD_CAST "p11-declaration", BAD_CAST buffer);
    } 
    if (is_constructor) {
        // TODO for methods: Add this to the symbol table: name: this, type: ClassName, kind: K_ARG, index: 0
        if (strcmp(current_instr.type, "identifier") != 0) {
            fprintf(stderr, "Error: Missing subroutine identifier.\n");
            return current_instr;
        } 
        char *constr_ident = current_instr.value;
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.value, "new") != 0) {
            fprintf(stderr, "Error: Missing new keyword constructor. val=%s\n", current_instr.value);
            return current_instr;
        }
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        asprintf(&current_subroutine_name, "%s.new", constr_ident);
        current_instr = advance_parser(jack_file);

    } else {
        if (!is_valid_type(current_instr) && strcmp(current_instr.value, "void") != 0) {
            fprintf(stderr, "Error: Invalid subroutine return type. val=%s\n", current_instr.value);
            return current_instr;
        }
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.type, "identifier") != 0) {
            fprintf(stderr, "Error: Missing subroutine identifier.\n");
            return current_instr;
        } 
        // @VM-OPERATION
        fprintf(vm_file, "%s.%s ", class_name, current_instr.value);
        current_subroutine_name = current_instr.value;
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
    }

    if (strcmp(current_instr.value, "(") != 0) {
        fprintf(stderr, "Error: Missing opening Paranthesis parameterlist.\n");
        return current_instr;
    }
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    current_instr = compile_parameter_list(jack_file, current_instr, subroutine_node, vm_file);   
    if (strcmp(current_instr.value, ")") != 0) {
        fprintf(stderr, "Error: Missing closing Paranthesis parameterlist.\n");
        return current_instr;       
    }
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    current_instr = compile_subroutine_body(jack_file, current_instr, subroutine_node, vm_file);
    return current_instr;
}

CurrentInstr compile_class_var_dec(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node, Kind kind) {
    char buffer[256];
    xmlNodePtr var_dec_node = xmlNewChild(root_node, NULL, BAD_CAST "classVarDec", BAD_CAST "");
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    if (!is_valid_type(current_instr)) {
        fprintf(stderr, "Error: static or field missing type. type=%s, val=%s\n", current_instr.type, current_instr.value);
        return current_instr;
    }
    char *type = current_instr.value;
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.type, "identifier") == 0) {
        Identifier *ident = get_ident(current_instr.value, class_table);
        if (kind == K_STATIC) {
            if (ident == NULL || ident->kind != K_STATIC) {
                insert_ident(current_instr.value, type, static_index, kind, class_table);
                static_index++;
            }
        } else if (kind == K_FIELD) {
            if (ident == NULL || ident->kind != K_FIELD) {
                insert_ident(current_instr.value, type, field_index, kind, class_table);
                field_index++;
            }
        } else {
            fprintf(stderr, "Warn: Declared static or field identifier twice.\n");
        }
        ident = get_ident(current_instr.value, class_table);
        snprintf(buffer, sizeof(buffer), "name: %s, type: %s, kind: %d, index: %d", 
             ident->name, ident->type, ident->kind, ident->kind_index);
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        xmlNewChild(var_dec_node, NULL, BAD_CAST "p11-declaration", BAD_CAST buffer);
        current_instr = advance_parser(jack_file);
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
    }

    return current_instr;
}

void compile_class(FILE *jack_file, xmlNodePtr node) {
    // Check for class keyword
    CurrentInstr current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, "class") != 0) {
        fprintf(stderr, "Error: File does not start with class keyword.\n");
        return;
    } 
    
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.type, "identifier") != 0) {
        fprintf(stderr, "Error: Missing class identifier.\n");
        return;
    }
    class_name = current_instr.value;
    char vm_name[strlen(class_name) + 18];
    snprintf(vm_name, sizeof(vm_name), "./test_output/%s.vm", class_name);
    FILE *vm_file = fopen(vm_name, "w");
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, "{") != 0) {
        fprintf(stderr, "Error: Missing class opening braces.\n");
        return;
    }
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, "static") == 0 
    || strcmp(current_instr.value, "field") == 0) {
        current_instr = compile_class_var_dec(jack_file, current_instr, node, strcmp(current_instr.value, "static") == 0 ? K_STATIC : K_FIELD);
    }   
    
    while (strcmp(current_instr.value, "function") == 0 || 
    strcmp(current_instr.value, "method") == 0 || 
    strcmp(current_instr.value, "constructor") == 0) {
        current_instr = compile_subroutine(jack_file, current_instr, node, vm_file);
    }
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
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
    static_index = 0;
    field_index = 0;
    class_name = NULL;
    compile_class(jack_file, root_node);


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