#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>
#include <llvm/IR/Value.h>

//所有的出發點
class ExprAST {
    public:
        virtual ~ExprAST() = default;
        virtual llvm::Value *Codegen() = 0;
};

//數字
class NumberExprAST: public ExprAST {
    double Val;
    public:
        NumberExprAST(double Val) : Val(Val) {}
        llvm::Value *Codegen() override;
};

//變數定義
class VariableExprAST: public ExprAST {
    std::string Name;
    public:
        VariableExprAST(const std::string &Name) 
        : Name(Name) {}
        std::string getName() const { return Name; }
        llvm::Value *Codegen() override;
};

// 二元運算表達式，反正就是加減乘除之類的
class BinaryExprAST: public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) 
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        llvm::Value *Codegen() override;
};

// 條件if then else
class IfExprAST: public ExprAST {
    std::unique_ptr<ExprAST> Cond, Then, Else;
    public:
        IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then, std::unique_ptr<ExprAST> Else)
        : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}
        llvm::Value *Codegen() override;
};

// 把函數塞進去
class CallExprAST: public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    public:
        CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
        llvm::Value *Codegen() override;
};

//函數原型
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    public:
        PrototypeAST(const std::string Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
        llvm::Function *Codegen();
};

//函數定義
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
        llvm::Function *Codegen();
};

#endif