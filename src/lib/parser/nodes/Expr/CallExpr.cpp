//
// Created by rvigee on 10/2/19.
//

extern "C" {
#include "lib/vm/value.h"
#include <lib/vm/utils.h>
}

#include "CallExpr.h"
#include <utility>
#include "IdentifierExpr.h"
#include "lib/parser/nodes/TypeDescriptor.h"
#include "lib/compiler/ReturnTypes.h"
#include <lib/compiler/Compiler.h>
#include <lib/compiler/CompilerError.h>
#include <lib/compiler/utils/Utils.h>

BaseCallExpr::BaseCallExpr(const Token *rParen, std::vector<Expr *> args) : rParen(rParen), args(std::move(args)) {}

std::vector<ByteResolver *> BaseCallExpr::compile(Compiler *compiler) {
    auto bytes = std::vector<ByteResolver *>();

    loadArgs(compiler, bytes);

    auto inlined = loadCallAddr(compiler, bytes);

    if (!inlined) {
        auto argc = getArgumentsTypes(compiler).size();
        auto retc = getFunctionReturnTypes(compiler).size();

        static_assert(sizeof(OP_T) == 8);
        bool refs[64] = { 0 };
        for (auto i = 0u; i < argc; ++i) {
            refs[i] = isArgumentReference(compiler, i);
        }

        auto refsn = pack64b(refs);

        bytes.push_back(new ByteResolver(OpCode::OP_CALL, rParen->position));
        bytes.push_back(new ByteResolver(argc));
        bytes.push_back(new ByteResolver(retc));
        bytes.push_back(new ByteResolver(refsn));
    }

    return bytes;
}

void BaseCallExpr::loadArgs(Compiler *compiler, std::vector<ByteResolver *> &bytes) {
    auto arguments = getArguments(compiler);

    for (auto arg : arguments) {
        auto argBytes = arg->compile(compiler);
        bytes.insert(bytes.end(), argBytes.begin(), argBytes.end());
    }
}

std::vector<TypeDescriptor *> BaseCallExpr::getArgumentsTypes(Compiler *compiler) {
    auto arguments = getArguments(compiler);

    for (auto arg : arguments) {
        auto types = arg->getReturnType(compiler);

        if (!types.single()) {
            if (types.empty()) {
                throw CompilerError("This parameter doesnt return any value");
            } else if (types.size() > 1) {
                throw CompilerError("This parameter returns more than one value");
            }
        }
    }

    return Utils::getTypes(arguments, compiler);
}

FunctionNotFoundError BaseCallExpr::getFunctionNotFoundError(Compiler *compiler) {
    auto actualArgumentsTypes = Utils::getTypes(args, compiler);

    auto argumentsTypes = getArgumentsTypes(compiler);

    return FunctionNotFoundError("func ({{args}})", argumentsTypes, rParen);
}

std::vector<Expr *> BaseCallExpr::getArguments(Compiler *compiler) {
    return args;
}

void BaseCallExpr::symbolize(Compiler *compiler) {
    for (auto arg: args) {
        arg->symbolize(compiler);
    }
}
