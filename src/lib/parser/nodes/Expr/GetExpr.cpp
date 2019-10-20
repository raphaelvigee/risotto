//
// Created by rvigee on 10/6/19.
//

extern "C" {
#include <lib/vm/chunk.h>
}

#include "GetExpr.h"
#include <lib/compiler/CompilerError.h>
#include <lib/compiler/utils/Utils.h>
#include "lib/compiler/Compiler.h"
#include "lib/compiler/TypeReference.h"
#include "lib/compiler/ReturnTypes.h"
#include "lib/compiler/TypeDefinition.h"

StructTypeDefinition *getStructTypeDefinition(TypeReference *typeRef) {
    if (auto structRef = dynamic_cast<StructTypeReference *>(typeRef)) {
        return dynamic_cast<StructTypeDefinition *>(structRef->getTypeDefinition());
    }

    if (auto namedRef = dynamic_cast<NamedTypeReference *>(typeRef)) {
        return dynamic_cast<StructTypeDefinition *>(namedRef->getTypeDefinition());
    }

    return nullptr;
}

GetExpr::GetExpr(Expr *callee, Token *identifier) : callee(callee), identifier(identifier) {

}

std::vector<ByteResolver *> GetExpr::compile(Compiler *compiler) {
    auto bytes = std::vector<ByteResolver *>();

    auto returnType = getReturnType(compiler);

    if (!returnType.single()) {
        throw CompilerError("Must resolve to a single symbol");
    }

    auto calleeReturnType = callee->getReturnType(compiler);

    if (auto structDef = getStructTypeDefinition(calleeReturnType[0])) {
        auto typeDef = callee->getReturnType(compiler);

        auto calleeBytes = callee->compile(compiler);
        bytes.insert(bytes.begin(), calleeBytes.begin(), calleeBytes.end());

        bytes.push_back(new ByteResolver(OP_LOAD_INSTANCE, &identifier->position));
        bytes.push_back(new ByteResolver(structDef->getFieldIndex(structDef->fields.find(identifier->lexeme)), nullptr));

        return bytes;
    }

    throw CompilerError("Unhandled type");
}

ReturnTypes GetExpr::computeReturnType(Compiler *compiler) {
    auto calleeType = callee->getReturnType(compiler);

    if (!calleeType.single()) {
        throw CompilerError("Return type has to be single", identifier->position);
    }

    auto returnTypes = ReturnTypes();

    if (auto structDef = getStructTypeDefinition(calleeType[0])) {
        auto field = structDef->fields.find(identifier->lexeme);

        if (field != nullptr) {
            returnTypes.push_back(field->typeRef);
        }
    }

    return returnTypes;
}
