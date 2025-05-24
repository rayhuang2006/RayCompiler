#include"../include/ast.h"
#include"../include/lexer.h"
#include"../include/parser.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

void InitializeCodegen(){
    TheModule = std::make_unique<llvm::Module>("RayCompiler", TheContext);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

static llvm::Value *LogErrorV(const char *Str){
    std::cerr << "Codegen Error: " << Str << " (Current token: " << CurTok << ")" << std::endl;
    return nullptr;
}

llvm::Value *NumberExprAST::Codegen(){
    return llvm::ConstantFP::get(TheContext, llvm::APFloat(Val));
}

llvm::Value *VariableExprAST::Codegen(){
    llvm::Value *V = NamedValues[Name];
    if(!V){
        return LogErrorV("Unknown variable name!");
    }

    return V;
}

llvm::Value *BinaryExprAST::Codegen(){
    if(Op == '='){
        VariableExprAST *LHSE = dynamic_cast<VariableExprAST*>(LHS.get());
        if(!LHSE){
            return LogErrorV("Destination of '=' must be a variable");
        }
        llvm::Value *Val = RHS->Codegen();
        if(!RHS){
            return nullptr;
        }
        NamedValues[LHSE->getName()] = Val;
        return Val;
    }
    
    llvm::Value *L = LHS->Codegen();
    llvm::Value *R = RHS->Codegen();

    if(!L || !R){
        return nullptr;
    }

    switch(Op){
        case '+':
            return Builder.CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder.CreateFSub(L, R, "subtmp");
        case '*':
            return Builder.CreateFMul(L, R, "multmp");
        case '/':
            return Builder.CreateFDiv(L, R, "divtmp");
        case '<':
            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(TheContext), "booltmp");
        case '>':
            L = Builder.CreateFCmpUGT(L, R, "cmptmp");
            return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(TheContext), "booltmp");
        default:
            return LogErrorV("Invalid binary operator");
    }
}

llvm::Value *IfExprAST::Codegen(){
    llvm::Value *CondV = Cond->Codegen();
    if(!CondV){
        return nullptr;
    }
    if(CondV->getType()->isDoubleTy()){
        return LogErrorV("Condition must be a double type"); 
    }
    CondV = Builder.CreateFCmpONE(CondV, llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)), "ifcond");


    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    llvm::Value *ThenV = Then->Codegen();
    if(!ThenV){
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    ElseBB->insertInto(TheFunction);
    Builder.SetInsertPoint(ElseBB);
    llvm::Value *ElseV = Else->Codegen();
    if(!ElseV){
        return nullptr;
    }
    Builder.CreateBr(ElseBB);
    ElseBB = Builder.GetInsertBlock();

    MergeBB->insertInto(TheFunction);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

llvm::Value *CallExprAST::Codegen(){
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
    if(!CalleeF){
        return LogErrorV("Unknown function referenced!");
    }

    if(CalleeF->arg_size() != Args.size()){
        return LogErrorV("Incorrect number of arguments passed!");
    }

    std::vector<llvm::Value *> ArgsV;
    for(auto &Arg : Args){
        llvm::Value *ArgV = Arg->Codegen();
        if(!ArgV){
            return nullptr;
        }
        ArgsV.push_back(ArgV);
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::Codegen(){
    std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(TheContext));
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), Doubles,false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    unsigned Idx = 0;
    for(auto &Arg : F->args()){
        Arg.setName(Args[Idx++]);
    }
    return F;
}

llvm::Function *FunctionAST::Codegen(){
    llvm::Function *TheFunction = Proto->Codegen();
    if(!TheFunction){
        return nullptr;
    }

    if(!TheFunction->empty()){
        return (llvm::Function*)LogErrorV("Function cannot be redefined!");
    }
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    for(auto &Arg : TheFunction->args()){
        NamedValues[std::string(Arg.getName())] = &Arg;
    }

    if (llvm::Value *RetVal = Body->Codegen()){
        Builder.CreateRet(RetVal);
        llvm::verifyFunction(*TheFunction);
        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

void HandleDefinition(){
    if(auto FnAST = ParseDefinition()){
        if(auto *FnIR = FnAST->Codegen()){
            std::cout << "Generated a function definition" << std::endl;
            llvm::raw_ostream &OS = llvm::errs();
            FnIR->print(OS);
        }
    }else{
        getNextTokenP();
    }
}

void HandleTopLevelExpression(){
    if(auto FnAST = ParseTopLevelExpr()){
        if(auto *FnIR = FnAST->Codegen()){
            std::cout << "Generated a top-level definition" << std::endl;
            llvm::raw_ostream &OS = llvm::errs();
            FnIR->print(OS);
        }
    }else{
        getNextTokenP();
    }
}

int main() {
    InitializeCodegen();
    while (true) {
      getNextTokenP();
      switch (CurTok) {
      case tok_eof:
        return 0;
      case tok_semicolon:
        getNextTokenP();
        break;
      case tok_def:
        HandleDefinition();
        break;
      default:
        HandleTopLevelExpression();
        break;
      }
    }
  }