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

const char *keywords[] = {"class", "constructor", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return", "function"};
int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

bool is_line_overflow(char* buf, FILE *jack_file) {
    if (strchr(buf, '\n') == NULL && !feof(jack_file)) {
        return true;
    }

    return false;
}

void write_identifier_or_integer() {

}



bool is_valid_type(CurrentInstr current_instr) {
    if (strcmp(current_instr.type, "identifier") == 0
    || strcmp(current_instr.value, " boolean " ) == 0
    || strcmp(current_instr.value, " int ") == 0
    || strcmp(current_instr.value, " char ") == 0) {
        return true;
    }

    return false;
}

CurrentInstr compile_parameter_list(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {
    xmlNodePtr paramlist_node = xmlNewChild(root_node, NULL, BAD_CAST "parameterList", BAD_CAST "");
    while (strcmp(current_instr.value, " ) ") != 0) {
        if (is_valid_type(current_instr)) {
            xmlNewChild(paramlist_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Error: Missing identifier after type in param list.\n");
                break;
            }
            xmlNewChild(paramlist_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, " , ") == 0) {
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
    xmlNodePtr var_dec_node = xmlNewChild(root_node, NULL, BAD_CAST "varDec", BAD_CAST "");
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    if (!is_valid_type(current_instr)) {
        fprintf(stderr, "Error: Wrong type in vardec\n");
        return current_instr;
    }
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.type, "identifier") == 0) {
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.value, " , ") == 0) {
            xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);           
        }
    }
    if (strcmp(current_instr.value, " ; ") != 0) {
        fprintf(stderr, "Error: Missing semicolon varDec\n");
        return current_instr;
    }
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    
    return current_instr;
}

bool is_statement(CurrentInstr current_instr) {
    if (strcmp(current_instr.value, " let ") == 0 ||
    strcmp(current_instr.value, " if ") == 0 ||
    strcmp(current_instr.value, " while ") == 0 ||
    strcmp(current_instr.value, " do ") == 0 ||
    strcmp(current_instr.value, " return ") == 0) {
        return true;
    }
    return false;
}

bool is_operator(const char* val) {
    if (val == NULL) return false;

    if (strstr(val, "&lt;") || strstr(val, "&gt;") || strstr(val, "&amp;")) {
        return true;
    }

    if (strlen(val) >= 2 && strchr("+-*/|=", val[1])) {
        return true;
    }

    return false;
}

bool is_keyword_constant(CurrentInstr instr) {
    if (instr.type == NULL || instr.value == NULL) return false;
    
    if (strcmp(instr.type, "keyword") != 0) return false;

    if (strcmp(instr.value, " true ") == 0 ||
        strcmp(instr.value, " false ") == 0 ||
        strcmp(instr.value, " null ") == 0 ||
        strcmp(instr.value, " this ") == 0) {
        return true;
    }

    return false;
}

CurrentInstr compile_expression_node(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node);
CurrentInstr compile_term(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node) {
    xmlNodePtr term_node = xmlNewChild(node, NULL, BAD_CAST "term", BAD_CAST "");

    if (strcmp(current_instr.value, " ~ ") == 0 || strcmp(current_instr.value, " - ") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        return compile_term(jack_file, current_instr, term_node);
    } 
    
    else if (strcmp(current_instr.value, " ( ") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        current_instr = compile_expression_node(jack_file, current_instr, term_node);
        
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        return advance_parser(jack_file); 
    }

    else if (strcmp(current_instr.type, "identifier") == 0) {
        xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        
        current_instr = advance_parser(jack_file);

        if (strcmp(current_instr.value, " [ ") == 0) {
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, term_node);
            
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            return advance_parser(jack_file);
        } 
        else if (strcmp(current_instr.value, " . ") == 0 || strcmp(current_instr.value, " ( ") == 0) {
            if (strcmp(current_instr.value, " . ") == 0) {
                xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.type, "identifier") != 0) {
                    fprintf(stderr, "Error: Missing identifier after . in compile term\n");
                    return current_instr;
                }
                xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.value, " ( ") != 0) {
                    fprintf(stderr, "Error: Missing ( in term . \n");
                    return current_instr;
                }
            }
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            xmlNodePtr expr_list_node = xmlNewChild(term_node, NULL, BAD_CAST "expressionList", BAD_CAST "");
            current_instr = compile_expression_node(jack_file, current_instr, expr_list_node);
            while (strcmp(current_instr.value, " , ") == 0) {
                xmlNewChild(expr_list_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node);
            }
            if (expr_list_node->children == NULL) {
                xmlNodeSetContent(expr_list_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, " ) ") != 0) {
                fprintf(stderr, "Error: No closing ) for expr list in term\n");
                return current_instr;
            }
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            return current_instr;
            
        } else {
            return current_instr;
        }
    }

    else {
        if (strcmp(current_instr.type, "stringConstant") == 0 ||
            strcmp(current_instr.type, "integerConstant") == 0 ||
            is_keyword_constant(current_instr)) {
            
            xmlNewChild(term_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            return advance_parser(jack_file);
        }
    }

    return current_instr;
}

