#include "parser.h"
#include <iostream>
#include <sstream>

using namespace std;

Parser::Parser(const vector<Token>& tokens) 
    : tokens(tokens), current_token(0) {}

Token& Parser::current() {
    return tokens[current_token];
}

Token& Parser::peek() {
    static Token eof(TokenType::END_OF_FILE, "");
    if (current_token + 1 < tokens.size()) {
        return tokens[current_token + 1];
    }
    return eof;
}

void Parser::consume() {
    if (current_token < tokens.size() && 
        tokens[current_token].type != TokenType::END_OF_FILE) {
        current_token++;
    }
}

bool Parser::match(TokenType type) {
    return current().type == type;
}

unique_ptr<ProgramNode> Parser::parse() {
    auto program = make_unique<ProgramNode>();
    
    while (!match(TokenType::END_OF_FILE)) {
        if (match(TokenType::SEQ)) {
            consume(); // Consumir SEQ
            auto seq = make_unique<SeqNode>();
            
            while (!match(TokenType::END_OF_FILE)) {
                auto stmt = parse_statement();
                if (stmt) {
                    seq->statements.push_back(move(stmt));
                } else {
                    break;
                }
            }
            
            program->statements.push_back(move(seq));
        } else {
            auto stmt = parse_statement();
            if (stmt) {
                program->statements.push_back(move(stmt));
            }
        }
    }
    
    return program;
}

unique_ptr<ASTNode> Parser::parse_statement() {
    if (match(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        // Atribuição
        string identifier = current().value;
        consume(); // Consumir identificador
        consume(); // Consumir '='
        
        auto assignment = make_unique<AssignmentNode>();
        assignment->identifier = identifier;
        assignment->expression = parse_expression();
        
        if (match(TokenType::SEMICOLON)) {
            consume();
        }
        
        return assignment;
    } else if (match(TokenType::PRINT)) {
        // Print
        consume(); // Consumir print
        
        auto print_node = make_unique<PrintNode>();
        print_node->expression = parse_expression();
        
        if (match(TokenType::SEMICOLON)) {
            consume();
        }
        
        return print_node;
    }
    
    // Se não reconhecer, pular token
    consume();
    return nullptr;
}

unique_ptr<ASTNode> Parser::parse_expression() {
    auto left = parse_term();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        TokenType op = current().type;
        consume();
        
        auto right = parse_term();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = move(left);
        bin_op->right = move(right);
        
        left = move(bin_op);
    }
    
    return left;
}

unique_ptr<ASTNode> Parser::parse_term() {
    auto left = parse_factor();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        TokenType op = current().type;
        consume();
        
        auto right = parse_factor();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = move(left);
        bin_op->right = move(right);
        
        left = move(bin_op);
    }
    
    return left;
}

unique_ptr<ASTNode> Parser::parse_factor() {
    if (match(TokenType::NUMBER)) {
        auto num_node = make_unique<NumberNode>();
        stringstream ss(current().value);
        ss >> num_node->value;
        consume();
        return num_node;
    } else if (match(TokenType::IDENTIFIER)) {
        auto id_node = make_unique<IdentifierNode>();
        id_node->name = current().value;
        consume();
        return id_node;
    } else if (match(TokenType::LPAREN)) {
        consume(); // Consumir '('
        auto expr = parse_expression();
        if (match(TokenType::RPAREN)) {
            consume(); // Consumir ')'
        }
        return expr;
    }
    
    // Erro - retornar nullptr
    consume();
    return nullptr;
}