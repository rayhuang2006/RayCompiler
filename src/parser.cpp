#include"../include/lexer.h"
#include"../include/ast.h"
#include<map>
#include<memory>
#include<iostream>

static int CurTok;

static int getNextTokenP(){
    CurTok = getNextToken();
    return CurTok;
}

static std::map<char, int> BinopPrecedence = {
    {'=', 10}, {'>', 20}, {'+', 30}, {'-', 30}, {'*', 40}, {'/', 40}
};

static std::unique_ptr<ExprAST> LogError(const char *Str) {
    std::cerr << "Error" << Str << std::endl;
    return nullptr;
};

static std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

//前項聲明
//筆記：就是先定義好，但是我先不告訴你這個東西是幹什麼用的
static std::unique_ptr<ExprAST> ParseExpression();

//數字解析
//筆記：這就是標準解析數字做法，基本上呢，你就是會吃掉這個數字，然後創造一個<NumberExprAST>(數字) 的節點，然後繼續去吃下一個token
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextTokenP();
    return Result;
};

//解析括號表達式
//筆記：這段基本上呢會先吃掉左括號，然後把後面的一整串吃成一個Expression樹，然後去吃吃看後面是不是右括號，如果不是就給個錯誤訊息，是就很完美了
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextTokenP();
    auto V = ParseExpression();
    if(!V){
        return nullptr;
    }
    if(CurTok != tok_rparen){
        return LogError("Expected ')' !");
    }
    getNextTokenP();
    return V;
};

//解析函數呼叫
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;
    getNextTokenP();

    if(CurTok != tok_lparen){
        return std::make_unique<VariableExprAST>(IdName);
    }

    getNextTokenP();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if(CurTok != tok_rparen){
        while(true){
            if(auto Arg = ParseExpression()){
                Args.push_back(std::move(Arg));
            } else {
                return nullptr;
            }

            if(CurTok == tok_rparen){
                break;
            } 

            if(CurTok != tok_comma) {
                return LogError("Expected ')' of ',' in argument list !");
            }
            getNextTokenP();
        }
    }
    getNextTokenP();
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
};

//解析if 表達式
static std::unique_ptr<ExprAST> ParseIfExpr(){
    getNextTokenP();
    auto Cond = ParseExpression();
    if(!Cond){
        return nullptr;
    }

    if(CurTok != tok_then){
        return LogError("Expected 'then' !");
    }
    getNextTokenP();

    auto Then = ParseExpression();
    if(!Then){
        return nullptr;
    }

    if(CurTok != tok_else){
        return LogError("Expected 'else' !");
    }
    getNextTokenP();

    auto Else = ParseExpression();
    if(!Else){
        return nullptr;
    }

    return std::make_unique<IfExprAST>(std::move(Cond),std::move(Then),std::move(Else));

}

//解析一些簡單的基礎表達
static std::unique_ptr<ExprAST> ParsePrimary(){
    switch(CurTok){
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case tok_lparen:
            return ParseParenExpr();
        case tok_if:
            return ParseIfExpr();
        default:
            return LogError("unknown token when expecting an expression!");
    }
}

//解析二元運算
//筆記：這個函數有點複雜，基本上是這樣，這個函數會先把一坨已經處理好的東西扔進函數裡，然後看現在的token是不是運算子，
//然後看這個運算子的優先級有沒有比較高，如果有的話，那就表示結束了，不用繼續解析判斷了，就回傳LHS，
//如果更高，我就吃掉這個，解析右邊的，然後繼續檢查右邊有沒有更更高的，然後把右邊的處理完之後，
//在跟原本的左邊結合在一起，塞進左邊已經處理好了的，開始新一輪。
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS){
    while(true){
        int TokPrec = (CurTok == tok_operator)? BinopPrecedence[CurrentOperator] : -1;

        if(TokPrec < ExprPrec){
            return LHS;
        }

        char BinOp = CurrentOperator;
        getNextTokenP();

        auto RHS = ParsePrimary();
        if(!RHS){
            return nullptr;
        }

        int NextTok = (CurTok == tok_operator)? BinopPrecedence[CurrentOperator] : -1;
        if(TokPrec < NextTok){
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if(!RHS){
                return nullptr;
            }
        }

        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

// 解析基本表達
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if(!LHS){
        return nullptr;
    }

    return ParseBinOpRHS(0, std::move(LHS));
}

// 解析函數的定義那一行（函數原型）
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if(CurTok != tok_identifier){
        return LogErrorP("Expected function name in prototype!");
    }

    std::string FnName = IdentifierStr;
    getNextTokenP();

    if(CurTok != tok_lparen){
        return LogErrorP("Expected '(' in prototype!");
    }

    std::vector <std::string> ArgNames;
    getNextTokenP();

    while(CurTok == tok_identifier){
        ArgNames.push_back(IdentifierStr);
        getNextTokenP();
        if(CurTok != tok_comma){
            break;
        }
        getNextTokenP();
    }

    if(CurTok != tok_rparen){
        return LogErrorP("Expected ')' in prototype!");
    }

    getNextTokenP();
    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames)); 
}

//解析函數定義
static std::unique_ptr<FunctionAST> ParseDefinition(){
    getNextTokenP();
    auto Proto = ParsePrototype();
    if(!Proto){
        return nullptr;
    }

    if(CurTok != tok_lparen && CurTok != tok_identifier && CurTok != tok_number && CurTok != tok_if){
        LogErrorP("expected expression in function body");
    }

    auto E = ParseExpression();
    if(!E){
        return nullptr;
    }

    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
}

//解析頂層表達式
//把函數結構串起來
static std::unique_ptr<FunctionAST>ParseTopLevelExpr(){
    if(auto E = ParseExpression()){
        auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }

    return nullptr;
}

static void MainLoop() {
    while(true){
        getNextTokenP();
        switch(CurTok){
            case tok_eof:
                return;
            case tok_semicolon:
                getNextTokenP();
                break;
            case tok_def:
                if(auto Fn = ParseDefinition()){
                    std::cout << "Parsed a function definition" << std::endl;
                } else {
                    getNextTokenP();
                }
                break;
            default:
                if(auto Fn = ParseTopLevelExpr()){
                    std::cout << "Parsed a top-level definition" << std::endl;
                } else {
                    getNextTokenP();
                }
                break;
        }
    }
}

//test
//先編譯：clang++ -std=c++17 src/lexer.cpp src/parser.cpp -Iinclude -o parser_test 
//然後把它塞進tests裡面，輸入 ./tests/parser_test < ./tests/test.ray
int main(){
    MainLoop();
    return 0;
}