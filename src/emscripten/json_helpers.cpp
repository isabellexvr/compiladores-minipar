#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "lexer.h"

// NOTE: Este arquivo deve conter apenas helpers de JSON; Removidos includes excessivos.

// Pequenos utilitários de escape
static std::string escape_json_str(const std::string &in)
{
    std::string out = in;
    for (size_t pos = 0; (pos = out.find('"', pos)) != std::string::npos;)
    {
        out.replace(pos, 1, "\\\"");
        pos += 2;
    }
    for (size_t pos = 0; (pos = out.find('\n', pos)) != std::string::npos;)
    {
        out.replace(pos, 1, "\\n");
    }
    return out;
}

// Tokens completos (mantém repetições)
void build_tokens_array(std::stringstream &json, const std::vector<Token> &tokens, bool categorized)
{
    json << "[";
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const auto &t = tokens[i];
        json << "{";
        if (categorized)
        {
            std::string cat;
            switch (t.type)
            {
            case TokenType::SEQ:
            case TokenType::PAR:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::INPUT:
                cat = "KEYWORD";
                break;
            case TokenType::IDENTIFIER:
                cat = "IDENTIFIER";
                break;
            case TokenType::NUMBER:
                cat = "NUMBER";
                break;
            case TokenType::ASSIGN:
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::MULTIPLY:
            case TokenType::DIVIDE:
            case TokenType::LESS_EQUAL:
            case TokenType::GREATER_EQUAL:
            case TokenType::EQUAL:
            case TokenType::NOT_EQUAL:
            case TokenType::LESS:
            case TokenType::GREATER:
                cat = "OPERATOR";
                break;
            case TokenType::LPAREN:
            case TokenType::RPAREN:
            case TokenType::LBRACE:
            case TokenType::RBRACE:
            case TokenType::SEMICOLON:
                cat = "DELIMITER";
                break;
            case TokenType::END:
                cat = "END";
                break;
            default:
                cat = "UNKNOWN";
                break;
            }
            json << "\"type\":\"" << cat << "\",";
        }
        else
        {
            json << "\"type\":" << static_cast<int>(t.type) << ",";
        }
        json << "\"value\":\"" << escape_json_str(t.value) << "\",\"line\":" << t.line << ",\"column\":" << t.column << "}";
        if (i + 1 < tokens.size())
            json << ",";
    }
    json << "]";
}

// Versão que elimina duplicados (primeira ocorrência do par tipo/valor)
void build_tokens_array_unique(std::stringstream &json, const std::vector<Token> &tokens, bool categorized)
{
    json << "[";
    std::unordered_set<std::string> seen; // key = type|value
    bool firstOut = true;
    for (const auto &t : tokens)
    {
        std::string typeStr;
        if (categorized)
        {
            switch (t.type)
            {
            case TokenType::SEQ:
            case TokenType::PAR:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::INPUT:
                typeStr = "KEYWORD";
                break;
            case TokenType::IDENTIFIER:
                typeStr = "IDENTIFIER";
                break;
            case TokenType::NUMBER:
                typeStr = "NUMBER";
                break;
            case TokenType::ASSIGN:
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::MULTIPLY:
            case TokenType::DIVIDE:
            case TokenType::LESS_EQUAL:
            case TokenType::GREATER_EQUAL:
            case TokenType::EQUAL:
            case TokenType::NOT_EQUAL:
            case TokenType::LESS:
            case TokenType::GREATER:
                typeStr = "OPERATOR";
                break;
            case TokenType::LPAREN:
            case TokenType::RPAREN:
            case TokenType::LBRACE:
            case TokenType::RBRACE:
            case TokenType::SEMICOLON:
                typeStr = "DELIMITER";
                break;
            case TokenType::END:
                typeStr = "END";
                break;
            default:
                typeStr = "UNKNOWN";
                break;
            }
        }
        std::string key = (categorized ? typeStr : std::to_string(static_cast<int>(t.type))) + "|" + t.value;
        if (seen.count(key))
            continue; // skip duplicate
        seen.insert(key);
        if (!firstOut)
            json << ",";
        firstOut = false;
        json << "{\"type\":";
        if (categorized)
            json << "\"" << typeStr << "\"";
        else
            json << static_cast<int>(t.type);
        json << ",\"value\":\"" << escape_json_str(t.value) << "\",\"line\":" << t.line << ",\"column\":" << t.column << "}";
    }
    json << "]";
}
