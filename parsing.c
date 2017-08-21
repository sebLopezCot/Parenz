#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

long eval_op_unary(char* op, long x) {
    if (strcmp(op, "-") == 0) { return -x; }
}

long eval_op_binary(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    if (strcmp(op, "%") == 0) { return x % y; }
}

long eval(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    char* op = t->children[1]->contents;

    long x = eval(t->children[2]);

    if (!strstr(t->children[3]->tag, "expr")) {
       x = eval_op_unary(op, x); 
    }

    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op_binary(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv) {

    // Create some parsers
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* UOperator = mpc_new("unaryoperator");
    mpc_parser_t* BOperator = mpc_new("binaryoperator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Parenz = mpc_new("parenz");

    // Define them with the following language
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                                                       \
          number    : /-?[0-9]+/ ;                                                                              \
          unaryoperator: '-' ;                                                                                  \
          binaryoperator  : '+' | '-' | '*' | '/' | '%' ;                                                       \
          expr      : <number> | '(' <binaryoperator> <expr> <expr>+ ')' | '(' <unaryoperator> <expr> ')' ;     \
          parenz    : /^/ (<binaryoperator> <expr> <expr>+ | <unaryoperator> <expr>) /$/ ;                      \
        ",
        Number, UOperator, BOperator, Expr, Parenz);

    puts("Parenz Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char* input = readline("parenz> ");

        add_history(input);
        
        // Attempt to parse the user input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Parenz, &r)){
            // On success print the AST
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            // error
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Number, UOperator, BOperator, Expr, Parenz);

    return 0;
}
