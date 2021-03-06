//
// Created by rvigee on 10/8/19.
//

#ifndef RISOTTOV2_ARRAYEXPR_H
#define RISOTTOV2_ARRAYEXPR_H


#include <lib/parser/nodes/Expr.h>
#include <lib/parser/nodes/TypeDescriptor.h>
#include "lib/compiler/ReturnTypes.h"

class ArrayExpr: public Expr {
public:
    TypeDescriptor *type;
    std::vector<Expr *> elements;

    ArrayExpr(TypeDescriptor *type, std::vector<Expr *> elements);

    std::vector<ByteResolver *> compile(Compiler *compiler) override;
protected:
    ReturnTypes computeReturnType(Compiler *compiler) override;

public:
    void symbolize(Compiler *compiler) override;
};


#endif //RISOTTOV2_ARRAYEXPR_H
