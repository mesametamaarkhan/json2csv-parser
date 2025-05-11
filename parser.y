%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* External variables from scanner */
extern int line;
extern int column;
extern FILE* yyin;

/* AST root node */
Node* root = NULL;

void yyerror(const char* s);
extern int yylex();
%}

%union {
    char* string_val;
    double double_val;
    int boolean_val;
    struct Node* node;
    struct Pair* pair;
}

%token <string_val> STRING
%token <double_val> NUMBER
%token <boolean_val> TRUE FALSE
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA NUL

%type <node> json value object array members elements
%type <pair> pair

%%

json:
    value { root = $1; }
    ;

value:
    object  { $$ = $1; }
    | array { $$ = $1; }
    | STRING { 
        $$ = create_string_node($1);
        free($1);  /* Free string allocated by lexer */
    }
    | NUMBER { $$ = create_number_node($1); }
    | TRUE { $$ = create_boolean_node(1); }
    | FALSE { $$ = create_boolean_node(0); }
    | NUL { $$ = create_null_node(); }
    ;

object:
    LBRACE RBRACE { $$ = create_object_node(NULL, 0); }
    | LBRACE members RBRACE { $$ = $2; }
    ;

members:
    pair { 
        $$ = create_object_node(NULL, 0);
        add_pair_to_object($$, $1);
    }
    | members COMMA pair { 
        $$ = $1;
        add_pair_to_object($$, $3);
    }
    ;

pair:
    STRING COLON value { $$ = create_pair_node($1, $3); free($1); }
    ;

array:
    LBRACKET RBRACKET { $$ = create_array_node(NULL, 0); }
    | LBRACKET elements RBRACKET { $$ = $2; }
    ;

elements:
    value { 
        $$ = create_array_node(NULL, 0);
        add_element_to_array($$, $1);
    }
    | elements COMMA value {
        $$ = $1;
        add_element_to_array($$, $3);
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, line, column);
    exit(1);
}