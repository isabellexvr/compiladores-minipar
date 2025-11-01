#include "tac_generator.h"
#include "ast_nodes.h"
#include <iostream>

using namespace std;

TACGenerator::TACGenerator() : temp_counter(0), label_counter(0) {}

string TACGenerator::new_temp()
{
    return "t" + to_string(temp_counter++);
}

string TACGenerator::new_label()
{
    return "L" + to_string(label_counter++);
}

vector<TACInstruction> TACGenerator::generate(ProgramNode *program)
{
    instructions.clear();
    temp_counter = 0;
    label_counter = 0;

    if (!program)
        return instructions;

    for (auto &stmt : program->statements)
    {
        if (auto seq = dynamic_cast<SeqNode *>(stmt.get()))
        {
            for (auto &seq_stmt : seq->statements)
            {
                generate_statement(seq_stmt.get());
            }
        }
        else if (auto par = dynamic_cast<ParNode *>(stmt.get()))
        {
            // Para PAR, processamos todos os SEQ em paralelo
            for (auto &par_stmt : par->statements)
            {
                if (auto seq = dynamic_cast<SeqNode *>(par_stmt.get()))
                {
                    for (auto &seq_stmt : seq->statements)
                    {
                        generate_statement(seq_stmt.get());
                    }
                }
            }
        }
        else
        {
            generate_statement(stmt.get());
        }
    }

    return instructions;
}

void TACGenerator::generate_statement(ASTNode *stmt)
{
    if (!stmt)
        return;

    if (auto assignment = dynamic_cast<AssignmentNode *>(stmt))
    {
        string temp = generate_expression(assignment->expression.get());
        instructions.push_back(TACInstruction(assignment->identifier, "=", temp));
    }
    else if (auto print_node = dynamic_cast<PrintNode *>(stmt))
    {
        string temp = generate_expression(print_node->expression.get());
        instructions.push_back(TACInstruction("", "print", temp));
    }
    else if (auto while_node = dynamic_cast<WhileNode *>(stmt))
    {
        string start_label = new_label();
        string end_label = new_label();

        // Label de início do loop
        instructions.push_back(TACInstruction(start_label, "label", ""));

        // Gerar condição
        string cond_temp = generate_expression(while_node->condition.get());

        // if_false cond_temp goto end_label
        instructions.push_back(TACInstruction("", "if_false", cond_temp, end_label));

        // Gerar corpo do while
        if (auto body_seq = dynamic_cast<SeqNode *>(while_node->body.get()))
        {
            for (auto &body_stmt : body_seq->statements)
            {
                generate_statement(body_stmt.get());
            }
        }
        else
        {
            generate_statement(while_node->body.get());
        }

        // goto start_label
        instructions.push_back(TACInstruction("", "goto", start_label));

        // Label de fim do loop
        instructions.push_back(TACInstruction(end_label, "label", ""));
    }
    else if (auto call = dynamic_cast<CallNode *>(stmt))
    {
        // evaluate args
        vector<string> argTemps;
        for (auto &a : call->args)
            argTemps.push_back(generate_expression(a.get()));
        // push args (simplified as assignments)
        for (size_t i = 0; i < argTemps.size(); ++i)
            instructions.push_back(TACInstruction("arg" + to_string(i), "=", argTemps[i]));
        instructions.push_back(TACInstruction("", call->name, "call"));
    }
    else if (auto send = dynamic_cast<SendNode *>(stmt))
    {
        // Avalia todos os argumentos e gera pacote de envio
        vector<string> temps;
        for (auto &a : send->arguments)
            temps.push_back(generate_expression(a.get()));
        // Instrução principal com contagem
        instructions.push_back(TACInstruction("", "send", send->channelName, to_string(temps.size())));
        // Instruções de argumento (canal, valor, índice)
        for (size_t i = 0; i < temps.size(); ++i)
        {
            instructions.push_back(TACInstruction(send->channelName, "send_arg", temps[i], to_string(i)));
        }
    }
    else if (auto recv = dynamic_cast<ReceiveNode *>(stmt))
    {
        // Instrução principal de receive com quantidade esperada
        instructions.push_back(TACInstruction("", "receive", recv->channelName, to_string(recv->variables.size())));
        // Bind de cada variável (var = recv canal idx)
        for (size_t i = 0; i < recv->variables.size(); ++i)
        {
            instructions.push_back(TACInstruction(recv->variables[i], "recv_arg", recv->channelName, to_string(i)));
        }
    }
    else if (auto fdecl = dynamic_cast<FunctionDeclNode *>(stmt))
    {
        // Label for function start
        instructions.push_back(TACInstruction(fdecl->name, "label", ""));
        if (auto bodySeq = dynamic_cast<SeqNode *>(fdecl->body.get()))
        {
            for (auto &s : bodySeq->statements)
                generate_statement(s.get());
        }
        else
        {
            generate_statement(fdecl->body.get());
        }
        // implicit end label
        instructions.push_back(TACInstruction(fdecl->name + "_end", "label", ""));
    }
    else if (auto ret = dynamic_cast<ReturnNode *>(stmt))
    {
        string valTemp = ret->value ? generate_expression(ret->value.get()) : "";
        instructions.push_back(TACInstruction("ret", "return", valTemp));
    }
    // REMOVA a parte do IfNode por enquanto - vamos implementar depois
}