CurrentInstr compile_expression_node(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr node) {
    xmlNodePtr expr_node = xmlNewChild(node, NULL, BAD_CAST "expression", BAD_CAST "");  
    current_instr = compile_term(jack_file, current_instr, expr_node);
    if (expr_node->children->children == NULL) {
        xmlUnlinkNode(expr_node);
        xmlFreeNode(expr_node);
        return current_instr;
    }
    while (is_operator(current_instr.value)) {
            xmlNewChild(expr_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_term(jack_file, current_instr, expr_node);
    }
    return current_instr;
}

CurrentInstr compile_statements(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {
    bool has_return = false;
    while(is_statement(current_instr)) {
        if (strcmp(current_instr.value, " let ") == 0) {
            xmlNodePtr let_node = xmlNewChild(root_node, NULL, BAD_CAST "letStatement", BAD_CAST "");    
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Missing identifier let statement\n");
                return current_instr;
            }
            // TODO: hier könnte [expression] kommen
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, " [ ") == 0) {
                xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, let_node);
                
                xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
            } 
            if (strcmp(current_instr.value, " = ") != 0) {
                fprintf(stderr, "Error missing equal sign let statement\n");
                return current_instr;
            }            
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, let_node);    
            if (strcmp(current_instr.value, " ; ") != 0) {
                fprintf(stderr, "Error: Missing semicolon end of let statement\n");
                return current_instr;
            }
            xmlNewChild(let_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
        } else if (strcmp(current_instr.value, " do ") == 0) {
            xmlNodePtr do_node = xmlNewChild(root_node, NULL, BAD_CAST "doStatement", BAD_CAST "");    
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.type, "identifier") != 0) {
                fprintf(stderr, "Missing identifier do statement\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);

            if (strcmp(current_instr.value, " . ") == 0) {
                xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.type, "identifier") != 0) {
                    fprintf(stderr, "Missing identifier after point doStatement\n");
                    return current_instr;
                }
                xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
            }

            if (strcmp(current_instr.value, " ( ") != 0) {
                fprintf(stderr, "Missing opening bracket do statement\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            xmlNodePtr expr_list_node = xmlNewChild(do_node, NULL, BAD_CAST "expressionList", BAD_CAST "");
            current_instr = compile_expression_node(jack_file, current_instr, expr_list_node);
            while (strcmp(current_instr.value, " , ") == 0) {
                xmlNewChild(expr_list_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                current_instr = compile_expression_node(jack_file, current_instr, expr_list_node);

            }
            if (expr_list_node->children == NULL) {
                xmlNodeSetContent(expr_list_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, " ) ") != 0) {
                fprintf(stderr, "Error: No closing ) for expr list in dostatement\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            if (strcmp(current_instr.value, " ; ") != 0) {
                fprintf(stderr, "Error: Missing semicolon do statment\n");
                return current_instr;
            }
            xmlNewChild(do_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
        } else if(strcmp(current_instr.value, " return ") == 0) {
            has_return = true;
            xmlNodePtr return_node = xmlNewChild(root_node, NULL, BAD_CAST "returnStatement", BAD_CAST "");
            xmlNewChild(return_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
            current_instr = compile_expression_node(jack_file, current_instr, return_node);
            if (strcmp(current_instr.value, " ; ") != 0) {
                fprintf(stderr, "Error: Missing semicolon return statement\n");
                return current_instr;
            }
            xmlNewChild(return_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);
        } else if(strcmp(current_instr.value, " if ") == 0) {
            xmlNodePtr if_node = xmlNewChild(root_node, NULL, BAD_CAST "ifStatement", BAD_CAST "");    
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, " ( ") != 0) {
                fprintf(stderr, "Error: Missing ( in ifStatement if block\n");
                return current_instr;
            }   
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);      
            current_instr = compile_expression_node(jack_file, current_instr, if_node);
            if (strcmp(current_instr.value, " ) ") != 0) {
                fprintf(stderr, "Error: Missing ) in ifStatement if block\n");
                return current_instr;                
            }
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            
            if (strcmp(current_instr.value, " { ") != 0) {
                fprintf(stderr, "Error: Missing { in ifStatement if block\n");
                return current_instr;                    
            }
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            xmlNodePtr if_statement_node = xmlNewChild(if_node, NULL, BAD_CAST "statements", BAD_CAST "");    
            current_instr = compile_statements(jack_file, current_instr, if_statement_node);
            if (if_statement_node->children == NULL) {
                xmlNodeSetContent(if_statement_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, " } ") != 0) {
                fprintf(stderr, "Error: Missing } in ifStatement if block\n");
                return current_instr;                  
            }
            xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, " else ") == 0) {
                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                if (strcmp(current_instr.value, " { ") != 0) {
                    fprintf(stderr, "Error: Missing { else block\n");
                    return current_instr;
                }
                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);
                xmlNodePtr else_statement_node = xmlNewChild(if_node, NULL, BAD_CAST "statements", BAD_CAST "");    
                current_instr = compile_statements(jack_file, current_instr, else_statement_node);
                if (else_statement_node->children == NULL) {
                    xmlNodeSetContent(else_statement_node, BAD_CAST "\n");
                }
                if (strcmp(current_instr.value, " } ") != 0) {
                    fprintf(stderr, "Error: Missing } in ifStatement if block\n");
                    return current_instr;                  
                }    
                xmlNewChild(if_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
                current_instr = advance_parser(jack_file);                              
            }        
        } else if (strcmp(current_instr.value, " while ") == 0) {
            // 'while' '(' expression ')' '{' statements '}'
            xmlNodePtr while_node = xmlNewChild(root_node, NULL, BAD_CAST "whileStatement", BAD_CAST "");    
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            if (strcmp(current_instr.value, " ( ") != 0) {
                fprintf(stderr, "Error: Missing ( in whileStatement\n");
                return current_instr;
            }  
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);      
            current_instr = compile_expression_node(jack_file, current_instr, while_node);
            if (strcmp(current_instr.value, " ) ") != 0) {
                fprintf(stderr, "Error: Missing ) in whileStatement\n");
                return current_instr;                
            } 
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  

            if (strcmp(current_instr.value, " { ") != 0) {
                fprintf(stderr, "Error: Missing { in whileStatement\n");
                return current_instr;                    
            }
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
            xmlNodePtr while_statements_node = xmlNewChild(while_node, NULL, BAD_CAST "statements", BAD_CAST "");    
            current_instr = compile_statements(jack_file, current_instr, while_statements_node);
            if (while_statements_node->children == NULL) {
                xmlNodeSetContent(while_statements_node, BAD_CAST "\n");
            }
            if (strcmp(current_instr.value, " } ") != 0) {
                fprintf(stderr, "Error: Missing } in while statement block\n");
                return current_instr;                  
            }
            xmlNewChild(while_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
            current_instr = advance_parser(jack_file);  
        } else {
            current_instr = advance_parser(jack_file);
        }
    }

    return current_instr;
}

