#include "arm_generator.h"
#include <iostream>

using namespace std;

ARMGenerator::ARMGenerator() : register_counter(0) {}

vector<string> ARMGenerator::generate(const vector<TACInstruction>& tac) {
    arm_code.clear();
    register_counter = 0;
    var_registers.clear();
    
    // Cabeçalho do programa ARM
    arm_code.push_back(".global _start");
    arm_code.push_back(".text");
    arm_code.push_back("_start:");
    
    for (const auto& instr : tac) {
        if (instr.op == "print") {
            // Implementação simples do print (usando syscall)
            string reg = get_register(instr.arg1);
            arm_code.push_back("mov r0, #1"); // stdout
            arm_code.push_back("ldr r1, =msg_" + instr.arg1);
            arm_code.push_back("mov r2, #4"); // tamanho
            arm_code.push_back("mov r7, #4"); // syscall write
            arm_code.push_back("svc #0");
        } else if (instr.op.empty()) {
            // Atribuição simples
            string dest_reg = get_register(instr.result);
            if (isdigit(instr.arg1[0])) {
                arm_code.push_back("mov " + dest_reg + ", #" + instr.arg1);
            } else {
                string src_reg = get_register(instr.arg1);
                arm_code.push_back("mov " + dest_reg + ", " + src_reg);
            }
        } else {
            // Operação binária
            string dest_reg = get_register(instr.result);
            string left_reg = get_register(instr.arg1);
            string right_reg = get_register(instr.arg2);
            
            if (instr.op == "+") {
                arm_code.push_back("add " + dest_reg + ", " + left_reg + ", " + right_reg);
            } else if (instr.op == "-") {
                arm_code.push_back("sub " + dest_reg + ", " + left_reg + ", " + right_reg);
            } else if (instr.op == "*") {
                arm_code.push_back("mul " + dest_reg + ", " + left_reg + ", " + right_reg);
            } else if (instr.op == "/") {
                // Divisão simples - em ARM real precisaria de mais instruções
                arm_code.push_back("// div " + dest_reg + ", " + left_reg + ", " + right_reg);
            }
        }
    }
    
    // Finalizar programa
    arm_code.push_back("mov r7, #1"); // syscall exit
    arm_code.push_back("svc #0");
    
    return arm_code;
}

string ARMGenerator::get_register(const string& var) {
    if (var_registers.find(var) == var_registers.end()) {
        var_registers[var] = allocate_register();
    }
    return var_registers[var];
}

string ARMGenerator::allocate_register() {
    if (register_counter < 8) {
        return "r" + to_string(register_counter++);
    }
    return "r7"; // Fallback
}

void ARMGenerator::print_arm() {
    for (const auto& line : arm_code) {
        cout << line << endl;
    }
}