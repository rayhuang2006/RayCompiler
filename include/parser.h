#ifndef PARSER_H
#define PARSER_H

#include "../include/ast.h"

std::unique_ptr<FunctionAST> ParseDefinition();
std::unique_ptr<FunctionAST> ParseTopLevelExpr();
int getNextTokenP();

#endif