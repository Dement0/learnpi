%{
#include <stdio.h>
#include <stdlib.h>
#include "learnpi.h"
#include "functions.h"

#define YYDEBUG 1

int yylex();   
%}

%union {
  struct ast *ast;
  struct symbol_list *symbol_list;
  struct val *value;
  int function_id;
  char *str;
  int type;
}

%token <type> TYPE COMPLEX_TYPE
%token <str> NAME
%token <value> VALUE
%token <function_id> BUILT_IN_FUNCTION
%token IF ELSE EOL WHILE FOR FUN
%token <integer> OR_OPERATION AND_OPERATION NOT_OPERATION

%nonassoc <function_id> CMP
%right '='
%left AND_OPERATION OR_OPERATION NOT_OPERATION
%left '+' '-'
%left '*' '/'
%nonassoc '|'
%nonassoc UMINUS

%type <ast> statement control_flow loop_flow exp list explist
%type <symbol_list> sym_list

%start learnpi

%%
learnpi: /* nothing */
   | learnpi statement {
      struct val *value = eval($2);
      if(value) {
         treefree($2);
      }
    }
   | learnpi FUN NAME '(' sym_list ')' '=' '{' EOL list '}' EOL { dodef($3, $5, $10); }
   | learnpi FUN NAME '(' ')' '=' '{' EOL list '}' EOL { dodef($3, NULL, $9); }
   | learnpi error EOL { yyerrok; yyparse(); }
;

statement: control_flow EOL
   | loop_flow EOL
   | TYPE NAME '=' explist ';'           { $$ = new_assignment($2, $4 ); }
   | TYPE NAME '=' explist EOL           { $$ = new_assignment($2, $4 ); }
   | TYPE NAME EOL                       { $$ = new_declaration($2, $1); }
   | COMPLEX_TYPE NAME '=' explist EOL   { $$ = new_complex_assignment($2, $1, $4);}
   | COMPLEX_TYPE NAME EOL               { $$ = new_declaration($2, $1); }
   | exp EOL
;

control_flow: IF '(' EOL exp EOL ')' '{' EOL list EOL '}' ELSE '{' EOL list EOL '}'   { $$ = newflow(IF_STATEMENT, $4, $9, $15); }
   | IF '(' EOL exp EOL ')' '{' EOL list '}' ELSE '{' EOL list EOL '}'                { $$ = newflow(IF_STATEMENT, $4, $9, $14); }
   | IF '(' EOL exp EOL ')' '{' EOL list '}' ELSE '{' EOL list '}'                    { $$ = newflow(IF_STATEMENT, $4, $9, $14); }
   | IF '(' EOL exp ')' '{' EOL list EOL '}' EOL ELSE EOL '{' EOL list EOL '}'        { $$ = newflow(IF_STATEMENT, $4, $8, $16); }
   | IF '(' EOL exp ')' '{' EOL list EOL '}' EOL ELSE '{' EOL list EOL '}'            { $$ = newflow(IF_STATEMENT, $4, $8, $15); }
   | IF '(' EOL exp ')' '{' EOL list '}' EOL ELSE '{' EOL list EOL '}'                { $$ = newflow(IF_STATEMENT, $4, $8, $14); }
   | IF '(' EOL exp ')' '{' EOL list '}' EOL ELSE '{' EOL list '}'                    { $$ = newflow(IF_STATEMENT, $4, $8, $14); }
   | IF '(' EOL exp ')' '{' EOL list '}' ELSE '{' EOL list EOL '}'                    { $$ = newflow(IF_STATEMENT, $4, $8, $13); }
   | IF '(' EOL exp ')' '{' EOL list '}' ELSE '{' EOL list '}'                        { $$ = newflow(IF_STATEMENT, $4, $8, $13); }
   | IF '(' exp EOL ')' '{' EOL list '}' EOL ELSE '{' EOL list EOL '}'                { $$ = newflow(IF_STATEMENT, $3, $8, $14); }
   | IF '(' exp EOL ')' '{' EOL list '}' EOL ELSE '{' EOL list '}'                    { $$ = newflow(IF_STATEMENT, $3, $8, $14); }
   | IF '(' exp EOL ')' '{' EOL list EOL '}'                                          { $$ = newflow(IF_STATEMENT, $3, $8, NULL); }
   | IF '(' exp ')' '{' EOL list EOL '}' EOL ELSE '{' EOL list EOL '}'                { $$ = newflow(IF_STATEMENT, $3, $7, $14); }
   | IF '(' exp ')' '{' EOL list EOL '}' EOL                                          { $$ = newflow(IF_STATEMENT, $3, $7, NULL); }
   | IF '(' exp ')' '{' EOL list '}' EOL ELSE '{' EOL list EOL '}'                    { $$ = newflow(IF_STATEMENT, $3, $7, $13); }
   | IF '(' exp ')' '{' EOL list '}' EOL ELSE '{' EOL list '}'                        { $$ = newflow(IF_STATEMENT, $3, $7, $13); }
   | IF '(' exp ')' '{' EOL list '}' ELSE '{' EOL list EOL '}'                        { $$ = newflow(IF_STATEMENT, $3, $7, $12); }
   | IF '(' exp ')' '{' EOL list '}' ELSE '{' EOL list '}'                            { $$ = newflow(IF_STATEMENT, $3, $7, $12); }
   | IF '(' exp ')' '{' list '}'                                                      { $$ = newflow(IF_STATEMENT, $3, $6, NULL); }
