//
// Created by rvigee on 10/16/19.
//

#include <lib/compiler/utils/Utils.h>
#include "GetCallExpr.h"
#include "lib/compiler/Compiler.h"
#include "lib/compiler/TypeDefinition.h"

GetCallExpr::GetCallExpr(Expr *callee, Token *op, Token *identifier, Token *rParen, const std::vector<Expr *> &args)
        : MixedCallExpr(rParen, args), callee(callee), op(op), identifier(identifier) {}

std::vector<Expr *> GetCallExpr::getArguments(Compiler *compiler) {
    auto args = std::vector<Expr *>();
    args.push_back(callee);

    auto baseArgs = BaseCallExpr::getArguments(compiler);
    args.insert(args.end(), baseArgs.begin(), baseArgs.end());

    return args;
}

FunctionNotFoundError GetCallExpr::getFunctionNotFoundError(Compiler *compiler) {
    auto leftReturnType = callee->getReturnType(compiler);
    auto argsTypes = getArgumentsTypes(compiler);
    auto actualArgsTypes = std::vector<TypeReference *>(argsTypes.begin() + 1, argsTypes.end());

    return FunctionNotFoundError(leftReturnType[0]->toString() + "." + identifier->lexeme + "({{args}})", actualArgsTypes, identifier);
}

VariableEntry *GetCallExpr::getVariableEntry(Compiler *compiler) {
    return nullptr;
}

FunctionEntry *GetCallExpr::getFunctionEntry(Compiler *compiler) {
    auto calleeType = callee->getReturnType(compiler);

    if (!calleeType.single()) {
        throw CompilerError("Return type has to be single", identifier->position);
    }

    auto typeDef = calleeType[0]->getTypeDefinition(compiler);

    return typeDef->functions.find(identifier->lexeme, getArgumentsTypes(compiler));
}

void GetCallExpr::loadVariableEntryAddr(Compiler *compiler, std::vector<ByteResolver *> &bytes) {
    throw CompilerError("Unimplemented", identifier->position);
}

bool GetCallExpr::needAddrResolution(Compiler *compiler) {
    return act<bool>(compiler, [](FunctionTypeReference *functionRef) {
        return false;
    }, [](FunctionEntry *functionEntry) {
        return dynamic_cast<InterfaceFunctionEntry *>(functionEntry) != nullptr;
    });
}
