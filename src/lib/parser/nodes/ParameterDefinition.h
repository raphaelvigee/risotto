//
// Created by rvigee on 10/2/19.
//

#ifndef RISOTTOV2_PARAMETERDEFINITION_H
#define RISOTTOV2_PARAMETERDEFINITION_H


#include <lib/tokenizer/Token.h>

class TypeDescriptor;

class ParameterDefinition {
public:
    const Token *name;
    TypeDescriptor *type;
    bool asReference;

    ParameterDefinition(const Token *name, TypeDescriptor *type, bool asReference);
    ParameterDefinition(const std::string& name, TypeDescriptor *type, bool asReference);
};


#endif //RISOTTOV2_PARAMETERDEFINITION_H
