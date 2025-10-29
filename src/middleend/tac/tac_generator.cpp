#include "tac_generator.h"
#include "ast_nodes.h"
#include <iostream>

using namespace std;

TACGenerator::TACGenerator() : temp_counter(0), label_counter(0) {}

string TACGenerator::new_temp() {
    return "t" + to_string(temp_counter++);
}

vector<TACInstruction> TACGenerator::generate(ProgramNode* program) {
    instructions.clear();
    temp_counter = 0;
    label_counter = 0;
    
    if (!program) return instructions;
    
    for (auto& stmt : program->statements) {
        if (auto seq = dynamic_cast<SeqNode*>(stmt.get())) {
            for (auto& seq_stmt : seq->statements) {
                generate_statement(seq_stmt.get());
            }
        } else if (auto par = dynamic_cast<ParNode*>(stmt.get())) {
            // Para PAR, processamos todos os SEQ em paralelo
            // Em um compilador real, isso geraria código para threads
            // Por enquanto, vamos processar sequencialmente
            for (auto& par_stmt : par->statements) {
                if (auto seq = dynamic_cast<SeqNode*>(par_stmt.get())) {
                    for (auto& seq_stmt : seq->statements) {
                        generate_statement(seq_stmt.get());
                    }
                }
            }
        } else {
            generate_statement(stmt.get());
        }
    }

    cout << "=== TAC DEBUG ===" << endl;
    for (const auto& instr : instructions) {
        if (instr.op == "print") {
            cout << "print " << instr.arg1 << endl;
        } else if (instr.op.empty()) {
            cout << instr.result << " = " << instr.arg1 << endl;
        } else if (instr.op == "label") {
            cout << instr.result << ":" << endl;
        } else if (instr.op == "if_false") {
            cout << "if_false " << instr.arg1 << " goto " << instr.arg2 << endl;
        } else if (instr.op == "goto") {
            cout << "goto " << instr.arg2 << endl;
        } else {
            cout << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << endl;
        }
    }
    cout << "=================" << endl;
    
    return instructions;
    
    return instructions;
}

void TACGenerator::generate_statement(ASTNode* stmt) {
    if (!stmt) return;
    
    if (auto assignment = dynamic_cast<AssignmentNode*>(stmt)) {
        string temp = generate_expression(assignment->expression.get());
        instructions.push_back(TACInstruction(assignment->identifier, temp));
    } else if (auto print_node = dynamic_cast<PrintNode*>(stmt)) {
        string temp = generate_expression(print_node->expression.get());
        instructions.push_back(TACInstruction("", temp, "print"));
    } else if (auto while_node = dynamic_cast<WhileNode*>(stmt)) {
        // Gerar labels para o loop
        string start_label = "L" + to_string(label_counter++);
        string end_label = "L" + to_string(label_counter++);
        
        // Label de início do loop
        instructions.push_back(TACInstruction(start_label, "", "label"));
        
        // Gerar condição - CORRIGIDO: usar generate_expression
        string cond_temp = generate_expression(while_node->condition.get());
        instructions.push_back(TACInstruction("", cond_temp, "if_false", end_label));
        
        // Gerar corpo do while
        if (auto body_seq = dynamic_cast<SeqNode*>(while_node->body.get())) {
            for (auto& body_stmt : body_seq->statements) {
                generate_statement(body_stmt.get());
            }
        } else {
            generate_statement(while_node->body.get());
        }
        
        // Jump de volta para o início
        instructions.push_back(TACInstruction("", "", "goto", start_label));
        
        // Label de fim do loop
        instructions.push_back(TACInstruction(end_label, "", "label"));
    }
}

string TACGenerator::generate_expression(ASTNode* node) {
    if (!node) return "error";
    
    if (auto num = dynamic_cast<NumberNode*>(node)) {
        return to_string(num->value);
    } else if (auto id = dynamic_cast<IdentifierNode*>(node)) {
        return id->name;
    } else if (auto bin_op = dynamic_cast<BinaryOpNode*>(node)) {
        string left = generate_expression(bin_op->left.get());
        string right = generate_expression(bin_op->right.get());
        string temp = new_temp();
        
        string op;
        switch (bin_op->op) {
            case PLUS: op = "+"; break;
            case MINUS: op = "-"; break;
            case MULTIPLY: op = "*"; break;
            case DIVIDE: op = "/"; break;
            case EQUALS: op = "=="; break;
            case NOT_EQUALS: op = "!="; break;
            case LESS: op = "<"; break;
            case LESS_EQUAL: op = "<="; break;
            case GREATER: op = ">"; break;
            case GREATER_EQUAL: op = ">="; break;
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