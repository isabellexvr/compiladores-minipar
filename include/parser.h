#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast_nodes.h"
#include <memory>
#include <vector>

class Parser
{
private:
    std::vector<Token> tokens;
    size_t current_token;

    Token &current();
    Token &peek();
    void consume();
    bool match(TokenType type);

    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_expression();
    std::unique_ptr<ASTNode> parse_comparison();
    std::unique_ptr<ASTNode> parse_term();
    std::unique_ptr<ASTNode> parse_factor();
    std::unique_ptr<ASTNode> parse_primary();
    std::unique_ptr<ASTNode> parse_while_statement();
    std::unique_ptr<SeqNode> parse_seq_block();

public:
    Parser(const std::vector<Token> &tokens);
    std::unique_ptr<ProgramNode> parse();
};

#endif