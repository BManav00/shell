#ifndef PARSER_H
#define PARSER_H

typedef enum {
    T_NAME, T_PIPE, T_AMP, T_AND_IF, T_INPUT, T_OUTPUT, T_APPEND, T_SEMI, T_END
} tokentype;

typedef struct {
    tokentype type;
    char text[1024];
} token;

extern token tokens[1024];
extern int tokencnt;
extern int posok;

void tokenize(char *input);
int parseok();

#endif // PARSER_H

//llm code