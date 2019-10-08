//
// Created by rvigee on 10/4/19.
//

#ifndef RISOTTOV2_TYPESTABLE_H
#define RISOTTOV2_TYPESTABLE_H

#include <string>
#include <vector>
#include "FunctionsTable.h"

class TypeEntry {
public:
    TypeEntry(std::string name);

    std::string name;
    FunctionsTable functions;
    FunctionsTable operators;
    FunctionsTable prefixes;

    bool canReceiveType(TypeEntry *type);

    FunctionEntry *addFunction(const std::string& selfName, bool asReference, FunctionEntry *entry);
    FunctionEntry *addOperator(const std::string& selfName, bool asReference, FunctionEntry *entry);
    FunctionEntry *addPrefix(const std::string& selfName, bool asReference, FunctionEntry *entry);

    virtual bool isFunction();
    virtual FunctionTypeEntry *asFunctionTypeEntry();

    virtual ~TypeEntry() = default;

    void addSelf(FunctionEntry *entry);

    void addSelf(const std::string &selfName, bool asReference, FunctionEntry *entry);
};

class FunctionTypeEntry: public TypeEntry {
public:
    FunctionEntry *function;

    FunctionTypeEntry(std::string name, FunctionEntry *function);

    bool isFunction() override;

    FunctionTypeEntry *asFunctionTypeEntry() override;
};

class TypesTable {
private:
    std::vector<TypeEntry *> entries;
public:
    TypeEntry *find(const std::string& name);
    TypeEntry *add(std::string name);
};


#endif //RISOTTOV2_TYPESTABLE_H
