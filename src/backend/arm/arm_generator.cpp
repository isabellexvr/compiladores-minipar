#include "arm_generator.h"
#include <iostream>
#include <cctype>
using namespace std;

ARMGenerator::ARMGenerator() : register_counter(0), label_counter(0) {}

vector<string> ARMGenerator::generate(const vector<TACInstruction>& tac) {
    arm_code.clear();
    register_counter = 0;
    label_counter = 0;
    var_registers.clear();
    
    arm_code.push_back(".global _start");
    arm_code.push_back(".text");
    arm_code.push_back("_start:");
    
    // Estratégia simples: usar r0–r6 para variáveis
    for (const auto& instr : tac) {
        if (instr.op == "label") {
            arm_code.push_back(instr.result + ":");
        }
        else if (instr.op == "print") {
            string reg = get_register(instr.arg1);
            arm_code.push_back("mov r0, " + reg);
            arm_code.push_back("b .");
        }
        else if (instr.op.empty() || instr.op == "=") {
            // Atribuição: x = y ou x = 5
            string dest_reg = get_register(instr.result);

            if (!instr.arg1.empty()) {
                if (isdigit(instr.arg1[0])) {
                    // Ex: x = 10
                    arm_code.push_back("mov " + dest_reg + ", #" + instr.arg1);
                } else {
                    // Ex: x = y
                    string src_reg = get_register(instr.arg1);
                    if (dest_reg != src_reg) {
                        arm_code.push_back("mov " + dest_reg + ", " + src_reg);
                    }
                }
            }
        }
        else if (instr.op == "+") {
            string dest_reg = get_register(instr.result);
            string left_reg = get_register(instr.arg1);
            string right_reg = get_register(instr.arg2);
            arm_code.push_back("add " + dest_reg + ", " + left_reg + ", " + right_reg);
        }
        else if (instr.op == "*") {
            string dest_reg = get_register(instr.result);
            string left_reg = get_register(instr.arg1);
            string right_reg = get_register(instr.arg2);
            arm_code.push_back("mul " + dest_reg + ", " + left_reg + ", " + right_reg);
        }
        else if (instr.op == "<=") {
            string dest_reg = get_register(instr.result);
            string left_reg = get_register(instr.arg1);
            string right_reg = get_register(instr.arg2);
            
            string true_label = "L_true_" + to_string(label_counter++);
            string end_label = "L_end_" + to_string(label_counter++);
            
            arm_code.push_back("cmp " + left_reg + ", " + right_reg);
            arm_code.push_back("mov " + dest_reg + ", #0");
            arm_code.push_back("bgt " + end_label);
            arm_code.push_back(true_label + ":");
            arm_code.push_back("mov " + dest_reg + ", #1");
            arm_code.push_back(end_label + ":");
        }
        else if (instr.op == "if_false") {
            string cond_reg = get_register(instr.arg1);
            arm_code.push_back("cmp " + cond_reg + ", #0");
            arm_code.push_back("beq " + instr.arg2);
        }
        else if (instr.op == "goto") {
            arm_code.push_back("b " + instr.arg2);
        }
    }

    // Em vez de syscall Linux, loop final (para inspeção estável)
    arm_code.push_back("b .");
    
    return arm_code;
}

string ARMGenerator::get_register(const string& var) {
    if (var_registers.find(var) == var_registers.end()) {
        // Estratégia simples: r0–r6
        if (register_counter < 7) {
            var_registers[var] = "r" + to_string(register_counter++);
        } else {
            // Reutiliza r6 se precisar
            var_registers[var] = "r6";
        }
    }
    return var_registers[var];
}

void ARMGenerator::print_arm(std::ostream& out) {
    for (const auto &line : arm_code) {
        out << line << std::endl;
    }
}
