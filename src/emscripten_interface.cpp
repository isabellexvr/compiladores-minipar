#include "emscripten_interface.h"
#include "lexer.h"
#include "parser.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include "ast_printer.h"
#include "symbol_table.h"
#include <sstream>
#include <vector>
#include <memory>

using namespace std;

// Função principal que será chamada do JavaScript - VERSÃO TEXTO
EMSCRIPTEN_KEEPALIVE
char *compile_minipar(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream result;

        // === ANALISADOR LÉXICO ===
        result << "=== ANALISADOR LÉXICO ===\n";

        Lexer lexer(source);
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

        if (ast)
        {
            result << "AST gerada com sucesso!\n";

            // === TABELA DE SÍMBOLOS ===
            SymbolTable symtab;
            build_symbol_table(ast.get(), symtab);
            result << "\n=== TABELA DE SÍMBOLOS ===\n";
            print_symbol_table(symtab, result);

            // === ÁRVORE SINTÁTICA ===
            ASTPrinter printer;
            result << "\n=== ÁRVORE SINTÁTICA ===\n";
            result << printer.print(*ast) << "\n";

            // === CÓDIGO DE TRÊS ENDEREÇOS ===
            result << "\n=== CÓDIGO DE TRÊS ENDEREÇOS ===\n";
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());
            
            // ✅ GARANTIR que o TAC seja impresso em formato textual
            // Criar um stringstream separado para o TAC
            stringstream tac_stream;
            tac_gen.print_tac(tac_stream);
            string tac_text = tac_stream.str();
            
            // Se o TAC não foi impresso, forçar a impressão
            if (tac_text.empty()) {
                tac_gen.print_tac(result); // Imprime diretamente no resultado
            } else {
                result << tac_text; // Usa o texto já gerado
            }

            // === CÓDIGO ASSEMBLY ARMv7 ===
            // === CÓDIGO ASSEMBLY ARMv7 ===
            result << "\n=== CÓDIGO ASSEMBLY ARMv7 ===\n";
            ARMGenerator arm_gen;
            auto tac_for_arm = tac_gen.generate(ast.get()); // Gerar TAC específico para ARM
            auto arm_code = arm_gen.generate(tac_for_arm);

            // AGORA USE O print_arm() que funciona no main.cpp
            //result << "\n// Código ARM via print_arm():\n";
            arm_gen.print_arm(result); // ← ESTA É A LINHA CHAVE!
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

// ✅ FUNÇÃO ALTERNATIVA: Versão JSON (se quiser usar no futuro)
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

            // Symbol table JSON
            SymbolTable symtab;
            build_symbol_table(ast.get(), symtab);
            json_result << "\"symbols\":[";
            auto all = symtab.get_all_symbols();
            for (size_t i = 0; i < all.size(); ++i)
            {
                const auto &sym = all[i];
                string stype;
                switch (sym.type)
                {
                case SymbolType::VARIABLE:
                    stype = "VAR";
                    break;
                case SymbolType::FUNCTION:
                    stype = "FUN";
                    break;
                case SymbolType::CHANNEL:
                    stype = "CHAN";
                    break;
                }
                json_result << "{\"name\":\"" << sym.name << "\",\"symbolType\":\"" << stype
                            << "\",\"dataType\":\"" << (sym.data_type.empty() ? "?" : sym.data_type) << "\"}";
                if (i < all.size() - 1)
                    json_result << ",";
            }
            json_result << "],";

            // TAC estruturado
            json_result << "\"tac\":[";
            for (size_t i = 0; i < tac.size(); ++i)
            {
                const auto &instr = tac[i];
                json_result << "{"
                            << "\"result\":\"" << instr.result << "\","
                            << "\"arg1\":\"" << instr.arg1 << "\","
                            << "\"op\":\"" << instr.op << "\","
                            << "\"arg2\":\"" << instr.arg2 << "\","
                            << "\"type\":\"";

                // Classificação de tipos
                if (instr.op == "print") {
                    json_result << "PRINT";
                } else if (instr.op == "label") {
                    json_result << "LABEL";
                } else if (instr.op == "if_false") {
                    json_result << "CONDITIONAL_JUMP";
                } else if (instr.op == "goto") {
                    json_result << "JUMP";
                } else if (instr.op.empty()) {
                    json_result << "ASSIGNMENT";
                } else {
                    json_result << "BINARY_OP";
                }

                json_result << "\"}";

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

// ✅ FUNÇÃO ESPECÍFICA PARA TAC (se quiser usar no futuro)
EMSCRIPTEN_KEEPALIVE
char *compile_minipar_tac(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream json_result;

        json_result << "{";

        // Análise Léxica e Sintática
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        if (ast)
        {
            // Gerar TAC
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());

            json_result << "\"success\": true,";
            json_result << "\"tac\":[";

            for (size_t i = 0; i < tac.size(); ++i)
            {
                const auto &instr = tac[i];
                json_result << "{"
                            << "\"result\":\"" << instr.result << "\","
                            << "\"op\":\"" << instr.op << "\","
                            << "\"arg1\":\"" << instr.arg1 << "\","
                            << "\"arg2\":\"" << instr.arg2 << "\","
                            << "\"type\":\"";

                // Classificar o tipo de instrução para facilitar no frontend
                if (instr.op == "print")
                {
                    json_result << "PRINT";
                }
                else if (instr.op == "label")
                {
                    json_result << "LABEL";
                }
                else if (instr.op == "if_false")
                {
                    json_result << "CONDITIONAL_JUMP";
                }
                else if (instr.op == "goto")
                {
                    json_result << "JUMP";
                }
                else if (instr.op.empty())
                {
                    json_result << "ASSIGNMENT";
                }
                else
                {
                    json_result << "BINARY_OP";
                }

                json_result << "\"}";

                if (i < tac.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "]";
        }
        else
        {
            json_result << "\"success\": false,";
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
        string error = "{\"success\":false,\"error\":\"Exception: " + string(e.what()) + "\"}";
        char *error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}