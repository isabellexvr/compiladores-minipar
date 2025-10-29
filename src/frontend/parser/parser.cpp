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
    
    while (!match(END_OF_FILE)) {
        if (match(C_CHANNEL)) {
            // ... código do channel ...
        } else if (match(SEQ)) {
            // SEQ block
            auto seq = parse_seq_block();
            program->statements.push_back(std::move(seq));
        } else if (match(PAR)) {
            // PAR block
            consume(); // consumir PAR
            auto par = make_unique<ParNode>();
            
            // Parse todos os SEQ dentro do PAR
            while (match(SEQ)) {
                auto seq = parse_seq_block();
                par->statements.push_back(std::move(seq));
            }
            
            program->statements.push_back(std::move(par));
        } else {
            auto stmt = parse_statement();
            if (stmt) {
                program->statements.push_back(std::move(stmt));
            } else {
                consume(); // pular token desconhecido
            }
        }
    }
    
    return program;
}

unique_ptr<SeqNode> Parser::parse_seq_block() {
    consume(); // consumir SEQ
    auto seq = make_unique<SeqNode>();
    
    // Parse statements até encontrar outro SEQ, PAR, ou fim
    while (!match(END_OF_FILE) && !match(SEQ) && !match(PAR)) {
        auto stmt = parse_statement();
        if (stmt) {
            seq->statements.push_back(std::move(stmt));
        } else {
            break;
        }
    }
    
    return seq;
}

unique_ptr<ASTNode> Parser::parse_statement() {
    if (match(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        // Assignment
        string identifier = current().value;
        consume(); // identifier
        consume(); // '='
        
        auto assignment = make_unique<AssignmentNode>();
        assignment->identifier = identifier;
        assignment->expression = parse_expression();
        
        if (match(TokenType::SEMICOLON)) {
            consume();
        }
        
        return assignment;
        
    } else if (match(TokenType::PRINT)) {
        // Print
        consume();
        
        auto print_node = make_unique<PrintNode>();
        print_node->expression = parse_expression();
        
        if (match(TokenType::SEMICOLON)) {
            consume();
        }
        
        return print_node;
        
    } else if (match(TokenType::INPUT)) {
        // Input
        consume();
        
        if (match(TokenType::IDENTIFIER)) {
            auto input_node = make_unique<InputNode>();
            input_node->identifier = current().value;
            consume();
            
            if (match(TokenType::SEMICOLON)) {
                consume();
            }
            
            return input_node;
        }
    } else if (match(TokenType::WHILE)) {
        return parse_while_statement();
    }
    
    // Se não reconhecer, pular token
    consume();
    return nullptr;
}

unique_ptr<ASTNode> Parser::parse_comparison() {
    auto left = parse_term();
    
    while (match(LESS_EQUAL) || match(LESS) || match(GREATER) || match(GREATER_EQUAL) ||
           match(EQUALS) || match(NOT_EQUALS)) {
        TokenType op = current().type;
        consume();
        
        auto right = parse_term();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);
        
        left = std::move(bin_op);
    }
    
    return left;
}

unique_ptr<ASTNode> Parser::parse_expression() {
    return parse_comparison();  // MUDAR PARA ISSO
    // REMOVER o código antigo abaixo:
    /*
    auto left = parse_term();
    
    while (match(PLUS) || match(MINUS)) {
        TokenType op = current().type;
        consume();
        
        auto right = parse_term();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);
        
        left = std::move(bin_op);
    }
    
    return left;
    */
}

unique_ptr<ASTNode> Parser::parse_term() {
    auto left = parse_factor();
    
    while (match(PLUS) || match(MINUS)) {  // + e - vão AQUI no term
        TokenType op = current().type;
        consume();
        
        auto right = parse_factor();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);
        
        left = std::move(bin_op);
    }
    
    return left;
}

unique_ptr<ASTNode> Parser::parse_factor() {
    auto left = parse_primary();
    
    while (match(MULTIPLY) || match(DIVIDE)) {  // * e / vão AQUI no factor
        TokenType op = current().type;
        consume();
        
        auto right = parse_primary();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);
        
        left = std::move(bin_op);
    }
    
    return left;
}

unique_ptr<ASTNode> Parser::parse_primary() {
    if (match(NUMBER)) {
        auto num_node = make_unique<NumberNode>();
        stringstream ss(current().value);
        ss >> num_node->value;
        consume();
        return num_node;
    } else if (match(IDENTIFIER)) {
        auto id_node = make_unique<IdentifierNode>();
        id_node->name = current().value;
        consume();
        return id_node;
    } else if (match(LPAREN)) {
        consume(); // Consumir '('
        auto expr = parse_expression();
        if (match(RPAREN)) {
            consume(); // Consumir ')'
        }
        return expr;
    }
    
    // Erro - retornar nullptr
    consume();
    return nullptr;
}

unique_ptr<ASTNode> Parser::parse_while_statement() {
    consume(); // consumir 'while'
    
    auto while_node = make_unique<WhileNode>();
    
    // Consumir '('
    if (match(LPAREN)) {
        consume();
    }
    
    // Parse condition (expressão completa)
    while_node->condition = parse_expression();
    
    // Consumir ')'
    if (match(RPAREN)) {
        consume();
    }
    
    // Parse body - assumindo que as próximas instruções pertencem ao while
    // Em uma versão mais robusta, deveríamos detectar blocos com {}
    auto body_seq = make_unique<SeqNode>();
    
    // Parse as instruções que provavelmente são do corpo do while
    // Por enquanto, vamos assumir que são as próximas 2 instruções
    for (int i = 0; i < 2 && !match(END_OF_FILE); i++) {
        auto stmt = parse_statement();
        if (stmt) {
            body_seq->statements.push_back(std::move(stmt));
        }
    }
    
    while_node->body = std::move(body_seq);
    return while_node;
}