CurrentInstr compile_subroutine_body(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {  
    if (strcmp(current_instr.value, " { ") != 0) {
        fprintf(stderr, "Error: Missing opening Braces subroutine body.\n");
        return current_instr;        
    }
    xmlNodePtr body_node = xmlNewChild(root_node, NULL, BAD_CAST "subroutineBody", BAD_CAST "");
    xmlNewChild(body_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, " var ") == 0) {
    current_instr = compile_var_dec(jack_file, current_instr, body_node);
    }

    xmlNodePtr statement_node = xmlNewChild(body_node, NULL, BAD_CAST "statements", BAD_CAST "");
    current_instr = compile_statements(jack_file, current_instr, statement_node);
    if (statement_node->children == NULL) {
        xmlNodeSetContent(statement_node, BAD_CAST "\n");
    }
    if (strcmp(current_instr.value, " } ") != 0) {
        fprintf(stderr, "Error: Missing closing brace for subroutine body\n");
        return current_instr;
    }
    xmlNewChild(body_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    return advance_parser(jack_file);
}

CurrentInstr compile_subroutine(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {
    static int count = 0;
    count++;
    bool is_constructor = strcmp(current_instr.value, " constructor ") == 0;
    xmlNodePtr subroutine_node = xmlNewChild(root_node, NULL, BAD_CAST "subroutineDec", BAD_CAST "");
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    if (is_constructor) {
        if (strcmp(current_instr.type, "identifier") != 0) {
            fprintf(stderr, "Error: Missing subroutine identifier.\n");
            return current_instr;
        } 
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.value, " new ") != 0) {
            fprintf(stderr, "Error: Missing new keyword constructor. val=%s\n", current_instr.value);
            return current_instr;
        }
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);

    } else {
        if (!is_valid_type(current_instr) && strcmp(current_instr.value, " void ") != 0) {
            fprintf(stderr, "Error: Invalid subroutine return type. val=%s\n", current_instr.value);
            return current_instr;
        }
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        if (strcmp(current_instr.type, "identifier") != 0) {
            fprintf(stderr, "Error: Missing subroutine identifier.\n");
            return current_instr;
        } 
        xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
    }

    if (strcmp(current_instr.value, " ( ") != 0) {
        fprintf(stderr, "Error: Missing opening Paranthesis parameterlist.\n");
        return current_instr;
    }
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    current_instr = compile_parameter_list(jack_file, current_instr, subroutine_node);   
    if (strcmp(current_instr.value, " ) ") != 0) {
        fprintf(stderr, "Error: Missing closing Paranthesis parameterlist.\n");
        return current_instr;       
    }
    xmlNewChild(subroutine_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    current_instr = compile_subroutine_body(jack_file, current_instr, subroutine_node);
    return current_instr;
}

