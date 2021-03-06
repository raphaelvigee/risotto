//
// Created by rvigee on 10/11/19.
//

#ifndef RISOTTOV2_TypeDefinition_H
#define RISOTTOV2_TypeDefinition_H

#include <string>
#include <vector>
#include "FunctionsTable.h"
#include "VariablesTable.h"

class InterfaceTypeDescriptor;

class TypeDefinition {
public:
    FunctionsTable functions = FunctionsTable();
    FunctionsTable operators = FunctionsTable();
    FunctionsTable prefixes = FunctionsTable();

    TypeDefinition();

    explicit TypeDefinition(const ValueTypeContainer *vtc);

    virtual bool canReceiveType(TypeDefinition *type);

    FunctionEntry *addFunction(ParameterDefinition *self, FunctionEntry *entry);

    FunctionEntry *addOperator(ParameterDefinition *self, FunctionEntry *entry);

    FunctionEntry *addPrefix(ParameterDefinition *self, FunctionEntry *entry);

    virtual ~TypeDefinition() = default;

    virtual bool isSame(TypeDefinition *other);

    const ValueTypeContainer *getVTC();

    virtual const ValueTypeContainer *resolveVTC() = 0;

protected:
    static void addSelf(ParameterDefinition *self, FunctionEntry *entry);

private:
    const ValueTypeContainer *vtc = nullptr;
};

class NilTypeDefinition : public TypeDefinition {
public:
    static NilTypeDefinition Def;

    NilTypeDefinition();

    bool isSame(TypeDefinition *other) override;

    const ValueTypeContainer *resolveVTC() override;
};

class ArrayTypeDefinition : public TypeDefinition {
public:
    TypeDescriptor *element;

    explicit ArrayTypeDefinition(TypeDescriptor *element);

    bool isSame(TypeDefinition *other) override;

    const ValueTypeContainer *resolveVTC() override;
};

class ScalarTypeDefinition : public TypeDefinition {
public:
    std::string name;

    ScalarTypeDefinition(std::string name, const ValueTypeContainer *vtc);

    bool isSame(TypeDefinition *other) override;

    const ValueTypeContainer *resolveVTC() override;
};

class FunctionTypeDefinition : public TypeDefinition {
public:
    FunctionTypeDescriptor *descriptor;

    explicit FunctionTypeDefinition(FunctionTypeDescriptor *descriptor);

    bool canReceiveType(TypeDefinition *type) override;

    bool isSame(TypeDefinition *other) override;

    const ValueTypeContainer *resolveVTC() override;
};

class StructTypeDefinition : public TypeDefinition {
public:
    VariablesTable fields;
    std::vector<FunctionEntry *> constructors;

    explicit StructTypeDefinition(VariablesTable fields);

    FunctionEntry *addConstructor(ParameterDefinition *self, FunctionEntry *entry);

    int getFieldIndex(VariableEntry *entry);

    const ValueTypeContainer *resolveVTC() override;
};

class InterfaceTypeDefinition : public TypeDefinition {
public:
    InterfaceTypeDescriptor *descriptor;

    explicit InterfaceTypeDefinition(InterfaceTypeDescriptor *descriptor,
                                     const std::vector<FunctionEntry *> &functions);

    bool canReceiveType(TypeDefinition *type) override;

    const ValueTypeContainer *resolveVTC() override;
};

#endif //RISOTTOV2_TypeDefinition_H
