#include "emscripten_interface.h"
#include "lexer.h"
#include "parser.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include <sstream>
#include <vector>
#include <memory>

using namespace std;

// Função principal que será chamada do JavaScript
EMSCRIPTEN_KEEPALIVE
char* compile_minipar(const char* source_code) {
    try {
        string source(source_code);
        stringstream result;
        
        // 1. Análise Léxica
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        result << "=== TOKENS ===\n";
        for (const auto& token : tokens) {
            result << "Token: " << static_cast<int>(token.type) 
                   << " Valor: '" << token.value << "'\n";
        }
        
        // 2. Análise Sintática
        Parser parser(tokens);
        auto ast = parser.parse();
        
        if (ast) {
            result << "\n=== AST GERADA ===\nSucesso!\n";
            
            // 3. Geração de Código de 3 Endereços
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());
            
            result << "\n=== CÓDIGO 3 ENDEREÇOS ===\n";
            stringstream tac_stream;
            for (const auto& instr : tac) {
                if (instr.op == "print") {
                    tac_stream << "print " << instr.arg1 << "\n";
                } else if (instr.op.empty()) {
                    tac_stream << instr.result << " = " << instr.arg1 << "\n";
                } else {
                    tac_stream << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << "\n";
                }
            }
            result << tac_stream.str();
            
            // 4. Geração de Código ARM
            ARMGenerator arm_gen;
            auto arm_code = arm_gen.generate(tac);
            
            result << "\n=== CÓDIGO ARMv7 ===\n";
            for (const auto& line : arm_code) {
                result << line << "\n";
            }
            
        } else {
            result << "\n=== ERRO SINTÁTICO ===\n";
        }
        
        // Retornar string para JavaScript
        string output = result.str();
        char* output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
        
    } catch (const exception& e) {
        string error = "Erro durante compilação: " + string(e.what());
        char* error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}

// Liberar memória alocada
EMSCRIPTEN_KEEPALIVE
void free_string(char* str) {
    delete[] str;
}

EMSCRIPTEN_KEEPALIVE
char* compile_minipar_json(const char* source_code) {
    try {
        string source(source_code);
        stringstream json_result;
        
        json_result << "{";
        
        // 1. Análise Léxica - Retornar tokens estruturados
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        json_result << "\"tokens\":[";
        for (size_t i = 0; i < tokens.size(); ++i) {
            const auto& token = tokens[i];
            json_result << "{"
                       << "\"type\":" << static_cast<int>(token.type) << ","
                       << "\"value\":\"" << token.value << "\","
                       << "\"line\":" << token.line << ","
                       << "\"column\":" << token.column
                       << "}";
            if (i < tokens.size() - 1) {
                json_result << ",";
            }
        }
        json_result << "],";
        
        // 2. Análise Sintática
        Parser parser(tokens);
        auto ast = parser.parse();
        
        json_result << "\"ast\":\"" << (ast ? "SUCCESS" : "ERROR") << "\",";
        
        // 3. Código de 3 Endereços
        if (ast) {
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());
            
            json_result << "\"tac\":[";
            for (size_t i = 0; i < tac.size(); ++i) {
                const auto& instr = tac[i];
                json_result << "{"
                           << "\"result\":\"" << instr.result << "\","
                           << "\"arg1\":\"" << instr.arg1 << "\","
                           << "\"op\":\"" << instr.op << "\","
                           << "\"arg2\":\"" << instr.arg2 << "\""
                           << "}";
                if (i < tac.size() - 1) {
                    json_result << ",";
                }
            }
            json_result << "],";
            
            // 4. Código ARM
            ARMGenerator arm_gen;
            auto arm_code = arm_gen.generate(tac);
            
            json_result << "\"arm\":[";
            for (size_t i = 0; i < arm_code.size(); ++i) {
                json_result << "\"" << arm_code[i] << "\"";
                if (i < arm_code.size() - 1) {
                    json_result << ",";
                }
            }
            json_result << "]";
        } else {
            json_result << "\"error\":\"Syntactic analysis failed\"";
        }
        
        json_result << "}";
        
        string output = json_result.str();
        char* output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
        
    } catch (const exception& e) {
        string error = "{\"error\":\"Exception: " + string(e.what()) + "\"}";
        char* error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}
