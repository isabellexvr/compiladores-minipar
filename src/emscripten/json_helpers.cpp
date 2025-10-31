#include <string>
#include <sstream>
#include <vector>
#include "lexer.h"
#include "parser.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include "ast_printer.h"
#include "symbol_table.h"

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

// Função de construção da seção de tokens (usada por compile_minipar e compile_minipar_json)
void build_tokens_array(std::stringstream &json, const std::vector<Token> &tokens, bool categorized)
{
    json << "[";
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const auto &t = tokens[i];
        json << "{";
        if (categorized)
        {
            // categoria textual semelhante à função token_type_to_string mas não depende de função externa
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
