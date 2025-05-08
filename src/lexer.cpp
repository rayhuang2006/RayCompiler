#include<cctype>
#include<string>
#include<iostream>
#include"../include/lexer.h"

std::string IdentifierStr;
double NumVal;
char CurrentOperator;

int getNextToken() {
    static int LastChar = ' ';

    while (std::isspace(LastChar)){
        LastChar = std::getchar();
    }


    if(std::isalpha(LastChar)) {
        IdentifierStr = LastChar;
        while (std::isalnum((LastChar = std::getchar())) || LastChar == '_'){
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "def") return tok_def;
        if (IdentifierStr == "if") return tok_if;
        if (IdentifierStr == "then") return tok_then;
        if (IdentifierStr == "else") return tok_else;
        return tok_identifier;
    }

    //處理浮點數
    if(std::isdigit(LastChar) || LastChar == '.'){
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = std::getchar();
        } while (std::isdigit(LastChar) || LastChar == '.');

        NumVal = std::stod(NumStr);
        return tok_number;
    }

    //處理註解
    if(LastChar == '#') {
        do{
            LastChar = std::getchar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if(LastChar != EOF){
            return getNextToken();
        }
    }

    if(LastChar == EOF){
        return tok_eof;
    }

    int ThisChar = LastChar;
    LastChar = std::getchar();
    switch (ThisChar) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '>':
        case '=':
            CurrentOperator = ThisChar;
            return tok_operator;
        case '(':
            return tok_lparen;
        case ')':
            return tok_rparen;
        case ';':
            return tok_semicolon;
        case ',':
            return tok_comma;
        default:
            std::cerr << "Unknown character: " << (char)ThisChar << std::endl;
            return getNextToken();
    }
}

//---------------------------------------------------------------------------------//
//下面是測試用的主函數，如果你想測試看看lexer的效果，你可以把主函數的註解取消掉，然後輸入以下指令 //
//clang++ -std=c++17 src/lexer.cpp -o lexer_test                                   //
//./lexer_test                                                                     //
//---------------------------------------------------------------------------------//


// int main(){
//     while(true){
//         int tok = getNextToken();
//         switch (tok) {
//             case tok_eof:
//                 std::cout << "EOF" << std::endl;
//                 return 0;
//             case tok_def:
//                 std::cout << "Token: def" << std::endl;
//                 break;
//             case tok_if:
//                 std::cout << "Token: if" << std::endl;
//                 break;
//             case tok_then:
//                 std::cout << "Token: then" << std::endl;
//                 break;
//             case tok_else:
//                 std::cout << "Token: else" << std::endl;
//                 break;
//             case tok_identifier:
//                 std::cout << "Token: identifier (" << IdentifierStr << ")" << std::endl;
//                 break;
//             case tok_number:
//                 std::cout << "Token: number (" << NumVal << ")" << std::endl;
//                 break;
//             case tok_operator:
//                 std::cout << "Token: operator (" << CurrentOperator << ")" << std::endl;
//                 break;
//             case tok_lparen:
//                 std::cout << "Token: (" << std::endl;
//                 break;
//             case tok_rparen:
//                 std::cout << "Token: )" << std::endl;
//                 break;
//             case tok_semicolon:
//                 std::cout << "Token: ;" << std::endl;
//                 break;
//             default:
//                 std::cout << "Token: unkown (" << tok << ")" << std::endl;
//                 break;
//         }
//     }
//     return 0;
// }


