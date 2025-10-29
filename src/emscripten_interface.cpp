#include "emscripten_interface.h"
#include "lexer.h"
#include "parser.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include "ast_printer.h" // <-- IMPORTANTE
#include <sstream>
#include <vector>
#include <memory>

using namespace std;

// Função principal que será chamada do JavaScript
EMSCRIPTEN_KEEPALIVE
char *compile_minipar(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream result;
        result << "=== DEBUG: source.length = " << source.length() << " ===\n";
        result << "=== DEBUG: source content START ===\n";
        result << source << "\n";
        result << "=== DEBUG: source content END ===\n";

        // === ANALISADOR LÉXICO ===
        result << "=== ANALISADOR LÉXICO ===\n";
        result << "\n=== DEBUG: Entrou no compile_minipar ===\n";

        Lexer lexer(source);
        result << "=== DEBUG: Tokens gerados ===\n";

        auto tokens = lexer.tokenize();

        for (const auto &token : tokens)
        {
            result << "Token: " << static_cast<int>(token.type)
                   << " Valor: '" << token.value << "' Linha: " << token.line << "\n";
        }

        // === ANALISADOR SINTÁTICO ===
        result << "\n=== ANALISADOR SINTÁTICO ===\n";
        Parser parser(tokens);
        auto ast = parser.parse();
        result << "=== DEBUG: AST parseada ===\n";

        if (ast)
        {
            result << "AST gerada com sucesso!\n";

            // === ÁRVORE SINTÁTICA ===
            ASTPrinter printer;
            result << "\n=== ÁRVORE SINTÁTICA ===\n";
            result << printer.print(*ast) << "\n";

            // === CÓDIGO DE TRÊS ENDEREÇOS ===
            result << "\n=== CÓDIGO DE TRÊS ENDEREÇOS ===\n";
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());
            tac_gen.print_tac(result);

            // === CÓDIGO ASSEMBLY ARMv7 ===
            result << "\n=== CÓDIGO ASSEMBLY ARMv7 ===\n";
            ARMGenerator arm_gen;
            auto arm_code = arm_gen.generate(tac);
            for (const auto &line : arm_code)
            {
                result << line << "\n";
            }
        }
        else
        {
            result << "Erro na análise sintática!\n";
        }

        // Retornar string para o JS
        string output = result.str();
        char *output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
    }
    catch (const exception &e)
    {
        string error = "Erro durante compilação: " + string(e.what());
        char *error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}

// Liberar memória alocada
EMSCRIPTEN_KEEPALIVE
void free_string(char *str)
{
    delete[] str;
}

EMSCRIPTEN_KEEPALIVE
char *compile_minipar_json(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream json_result;

        json_result << "{";

        // 1. Análise Léxica - Retornar tokens estruturados
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        json_result << "\"tokens\":[";
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            const auto &token = tokens[i];
            json_result << "{"
                        << "\"type\":" << static_cast<int>(token.type) << ","
                        << "\"value\":\"" << token.value << "\","
                        << "\"line\":" << token.line << ","
                        << "\"column\":" << token.column
                        << "}";
            if (i < tokens.size() - 1)
            {
                json_result << ",";
            }
        }
        json_result << "],";

        // 2. Análise Sintática
        Parser parser(tokens);
        auto ast = parser.parse();

        json_result << "\"ast\":\"" << (ast ? "SUCCESS" : "ERROR") << "\",";

        // 3. Código de 3 Endereços
        if (ast)
        {
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());

            json_result << "\"tac\":[";
            for (size_t i = 0; i < tac.size(); ++i)
            {
                const auto &instr = tac[i];
                json_result << "{"
                            << "\"result\":\"" << instr.result << "\","
                            << "\"arg1\":\"" << instr.arg1 << "\","
                            << "\"op\":\"" << instr.op << "\","
                            << "\"arg2\":\"" << instr.arg2 << "\""
                            << "}";
                if (i < tac.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "],";

            // 4. Código ARM
            ARMGenerator arm_gen;
            auto arm_code = arm_gen.generate(tac);

            json_result << "\"arm\":[";
            for (size_t i = 0; i < arm_code.size(); ++i)
            {
                json_result << "\"" << arm_code[i] << "\"";
                if (i < arm_code.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "]";
        }
        else
        {
            json_result << "\"error\":\"Syntactic analysis failed\"";
        }

        json_result << "}";

        string output = json_result.str();
        char *output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
    }
    catch (const exception &e)
    {
        string error = "{\"error\":\"Exception: " + string(e.what()) + "\"}";
        char *error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}