CurrentInstr compile_class_var_dec(FILE *jack_file, CurrentInstr current_instr, xmlNodePtr root_node) {
    xmlNodePtr var_dec_node = xmlNewChild(root_node, NULL, BAD_CAST "classVarDec", BAD_CAST "");
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    if (!is_valid_type(current_instr)) {
        fprintf(stderr, "Error: static or field missing type. type=%s, val=%s\n", current_instr.type, current_instr.value);
        return current_instr;
    }
    xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.type, "identifier") == 0) {
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
        xmlNewChild(var_dec_node, NULL, BAD_CAST current_instr.type, BAD_CAST current_instr.value);
        current_instr = advance_parser(jack_file);
    }

    return current_instr;
}

void compile_class(FILE *jack_file, xmlNodePtr node) {
    // Check for class keyword
    CurrentInstr current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, " class ") != 0) {
        fprintf(stderr, "Error: File does not start with class keyword.\n");
        return;
    } 
    
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.type, "identifier") != 0) {
        fprintf(stderr, "Error: Missing class identifier.\n");
        return;
    }
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    if (strcmp(current_instr.value, " { ") != 0) {
        fprintf(stderr, "Error: Missing class opening braces.\n");
        return;
    }
    xmlNewChild(node, NULL, BAD_CAST strdup(current_instr.type), BAD_CAST strdup(current_instr.value));
    current_instr = advance_parser(jack_file);
    while (strcmp(current_instr.value, " static ") == 0 
    || strcmp(current_instr.value, " field ") == 0) {
        current_instr = compile_class_var_dec(jack_file, current_instr, node);
    }   
    
    while (strcmp(current_instr.value, " function ") == 0 || 
    strcmp(current_instr.value, " method ") == 0 || 
    strcmp(current_instr.value, " constructor ") == 0) {
        current_instr = compile_subroutine(jack_file, current_instr, node);
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