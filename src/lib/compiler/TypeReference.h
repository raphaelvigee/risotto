//
// Created by rvigee on 10/7/19.
//

#ifndef RISOTTOV2_TYPEREFERENCE_H
#define RISOTTOV2_TYPEREFERENCE_H

#include <vector>
#include <string>

class TypeDefinition;
class FunctionTypeDefinition;

class FunctionEntry;

class Compiler;

class TypeReference {
public:
    virtual FunctionEntry *
    findFunction(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) = 0;

    virtual FunctionEntry *
    findOperator(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) = 0;

    virtual FunctionEntry *
    findPrefix(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) = 0;

    virtual bool canReceiveType(TypeReference *other) = 0;

    virtual bool isFunction();

    virtual std::string toString() = 0;

    virtual TypeDefinition *toTypeDefinition(Compiler *compiler) = 0;
};

class ArrayTypeReference : public TypeReference {
public:
    TypeReference *element;

    explicit ArrayTypeReference(TypeReference *element);

    FunctionEntry *
    findFunction(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findOperator(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findPrefix(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    bool canReceiveType(TypeReference *other) override;

    TypeDefinition *toTypeDefinition(Compiler *compiler) override;

    std::string toString() override;
};

class FunctionTypeReference : public TypeReference {
public:
    FunctionTypeDefinition *entry;

    explicit FunctionTypeReference(FunctionTypeDefinition *entry);

    FunctionEntry *
    findFunction(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findOperator(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findPrefix(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    bool canReceiveType(TypeReference *other) override;

    bool isFunction() override;

    TypeDefinition *toTypeDefinition(Compiler *compiler) override;

    std::string toString() override;
};

class ConcreteTypeReference : public TypeReference {
public:
    TypeDefinition *entry;

    explicit ConcreteTypeReference(TypeDefinition *entry);

    FunctionEntry *
    findFunction(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findOperator(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findPrefix(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    bool canReceiveType(TypeReference *other) override;

    std::string toString() override;

    TypeDefinition *toTypeDefinition(Compiler *compiler) override;
};

class InterfaceTypeReference : public TypeReference {
public:
    class Function {
        std::string name;
        std::vector<TypeReference *> arguments;
        TypeReference *returnType;

    public:
        Function(std::string name, std::vector<TypeReference *> arguments, TypeReference *returnType);
    };

    std::vector<Function *> functions;

    explicit InterfaceTypeReference(std::vector<Function *> functions);

    bool canReceiveType(TypeReference *other) override;

    FunctionEntry *
    findFunction(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findOperator(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    FunctionEntry *
    findPrefix(Compiler *compiler, const std::string &name, const std::vector<TypeReference *> &types) override;

    TypeDefinition *toTypeDefinition(Compiler *compiler) override;

    std::string toString() override;
};

class NamedTypeReference : public ConcreteTypeReference {
public:
    std::string name;

    explicit NamedTypeReference(std::string name, TypeDefinition *entry);

    std::string toString() override;
};

#endif //RISOTTOV2_TYPEREFERENCE_H