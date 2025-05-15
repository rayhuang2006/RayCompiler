#ifndef LEXER_H
#define LEXER_H 

#include<string>

enum Token {
    tok_eof = -1,
    tok_def = -2,
    tok_if = -3,
    tok_then = -4,
    tok_else = -5,
    tok_identifier = -6,
    tok_number = -7, // float
    tok_operator = -8, // + - * / > =
    tok_lparen = -9, // (
    tok_rparen = -10, // )
    tok_semicolon = -11, //;
    tok_comma = -12 //,
};

extern std::string IdentifierStr;
extern double NumVal;
extern char CurrentOperator;
extern int CurTok;

int getNextToken();
int getNextTOkenP();

#endif