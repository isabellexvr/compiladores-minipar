#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <vector>

// Forward declarations
struct ASTNode;
struct ProgramNode;
struct SeqNode;
struct AssignmentNode;
struct PrintNode;
struct BinaryOpNode;
struct NumberNode;
struct IdentifierNode;

// Nós da AST
struct ASTNode {
    virtual ~ASTNode() = default;
};

struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
};

struct SeqNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
};

struct AssignmentNode : public ASTNode {
    std::string identifier;
    std::unique_ptr<ASTNode> expression;
};

struct PrintNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
};

struct BinaryOpNode : public ASTNode {
    TokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
};

struct NumberNode : public ASTNode {
    int value;
};

struct IdentifierNode : public ASTNode {
    std::string name;
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current_token;
    
    Token& current();
    Token& peek();
    void consume();
    bool match(TokenType type);
    
    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_expression();
    std::unique_ptr<ASTNode> parse_term();
    std::unique_ptr<ASTNode> parse_factor();
    
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();
};

#endif