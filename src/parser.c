#include "../include/parser.h"
#include <string.h>
#include <ctype.h>

// Token array and state

token tokens[1024];
int tokencnt = 0;
int posok = 0;

void tokenize(char *input) {
    tokencnt = 0;
    int i = 0;
    int len = strlen(input);
    while (i < len) {
        if (isspace(input[i])) { i++; continue; }
        if (input[i] == '&') {
            if (i + 1 < len && input[i + 1] == '&') {
                tokens[tokencnt].type = T_AND_IF;
                strcpy(tokens[tokencnt].text, "&&");
                i += 2;
            } else {
                tokens[tokencnt].type = T_AMP;
                strcpy(tokens[tokencnt].text, "&");
                i++;
            }
            tokencnt++;
        } else if (input[i] == '|') {
            tokens[tokencnt].type = T_PIPE;
            strcpy(tokens[tokencnt].text, "|");
            i++; tokencnt++;
        } else if (input[i] == '<') {
            tokens[tokencnt].type = T_INPUT;
            strcpy(tokens[tokencnt].text, "<");
            i++; tokencnt++;
        } else if (input[i] == '>') {
            if (i + 1 < len && input[i + 1] == '>') {
                tokens[tokencnt].type = T_APPEND;
                strcpy(tokens[tokencnt].text, ">>");
                i += 2;
            } else {
                tokens[tokencnt].type = T_OUTPUT;
                strcpy(tokens[tokencnt].text, ">");
                i++;
            }
            tokencnt++;
        } else if (input[i] == ';') {
            tokens[tokencnt].type = T_SEMI;
            strcpy(tokens[tokencnt].text, ";");
            i++; tokencnt++;
        } else {
            int start = i;
            while (i < len && !isspace(input[i]) && strchr("|&<>;", input[i]) == NULL) i++;
            tokens[tokencnt].type = T_NAME;
            strncpy(tokens[tokencnt].text, input + start, i - start);
            tokens[tokencnt].text[i - start] = '\0';
            tokencnt++;
        }
    }
    tokens[tokencnt].type = T_END;
    strcpy(tokens[tokencnt].text, "");
    tokencnt++;
}

static int consume(tokentype type) {
    if (posok < tokencnt && tokens[posok].type == type) {
        posok++;
        return 1;
    }
    return 0;
}

static int parsename() { return consume(T_NAME); }

static int parseatomic() {
    if (!parsename()) return 0;
    while (1) {
        if (consume(T_NAME)) continue;
        if (consume(T_INPUT) || consume(T_OUTPUT) || consume(T_APPEND)) {
            if (!parsename()) return 0;
        } else break;
    }
    return 1;
}

static int parsecmd() {
    if (!parseatomic()) return 0;
    while (consume(T_PIPE)) {
        if (!parseatomic()) return 0;
    }
    return 1;
}

int parseok() {
    posok = 0;
    if (!parsecmd()) return 0;
    while (1) {
        if (consume(T_AND_IF)) {
            if (!parsecmd()) return 0;
        } else if (consume(T_AMP)) {
            if (tokens[posok].type == T_END) return 1;
            if (!parsecmd()) return 0;
        } else if (consume(T_SEMI)) {
            if (tokens[posok].type == T_END) return 1;
            if (!parsecmd()) return 0;
        } else {
            break;
        }
    }
    return tokens[posok].type == T_END;
}

//llm code