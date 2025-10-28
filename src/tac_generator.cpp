#include "tac_generator.h"
#include <iostream>

using namespace std;

TACGenerator::TACGenerator() : temp_counter(0) {}

string TACGenerator::new_temp() {
    return "t" + to_string(temp_counter++);
}

vector<TACInstruction> TACGenerator::generate(ProgramNode* program) {
    instructions.clear();
    temp_counter = 0;
    
    for (auto& stmt : program->statements) {
        if (auto seq = dynamic_cast<SeqNode*>(stmt.get())) {
            for (auto& seq_stmt : seq->statements) {
                if (auto assignment = dynamic_cast<AssignmentNode*>(seq_stmt.get())) {
                    string temp = generate_expression(assignment->expression.get());
                    instructions.push_back(TACInstruction(assignment->identifier, temp));
                } else if (auto print_node = dynamic_cast<PrintNode*>(seq_stmt.get())) {
                    string temp = generate_expression(print_node->expression.get());
                    instructions.push_back(TACInstruction("", temp, "print"));
                }
            }
        }
    }
    
    return instructions;
}

string TACGenerator::generate_expression(ASTNode* node) {
    if (auto num = dynamic_cast<NumberNode*>(node)) {
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, to_string(num->value)));
        return temp;
    } else if (auto id = dynamic_cast<IdentifierNode*>(node)) {
        return id->name;
    } else if (auto bin_op = dynamic_cast<BinaryOpNode*>(node)) {
        string left = generate_expression(bin_op->left.get());
        string right = generate_expression(bin_op->right.get());
        string temp = new_temp();
        
        string op;
        switch (bin_op->op) {
            case TokenType::PLUS: op = "+"; break;
            case TokenType::MINUS: op = "-"; break;
            case TokenType::MULTIPLY: op = "*"; break;
            case TokenType::DIVIDE: op = "/"; break;
            default: op = "?"; break;
        }
        
        instructions.push_back(TACInstruction(temp, left, op, right));
        return temp;
    }
    
    return "error";
}

void TACGenerator::print_tac() {
    for (const auto& instr : instructions) {
        if (instr.op == "print") {
            cout << "print " << instr.arg1 << endl;
        } else if (instr.op.empty()) {
            cout << instr.result << " = " << instr.arg1 << endl;
        } else {
            cout << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << endl;
        }
    }
}