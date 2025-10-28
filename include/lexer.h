#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    // Palavras-chave
    SEQ, PAR, IF, ELSE, WHILE, PRINT, INPUT,
    // Tipos
    INT, BOOL, STRING, C_CHANNEL,
    // Identificadores e literais
    IDENTIFIER, NUMBER, STRING_LITERAL,
    // Operadores
    PLUS, MINUS, MULTIPLY, DIVIDE, ASSIGN,
    // Comparação
    EQUALS, NOT_EQUALS, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    // Delimitadores
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA,
    // Comentário e fim
    COMMENT, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l = 1, int c = 1) 
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;
    char current_char;
    
    void advance();
    char peek();
    void skip_whitespace();
    void skip_comment();
    Token read_number();
    Token read_string();
    Token read_identifier();
    
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();
};

#endif