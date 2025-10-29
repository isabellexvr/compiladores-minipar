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
    arm_code.push_back(".data");
    arm_code.push_back("msg: .asciz \"%d\\n\"");
    arm_code.push_back(".text");
    arm_code.push_back("_start:");
    
    // Estratégia SIMPLES: usar r0-r6 para variáveis, r7 para syscall
    for (const auto& instr : tac) {
        if (instr.op == "label") {
            arm_code.push_back(instr.result + ":");
        }
        else if (instr.op == "print") {
            string reg = get_register(instr.arg1);
            arm_code.push_back("push {r0, r1}");    // salvar registradores
            arm_code.push_back("ldr r0, =msg");
            arm_code.push_back("mov r1, " + reg);
            arm_code.push_back("bl printf");
            arm_code.push_back("pop {r0, r1}");     // restaurar
        }
        else if (instr.op.empty()) {
            // Atribuição: x = y ou x = 5
            string dest_reg = get_register(instr.result);
            if (isdigit(instr.arg1[0])) {
                arm_code.push_back("mov " + dest_reg + ", #" + instr.arg1);
            } else {
                string src_reg = get_register(instr.arg1);
                if (dest_reg != src_reg) {
                    arm_code.push_back("mov " + dest_reg + ", " + src_reg);
                }
            }
        }
        else if (instr.op == "+") {
            string dest_reg = get_register(instr.result);
            string left_reg = get_register(instr.arg1);
            if (isdigit(instr.arg2[0])) {
                arm_code.push_back("add " + dest_reg + ", " + left_reg + ", #" + instr.arg2);
            } else {
                string right_reg = get_register(instr.arg2);
                arm_code.push_back("add " + dest_reg + ", " + left_reg + ", " + right_reg);
            }
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
    
    arm_code.push_back("mov r7, #1");
    arm_code.push_back("svc #0");
    
    return arm_code;
}

string ARMGenerator::get_register(const string& var) {
    if (var_registers.find(var) == var_registers.end()) {
        // Estratégia SIMPLES: r0, r1, r2, r3, r4, r5, r6
        if (register_counter < 7) {
            var_registers[var] = "r" + to_string(register_counter++);
        } else {
            // Reutilizar r6 se necessário
            var_registers[var] = "r6";
        }
    }
    return var_registers[var];
}

void ARMGenerator::print_arm() {
    for (const auto &line : arm_code)
        cout << line << endl;
}