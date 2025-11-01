#include "lexer.h"
#include <cctype>
#include <unordered_map>

using namespace std;

static unordered_map<string, TokenType> keywords = {
    {"SEQ", TokenType::SEQ},
    {"PAR", TokenType::PAR},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"fun", TokenType::FUN},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"COMP", TokenType::COMP},
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
    while (current_char != '\0' && isdigit(current_char))
    {
        number += current_char;
        advance();
    }
    return Token(TokenType::NUMBER, number, start_line, start_column);
}

Token Lexer::read_string()
{
    string str;
    int start_line = line;
    int start_column = column;
    advance();
    while (current_char != '\0' && current_char != '"')
    {
        str += current_char;
        advance();
    }
    advance();
    return Token(TokenType::STRING_LITERAL, str, start_line, start_column);
}

Token Lexer::read_identifier()
{
    string identifier;
    int start_line = line;
    int start_column = column;
    while (current_char != '\0' && (isalnum(current_char) || current_char == '_'))
    {
        identifier += current_char;
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
            break;
        }
        advance();
    }
    tokens.push_back(Token(TokenType::END, "", line, column));
    return tokens;
}
