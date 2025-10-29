#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "tac_generator.h"
#include "arm_generator.h"

using namespace std;

string read_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Erro ao abrir arquivo: " << filename << endl;
        return "";
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Uso: " << argv[0] << " <arquivo.minipar>" << endl;
        return 1;
    }
    
    string source_code = read_file(argv[1]);
    if (source_code.empty()) {
        return 1;
    }
    
    cout << "=== ANALISADOR LÉXICO ===" << endl;
    Lexer lexer(source_code);
    auto tokens = lexer.tokenize();
    
    // Mostrar apenas alguns tokens para não poluir a saída
    int count = 0;
    for (const auto& token : tokens) {
        if (count++ > 20) {
            cout << "... (mais tokens)" << endl;
            break;
        }
        cout << "Token: " << static_cast<int>(token.type) 
             << " Valor: '" << token.value << "'" 
             << " Linha: " << token.line << endl;
    }
    
    cout << "\n=== ANALISADOR SINTÁTICO ===" << endl;
    Parser parser(tokens);
    auto ast = parser.parse();
    
    if (ast) {
        cout << "AST gerada com sucesso!" << endl;
        
        // Imprimir AST
        ASTPrinter printer;
        cout << "\n=== ÁRVORE SINTÁTICA ===" << endl;
        cout << printer.print(*ast) << endl;
        
        cout << "\n=== CÓDIGO DE TRÊS ENDEREÇOS ===" << endl;
        TACGenerator tac_gen;
        auto tac = tac_gen.generate(ast.get());
        tac_gen.print_tac();
        
        cout << "\n=== CÓDIGO ASSEMBLY ARMv7 ===" << endl;
        ARMGenerator arm_gen;
        auto arm_code = arm_gen.generate(tac);
        arm_gen.print_arm();
    } else {
        cout << "Erro na análise sintática!" << endl;
    }
    
    return 0;
}