//
// Created by rvigee on 10/1/19.
//

#ifndef RISOTTOV2_TOKENIZER_H
#define RISOTTOV2_TOKENIZER_H

#include <vector>
#include <map>
#include "Token.h"

class Tokenizer {
public:
    static std::vector<Token> Tokenize(std::string str);

private:
    explicit Tokenizer(std::string src);

    std::string src;

    int start = 0;
    int current = start;

    int line = 0;
    int column = 0;

    std::vector<Token> tokens;

    static std::map<std::string, TokenType> keywords;

    std::vector<Token> tokenize();

    char advance();

    void scan();

    bool isAtEnd();

    void nextLine();

    void addToken(TokenType type);

    template<typename T>
    void addToken(TokenType type, T literal);

    bool match(char expected);

    void lexString();

    bool isDigit(char c);

    void number();

    char peekNext();

    char peek();

    char peekN(int n);

    bool isAlpha(char c);

    void identifier();

    bool isAlphaNumeric(char c);
};


#endif //RISOTTOV2_TOKENIZER_H
