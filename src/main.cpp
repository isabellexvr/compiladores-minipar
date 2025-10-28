#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
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
    
    for (const auto& token : tokens) {
        cout << "Token: " << static_cast<int>(token.type) 
             << " Valor: '" << token.value << "'" 
             << " Linha: " << token.line << endl;
    }
    
    cout << "\n=== ANALISADOR SINTÁTICO ===" << endl;
    Parser parser(tokens);
    auto ast = parser.parse();
    cout << "AST gerada com sucesso!" << endl;
    
    cout << "\n=== CÓDIGO DE TRÊS ENDEREÇOS ===" << endl;
    TACGenerator tac_gen;
    auto tac = tac_gen.generate(ast.get());
    tac_gen.print_tac();
    
    cout << "\n=== CÓDIGO ASSEMBLY ARMv7 ===" << endl;
    ARMGenerator arm_gen;
    auto arm_code = arm_gen.generate(tac);
    arm_gen.print_arm();
    
    return 0;
}