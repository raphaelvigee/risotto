//
// Created by rvigee on 10/2/19.
//

#ifndef RISOTTOV2_NODE_H
#define RISOTTOV2_NODE_H

#include <vector>
#include "lib/tokenizer/Token.h"
#include "lib/compiler/ByteResolver.h"

#define PToken const Token *

class Compiler;

class Node {
public:
    virtual ~Node() = default;

    virtual std::vector<ByteResolver *> compile(Compiler *compiler) = 0;

    virtual void symbolize(Compiler *compiler) = 0;
};


#endif //RISOTTOV2_NODE_H
