//
// Created by rvigee on 10/2/19.
//

#ifndef RISOTTOV2_CALLEXPR_H
#define RISOTTOV2_CALLEXPR_H


#include <vector>
#include <lib/tokenizer/Token.h>
#include <lib/parser/nodes/Expr.h>
#include <lib/compiler/CompilerError.h>
#include "lib/compiler/ReturnTypes.h"
#include <lib/compiler/TypeDefinition.h>

class BaseCallExpr : public Expr {
public:
    Token *rParen;
    std::vector<Expr *> args;

    BaseCallExpr(Token *rParen, std::vector<Expr *> args);

    std::vector<ByteResolver *> compile(Compiler *compiler) override;

    virtual std::vector<TypeDescriptor *> getArgumentsTypes(Compiler *compiler);

    virtual std::vector<Expr *> getArguments(Compiler *compiler);

    virtual FunctionNotFoundError getFunctionNotFoundError(Compiler *compiler);

    virtual bool isArgumentReference(Compiler *compiler, unsigned int i) = 0;

    virtual ReturnTypes getFunctionReturnTypes(Compiler *compiler) = 0;

    virtual void loadCallAddr(Compiler *compiler, std::vector<ByteResolver *> &bytes) = 0;

    virtual void loadArgs(Compiler *compiler, std::vector<ByteResolver *> &bytes);

    uint64_t packRefs(Compiler *compiler);

    void loadOpCall(Compiler *compiler, std::vector<ByteResolver *> &bytes);

    void loadOpCallBytecode(Compiler *compiler, std::vector<ByteResolver *> &bytes, CodeFunctionEntry *entry);

    void loadOpCallNative(Compiler *compiler, std::vector<ByteResolver *> &bytes, NativeFunctionEntry *entry);

    void symbolize(Compiler *compiler) override;
};

#endif //RISOTTOV2_CALLEXPR_H