string TACGenerator::generate_expression(ASTNode *node)
{
    if (!node)
        return "error";

    if (auto num = dynamic_cast<NumberNode *>(node))
    {
        // Para números, criar temporário: t0 = 10
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, "=", to_string(num->value)));
        return temp;
    }
    else if (auto id = dynamic_cast<IdentifierNode *>(node))
    {
        return id->name;
    }
    else if (auto bin_op = dynamic_cast<BinaryOpNode *>(node))
    {
        string left = generate_expression(bin_op->left.get());
        string right = generate_expression(bin_op->right.get());
        string temp = new_temp();

        string op;
        switch (bin_op->op)
        {
        case TokenType::PLUS:
            op = "+";
            break;
        case TokenType::MINUS:
            op = "-";
            break;
        case TokenType::MULTIPLY:
            op = "*";
            break;
        case TokenType::DIVIDE:
            op = "/";
            break;
        case TokenType::AND:
            op = "&&";
            break;
        case TokenType::OR:
            op = "||";
            break;
        case TokenType::EQUAL:
            op = "==";
            break;
        case TokenType::NOT_EQUAL:
            op = "!=";
            break;
        case TokenType::LESS:
            op = "<";
            break;
        case TokenType::LESS_EQUAL:
            op = "<=";
            break;
        case TokenType::GREATER:
            op = ">";
            break;
        case TokenType::GREATER_EQUAL:
            op = ">=";
            break;
        default:
            op = "?";
            break;
        }

        instructions.push_back(TACInstruction(temp, op, left, right));
        return temp;
    }
    else if (auto un = dynamic_cast<UnaryOpNode *>(node))
    {
        string inner = generate_expression(un->operand.get());
        string temp = new_temp();
        // Represent unary not as temp = ! inner
        instructions.push_back(TACInstruction(temp, "!", inner));
        return temp;
    }
    else if (auto b = dynamic_cast<BooleanNode *>(node))
    {
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, "=", b->value ? "1" : "0"));
        return temp;
    }
    // REMOVA a parte do UnaryOpNode por enquanto

    return "error";
}

void TACGenerator::print_tac(std::ostream &out)
{
    for (const auto &instr : instructions)
    {
        if (instr.op == "print")
        {
            out << "print " << instr.arg1 << "\n";
        }
        else if (instr.op == "label")
        {
            out << instr.result << ":\n";
        }
        else if (instr.op == "if_false")
        {
            out << "if_false " << instr.arg1 << " goto " << instr.arg2 << "\n";
        }
        else if (instr.op == "goto")
        {
            out << "goto " << instr.arg1 << "\n";
        }
        else if (instr.op == "=")
        {
            out << instr.result << " = " << instr.arg1 << "\n";
        }
        else if (instr.op == "send")
        {
            out << "send " << instr.arg1 << " count=" << instr.arg2 << "\n";
        }
        else if (instr.op == "send_arg")
        {
            out << instr.result << "[" << instr.arg2 << "] <= " << instr.arg1 << "\n";
        }
        else if (instr.op == "receive")
        {
            out << "receive " << instr.arg1 << " count=" << instr.arg2 << "\n";
        }
        else if (instr.op == "recv_arg")
        {
            out << instr.result << " = recv " << instr.arg1 << "[" << instr.arg2 << "]\n";
        }
        else
        {
            // Operações binárias: t0 = x + y
            out << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << "\n";
        }
    }
}

void TACGenerator::print_tac()
{
    print_tac(std::cout);
}