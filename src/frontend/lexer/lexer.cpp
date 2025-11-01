#include "lexer.h"
#include <cctype>
#include <unordered_map>

using namespace std;

static unordered_map<string, TokenType> keywords = {
    {"seq", TokenType::SEQ},
    {"par", TokenType::PAR},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"fun", TokenType::FUN},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"comp", TokenType::COMP},
    {"int", TokenType::INT},
    {"bool", TokenType::BOOL},
    {"string", TokenType::STRING},
    {"c_channel", TokenType::C_CHANNEL}};

Lexer::Lexer(const std::string &src)
    : source(src), position(0), line(1), column(1)
{
    current_char = source.empty() ? '\0' : source[0];
}

void Lexer::advance()
{
    if (current_char == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }
    position++;
    current_char = (position >= source.length()) ? '\0' : source[position];
}

char Lexer::peek()
{
    if (position + 1 >= source.length())
        return '\0';
    return source[position + 1];
}

void Lexer::skip_whitespace()
{
    while (current_char != '\0' && isspace(current_char))
        advance();
}
void Lexer::skip_comment()
{
    while (current_char != '\0' && current_char != '\n')
        advance();
    if (current_char == '\n')
        advance();
}

Token Lexer::read_number()
{
    string number;
    int start_line = line;
    int start_column = column;
    bool hasDot = false;
    int digitsBefore = 0;
    int digitsAfter = 0;

    // Primeiro parte inteira
    while (current_char != '\0' && isdigit(current_char))
    {
        number += current_char;
        digitsBefore++;
        advance();
    }
    // Parte fracionária opcional
    if (current_char == '.')
    {
        // Olhar próxima posição: se não houver dígito depois, não tratar como float
        if (peek() != '\0' && isdigit(peek()))
        {
            hasDot = true;
            number += current_char; // '.'
            advance();
            while (current_char != '\0' && isdigit(current_char))
            {
                number += current_char;
                digitsAfter++;
                advance();
            }
        }
        // Caso contrário não consome '.' aqui; deixa para tratador geral (será token DOT separado)
    }
    // Classificação: precisa ter pelo menos um dígito antes e depois do ponto para ser FLOAT
    if (hasDot && digitsBefore > 0 && digitsAfter > 0)
        return Token(TokenType::FLOAT, number, start_line, start_column);
    return Token(TokenType::NUMBER, number, start_line, start_column);
}

Token Lexer::read_string()
{
    string str;
    int start_line = line;
    int start_column = column;
    advance(); // skip opening quote
    while (current_char != '\0' && current_char != '"')
    {
        if (current_char == '\\' && peek() == '"')
        {
            advance(); // skip backslash
            str += '"';
            advance();
            continue;
        }
        str += current_char;
        advance();
    }
    if (current_char == '"')
        advance(); // closing quote
    return Token(TokenType::STRING_LITERAL, str, start_line, start_column);
}

Token Lexer::read_identifier()
{
    string identifier;
    int start_line = line;
    int start_column = column;
    while (current_char != '\0' && (isalnum(current_char) || current_char == '_'))
    {
        identifier += (char)tolower(current_char);
        advance();
    }
    auto it = keywords.find(identifier);
    if (it != keywords.end())
        return Token(it->second, identifier, start_line, start_column);
    return Token(TokenType::IDENTIFIER, identifier, start_line, start_column);
}

vector<Token> Lexer::tokenize()
{
    vector<Token> tokens;
    while (current_char != '\0')
    {
        skip_whitespace();
        if (current_char == '\0')
            break;
        if (current_char == '#')
        {
            skip_comment();
            continue;
        }
        if (isdigit(current_char))
        {
            tokens.push_back(read_number());
            continue;
        }
        // Não aceitar '.5' como float; somente d+.d+; caso '.' venha seguido de dígito sem parte inteira, token DOT + número posterior.
        if (current_char == '"')
        {
            tokens.push_back(read_string());
            continue;
        }
        if (isalpha(current_char) || current_char == '_')
        {
            tokens.push_back(read_identifier());
            continue;
        }
        int start_line = line;
        int start_column = column;
        switch (current_char)
        {
        case '+':
            tokens.push_back(Token(TokenType::PLUS, "+", start_line, start_column));
            break;
        case '-':
            tokens.push_back(Token(TokenType::MINUS, "-", start_line, start_column));
            break;
        case '*':
            tokens.push_back(Token(TokenType::MULTIPLY, "*", start_line, start_column));
            break;
        case '/':
            tokens.push_back(Token(TokenType::DIVIDE, "/", start_line, start_column));
            break;
        case '&':
            if (peek() == '&')
            {
                tokens.push_back(Token(TokenType::AND, "&&", start_line, start_column));
                advance();
            }
            break;
        case '|':
            if (peek() == '|')
            {
                tokens.push_back(Token(TokenType::OR, "||", start_line, start_column));
                advance();
            }
            break;
        case '=':
            if (peek() == '=')
            {
                tokens.push_back(Token(TokenType::EQUAL, "==", start_line, start_column));
                advance();
            }
            else
            {
                tokens.push_back(Token(TokenType::ASSIGN, "=", start_line, start_column));
            }
            break;
        case '!':
            if (peek() == '=')
            {
                tokens.push_back(Token(TokenType::NOT_EQUAL, "!=", start_line, start_column));
                advance();
            }
            else
            {
                tokens.push_back(Token(TokenType::NOT, "!", start_line, start_column));
            }
            break;
        case '<':
            if (peek() == '=')
            {
                tokens.push_back(Token(TokenType::LESS_EQUAL, "<=", start_line, start_column));
                advance();
            }
            else
            {
                tokens.push_back(Token(TokenType::LESS, "<", start_line, start_column));
            }
            break;
        case '>':
            if (peek() == '=')
            {
                tokens.push_back(Token(TokenType::GREATER_EQUAL, ">=", start_line, start_column));
                advance();
            }
            else
            {
                tokens.push_back(Token(TokenType::GREATER, ">", start_line, start_column));
            }
            break;
        case '(':
            tokens.push_back(Token(TokenType::LPAREN, "(", start_line, start_column));
            break;
        case ')':
            tokens.push_back(Token(TokenType::RPAREN, ")", start_line, start_column));
            break;
        case '{':
            tokens.push_back(Token(TokenType::LBRACE, "{", start_line, start_column));
            break;
        case '}':
            tokens.push_back(Token(TokenType::RBRACE, "}", start_line, start_column));
            break;
        case '[':
            tokens.push_back(Token(TokenType::LBRACKET, "[", start_line, start_column));
            break;
        case ']':
            tokens.push_back(Token(TokenType::RBRACKET, "]", start_line, start_column));
            break;
        case ';':
            tokens.push_back(Token(TokenType::SEMICOLON, ";", start_line, start_column));
            break;
        case ',':
            tokens.push_back(Token(TokenType::COMMA, ",", start_line, start_column));
            break;
        case '.':
            tokens.push_back(Token(TokenType::DOT, ".", start_line, start_column));
            break;
        default:
            // caractere desconhecido: ignorar mas poderia gerar token de erro
            // avançar para evitar loop
            break;
        }
        advance();
    }
    tokens.push_back(Token(TokenType::END, "", line, column));
    return tokens;
}
