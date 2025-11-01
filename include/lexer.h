#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

enum TokenType
{
    // Palavras-chave
    SEQ,
    PAR,
    IF,
    ELSE,
    WHILE,
    PRINT,
    INPUT,
    FUN,
    RETURN,
    TRUE,
    FALSE,
    COMP,
    // Tipos
    INT,
    BOOL,
    STRING,
    C_CHANNEL,
    // Identificadores e literais
    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,
    // Operadores aritméticos / atribuição
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,        // =
                   // Operadores de comparação
    EQUAL,         // ==
    NOT_EQUAL,     // !=
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=
                   // Operadores lógicos
    AND,           // &&
    OR,            // ||
    NOT,           // !
                   // Delimitadores
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    DOT,
    // Comentário e fim
    COMMENT,
    END
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, const std::string &v, int l = 1, int c = 1)
        : type(t), value(v), line(l), column(c) {}
};

class Lexer
{
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
    Lexer(const std::string &src);
    std::vector<Token> tokenize();
};

#endif