;

loop_flow: WHILE '(' EOL exp EOL ')' '{' EOL list EOL '}'    { $$ = newflow(LOOP_STATEMENT, $4, $9, NULL); }
   | WHILE '(' EOL exp ')' '{' EOL list EOL '}'              { $$ = newflow(LOOP_STATEMENT, $4, $8, NULL); }
   | WHILE '(' exp ')' '{' EOL list EOL '}'                  { $$ = newflow(LOOP_STATEMENT, $3, $7, NULL); }
   | WHILE '(' exp ')' '{' EOL list '}'                      { $$ = newflow(LOOP_STATEMENT, $3, $7, NULL); }
   | WHILE '(' exp ')' '{' list '}'                          { $$ = newflow(LOOP_STATEMENT, $3, $6, NULL); }
   | FOR '(' exp ';' exp ';' exp ')' '{' EOL list '}'        { $$ = new_for_flow(FOR_STATEMENT, $3, $5, $7, $11); }
;

exp: exp CMP exp                             { $$ = new_comparison($2, $1, $3); }
   | exp '+' exp                             { $$ = new_ast_with_children('+', $1, $3); }
   | exp '-' exp                             { $$ = new_ast_with_children('-', $1, $3); }
   | exp '*' exp                             { $$ = new_ast_with_children('*', $1, $3); }
   | exp '/' exp                             { $$ = new_ast_with_children('/', $1, $3); }
   | '|' exp                                 { $$ = new_ast_with_child('|', $2); }
   | exp AND_OPERATION exp                   { $$ = new_ast_with_children(LOGICAL_AND, $1, $3); }
   | exp OR_OPERATION exp                    { $$ = new_ast_with_children(LOGICAL_OR, $1, $3); }
   | '(' exp ')'                             { $$ = $2; }
   | '-' exp %prec UMINUS                    { $$ = new_ast_with_child(UNARY_MINUS, $2); }
   | VALUE                                   { $$ = new_value($1); }
   | NAME                                    { $$ = new_reference($1); }
   | NAME '=' exp                            { $$ = new_assignment($1, $3); }
   | BUILT_IN_FUNCTION '(' explist ')'       { $$ = new_builtin_function($1, NULL, $3); } /* Node for builtin function without name */
   | BUILT_IN_FUNCTION '(' ')'               { $$ = new_builtin_function($1, NULL, NULL); } /* Node for builtin function without name and parameters */
   | NAME '(' explist ')'                    { $$ = new_user_function($1, $3); } /* Node for user function call with parameters */
   | NAME '(' ')'                            { $$ = new_user_function($1, NULL); } /* Node for user function call without parameters */
;

list: /* nothing */ { $$ = NULL; }
   | statement list {
            if ($2 == NULL) {
                  $$ = $1;
            } else {
                  $$ = new_ast_with_children(STATEMENT_LIST, $1, $2);
            }
         }
;

explist: exp
   | exp ',' explist   { $$ = new_ast_with_children(STATEMENT_LIST, $1, $3); }
;

sym_list: NAME         { $$ = create_symbol_list($1, NULL); }
   | NAME ',' sym_list { $$ = create_symbol_list($1, $3); }
;

%%