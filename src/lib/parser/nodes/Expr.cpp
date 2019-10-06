//
// Created by rvigee on 10/2/19.
//

#include "Expr.h"

TypesEntries Expr::getReturnType(Compiler *compiler) {
    if (_isReturnTypesInit) {
        return _returnTypes;
    }

    _returnTypes = computeReturnType(compiler);
    _isReturnTypesInit = true;

    return _returnTypes;
}

TypesEntries Expr::computeReturnType(Compiler *compiler) {
    throw std::logic_error("computeReturnType must be implemented");
}
