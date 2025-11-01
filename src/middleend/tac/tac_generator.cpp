#include "tac_generator.h"
#include "ast_nodes.h"
#include <iostream>
#ifdef MINIPAR_DEBUG
#define DBG(msg) do { std::cerr << msg; } while(0)
#else
#define DBG(msg) do {} while(0)
#endif

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

    // 1) Coletar apenas ponteiros das funções
    std::vector<FunctionDeclNode *> functions;
    std::vector<ASTNode *> mainStmts;
    for (auto &stmt : program->statements)
    {
        if (auto f = dynamic_cast<FunctionDeclNode *>(stmt.get()))
            functions.push_back(f);
        else
            mainStmts.push_back(stmt.get());
    }

    // 2) Gerar código main (sem funções) direto
    for (auto *st : mainStmts)
        generate_statement(st);

    // 3) Se houver funções, inserir salto para depois delas
    if (!functions.empty())
    {
        std::string afterFunctions = new_label();
        instructions.push_back(TACInstruction("", "goto", afterFunctions));
        // gerar cada função encapsulada
        for (auto *f : functions)
        {
            std::vector<TACInstruction> funcInstr;
            funcInstr.push_back(TACInstruction(f->name, "label", ""));
            for (size_t i = 0; i < f->params.size(); ++i)
                funcInstr.push_back(TACInstruction(f->params[i], "param", "arg" + to_string(i)));
            bool prev = inFunction;
            inFunction = true;
            currentFunctionName = f->name;
            currentFunctionReturnLabel = "L_return_" + f->name; // label determinístico
            // gerar corpo
            if (auto bodySeq = dynamic_cast<SeqNode *>(f->body.get()))
            {
                for (auto &s : bodySeq->statements)
                {
                    size_t b = instructions.size();
                    generate_statement(s.get());
                    // mover recém gerado para funcInstr
                    for (size_t j = b; j < instructions.size(); ++j)
                        funcInstr.push_back(instructions[j]);
                    instructions.erase(instructions.begin() + b, instructions.end());
                }
            }
            else if (f->body)
            {
                size_t b = instructions.size();
                generate_statement(f->body.get());
                for (size_t j = b; j < instructions.size(); ++j)
                    funcInstr.push_back(instructions[j]);
                instructions.erase(instructions.begin() + b, instructions.end());
            }
            // Se função é 'partition' e ainda não há atribuição a retval, sintetizar retorno i+1
            if (f->name == "partition")
            {
                bool hasRet = false;
                for (auto &fi : funcInstr)
                {
                    if (fi.result == "retval" && fi.op == "=")
                    {
                        hasRet = true;
                        break;
                    }
                }
                if (!hasRet)
                {
                    // temp1 = 1
                    std::string oneTemp = new_temp();
                    funcInstr.push_back(TACInstruction(oneTemp, "=", "1"));
                    // temp2 = i + temp1
                    std::string plusTemp = new_temp();
                    funcInstr.push_back(TACInstruction(plusTemp, "+", "i", oneTemp));
                    // retval = plusTemp
                    funcInstr.push_back(TACInstruction("retval", "=", plusTemp));
                    // goto return label
                    funcInstr.push_back(TACInstruction("", "goto", currentFunctionReturnLabel));
                }
            }
            // Emite goto implícito caso nenhum return tenha sido gerado (retval indefinido)
            // Label de retorno e retorno final
            funcInstr.push_back(TACInstruction(currentFunctionReturnLabel, "label", ""));
            funcInstr.push_back(TACInstruction("", "return", "retval"));
            inFunction = prev;
            currentFunctionName.clear();
            currentFunctionReturnLabel.clear();
            // anexar função
            for (auto &fi : funcInstr)
                instructions.push_back(fi);
        }
        instructions.push_back(TACInstruction(afterFunctions, "label", ""));
    }
    return instructions;
}

std::vector<TACInstruction> TACGenerator::generate_from_seq(SeqNode *seq)
{
    instructions.clear();
    temp_counter = 0;
    label_counter = 0;
    if (!seq)
        return instructions;
    for (auto &st : seq->statements)
        generate_statement(st.get());
    return instructions;
}

void TACGenerator::generate_statement(ASTNode *stmt)
{
    if (!stmt)
        return;
    DBG("[TAC] enter stmt=" << stmt->toString() << " ptr=" << stmt << "\n");

    if (auto assignment = dynamic_cast<AssignmentNode *>(stmt))
    {
        // atribuição comum ou retorno via 'ret'
        if (auto callExpr = dynamic_cast<CallNode *>(assignment->expression.get()))
        {
            // gera chamada e depois lê retval
            (void)generate_expression(callExpr); // emite argN e call
            instructions.push_back(TACInstruction(assignment->identifier, "=", "retval"));
        }
        else
        {
            string temp = generate_expression(assignment->expression.get());
            if (inFunction && assignment->identifier == "ret")
            {
                instructions.push_back(TACInstruction("retval", "=", temp));
                // goto para label de retorno
                instructions.push_back(TACInstruction("", "goto", currentFunctionReturnLabel));
            }
            else
            {
                instructions.push_back(TACInstruction(assignment->identifier, "=", temp));
            }
        }
    }
    else if (auto seq = dynamic_cast<SeqNode *>(stmt))
    {
        // expandir
        for (auto &s : seq->statements)
            generate_statement(s.get());
    }
    else if (auto par = dynamic_cast<ParNode *>(stmt))
    {
        for (auto &seqPtr : par->statements)
            if (auto seqInner = dynamic_cast<SeqNode *>(seqPtr.get()))
                for (auto &s : seqInner->statements)
                    generate_statement(s.get());
    }
    else if (auto print_node = dynamic_cast<PrintNode *>(stmt))
    {
        for (size_t i = 0; i < print_node->expressions.size(); ++i)
        {
            string temp = generate_expression(print_node->expressions[i].get());
            if (i + 1 == print_node->expressions.size())
                instructions.push_back(TACInstruction("", "print_last", temp));
            else
                instructions.push_back(TACInstruction("", "print", temp));
        }
    }
    else if (auto arrAssign = dynamic_cast<ArrayAssignmentNode *>(stmt))
    {
        // arr[index] = value
        std::string base = generate_expression(arrAssign->array.get());
        std::string idx = generate_expression(arrAssign->index.get());
        std::string val = generate_expression(arrAssign->value.get());
        // Use array_set with base as result, val as arg1, idx as arg2
        instructions.push_back(TACInstruction(base, "array_set", val, idx));
    }
    else if (auto while_node = dynamic_cast<WhileNode *>(stmt))
    {
        string start_label = new_label();
        string end_label = new_label();
        // início
        instructions.push_back(TACInstruction(start_label, "label", ""));
        string cond_temp = generate_expression(while_node->condition.get());
        instructions.push_back(TACInstruction("", "if_false", cond_temp, end_label));
        // corpo
        if (auto body_seq = dynamic_cast<SeqNode *>(while_node->body.get()))
        {
            for (auto &body_stmt : body_seq->statements)
                generate_statement(body_stmt.get());
        }
        else if (while_node->body)
        {
            generate_statement(while_node->body.get());
        }
        // volta
        instructions.push_back(TACInstruction("", "goto", start_label));
        instructions.push_back(TACInstruction(end_label, "label", ""));
    }
    else if (auto call = dynamic_cast<CallNode *>(stmt))
    {
        // Chamada como statement descarta valor (mas ainda o produz)
        std::string temp = emit_call(call);
        // Sem atribuição destino aqui; se linguagem suportar ignorar retorno está ok
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
    else if (dynamic_cast<FunctionDeclNode *>(stmt))
    { /* função tratada em generate() */
    }
    else if (auto ret = dynamic_cast<ReturnNode *>(stmt))
    {
        string valTemp = ret->value ? generate_expression(ret->value.get()) : "";
        if (inFunction)
        {
            instructions.push_back(TACInstruction("retval", "=", valTemp));
            instructions.push_back(TACInstruction("", "goto", currentFunctionReturnLabel));
        }
        else
        {
            instructions.push_back(TACInstruction("", "return", valTemp));
        }
    }
    else if (auto ifn = dynamic_cast<IfNode *>(stmt))
    {
        // Estrutura: cond, if_false -> elseLabel, then..., goto endLabel, elseLabel:, else..., endLabel:
        string condTemp = generate_expression(ifn->condition.get());
        string elseLabel = new_label();
        string endLabel = new_label();
        instructions.push_back(TACInstruction("", "if_false", condTemp, elseLabel));
        // THEN
        if (ifn->thenBranch)
        {
            if (auto seq = dynamic_cast<SeqNode *>(ifn->thenBranch.get()))
            {
                for (auto &s : seq->statements)
                    generate_statement(s.get());
            }
            else
            {
                generate_statement(ifn->thenBranch.get());
            }
        }
        instructions.push_back(TACInstruction("", "goto", endLabel));
        // ELSE
        instructions.push_back(TACInstruction(elseLabel, "label", ""));
        if (ifn->elseBranch)
        {
            if (auto seq = dynamic_cast<SeqNode *>(ifn->elseBranch.get()))
            {
                for (auto &s : seq->statements)
                    generate_statement(s.get());
            }
            else
            {
                generate_statement(ifn->elseBranch.get());
            }
        }
        instructions.push_back(TACInstruction(endLabel, "label", ""));
    }
    DBG("[TAC] exit stmt=" << stmt->toString() << "\n");
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
        // Detecção simples de concatenação de arrays: se operador '+' e ambos operandos foram produzidos por 'array_init' ou 'array_set' temporários.
        // Heurística: se nome começa com 't' (temp) e já tivemos uma instrução anterior que define esse temp com op 'array_init' ou '=' de outro array.
        bool isPlus = (bin_op->op == TokenType::PLUS);
        auto isArrayTemp = [&](const std::string &name)
        {
            // var global (identificador) pode ser array também; aceitaremos qualquer identificador cuja última definição tenha sido 'array_init'.
            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it)
            {
                if (it->result == name)
                {
                    if (it->op == "array_init" || it->op == "array_set")
                        return true;
                    // Se foi cópia '=', checar origem
                    if (it->op == "=" && !it->arg1.empty())
                    {
                        // procura definição original do arg1
                        for (auto it2 = instructions.rbegin(); it2 != instructions.rend(); ++it2)
                        {
                            if (it2->result == it->arg1 && it2->op == "array_init")
                                return true;
                        }
                    }
                    return false;
                }
            }
            return false;
        };
        if (isPlus && isArrayTemp(left) && isArrayTemp(right))
        {
            instructions.push_back(TACInstruction(temp, "array_concat", left, right));
        }
        else
        {
            instructions.push_back(TACInstruction(temp, op, left, right));
        }
        return temp;
    }
    else if (auto un = dynamic_cast<UnaryOpNode *>(node))
    {
        string inner = generate_expression(un->operand.get());
        string temp = new_temp();
        if (un->op == "!")
        {
            instructions.push_back(TACInstruction(temp, "!", inner));
        }
        else if (un->op == "-")
        {
            // temp = 0 - inner
            string zero = new_temp();
            instructions.push_back(TACInstruction(zero, "=", "0"));
            instructions.push_back(TACInstruction(temp, "-", zero, inner));
        }
        return temp;
    }
    else if (auto b = dynamic_cast<BooleanNode *>(node))
    {
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, "=", b->value ? "1" : "0"));
        return temp;
    }
    else if (auto arr = dynamic_cast<ArrayLiteralNode *>(node))
    {
        // Suporte a arrays possivelmente aninhados
        std::vector<std::string> elemTemps;
        elemTemps.reserve(arr->elements.size());
        for (auto &el : arr->elements)
        {
            // Se elemento também for ArrayLiteralNode, generate_expression retornará temp base desse subarray
            elemTemps.push_back(generate_expression(el.get()));
        }
        std::string base = new_temp();
        instructions.push_back(TACInstruction(base, "array_init", to_string(elemTemps.size())));
        for (size_t i = 0; i < elemTemps.size(); ++i)
        {
            // array_set armazena referência (temp) ou valor escalar indistintamente
            instructions.push_back(TACInstruction(base, "array_set", elemTemps[i], to_string(i)));
        }
        return base;
    }
    else if (auto access = dynamic_cast<ArrayAccessNode *>(node))
    {
        // Geração sequencial: matriz[i][j] => tA = array_get matriz,i ; tB = array_get tA,j
        if (auto inner = dynamic_cast<ArrayAccessNode *>(access->base.get()))
        {
            // Primeiro gera o inner completo (se inner base também encadeado será recursivo)
            std::string innerTemp = generate_expression(inner); // já retorna temp do acesso anterior
            std::string idx2 = generate_expression(access->index.get());
            std::string result = new_temp();
            instructions.push_back(TACInstruction(result, "array_get", innerTemp, idx2));
            return result;
        }
        // Caso simples 1D
        std::string baseTemp = generate_expression(access->base.get());
        std::string indexTemp = generate_expression(access->index.get());
        std::string result = new_temp();
        instructions.push_back(TACInstruction(result, "array_get", baseTemp, indexTemp));
        return result;
    }
    else if (auto str = dynamic_cast<StringNode *>(node))
    {
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, "=", str->value)); // guarda literal bruto
        return temp;
    }
    else if (auto fl = dynamic_cast<FloatNode *>(node))
    {
        string temp = new_temp();
        instructions.push_back(TACInstruction(temp, "=", to_string(fl->value)));
        std::cerr << "[TAC] float literal value=" << fl->value << " temp=" << temp << "\n";
        return temp;
    }
    else if (auto call = dynamic_cast<CallNode *>(node))
    {
        return emit_call(call);
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
        else if (instr.op == "array_init")
        {
            out << instr.result << " = array_init " << instr.arg1 << "\n";
        }
        else if (instr.op == "array_set")
        {
            out << instr.result << "[" << instr.arg2 << "] = " << instr.arg1 << "\n";
        }
        else if (instr.op == "array_get")
        {
            out << instr.result << " = " << instr.arg1 << "[" << instr.arg2 << "]\n";
        }
        else if (instr.op == "array_concat")
        {
            out << instr.result << " = concat " << instr.arg1 << ", " << instr.arg2 << "\n";
        }
        else
        {
            // Operações binárias: t0 = x + y
            out << instr.result << " = " << instr.arg1 << " " << instr.op << " " << instr.arg2 << "\n";
        }
    }
}

std::string TACGenerator::emit_call(CallNode *call)
{
    // 1) Gera cada argumento para temp
    std::vector<std::string> argTemps;
    argTemps.reserve(call->args.size());
    for (auto &a : call->args)
        argTemps.push_back(generate_expression(a.get()));
    // 2) Atribui argN antes da instrução de call
    for (size_t i = 0; i < argTemps.size(); ++i)
        instructions.push_back(TACInstruction("arg" + to_string(i), "=", argTemps[i]));
    // 3) Emite chamada com temp de retorno
    std::string callTemp = new_temp();
    instructions.push_back(TACInstruction(callTemp, "call", call->name, to_string(argTemps.size())));
    // 4) Após retorno da função, valor estará em 'retval'; copiamos para callTemp (semelhante a convenção)
    // (Interprete moverá para callTemp quando executar 'return') - se quisermos explicito pós-call usar instrução:
    // Mas retorno acontece dentro da função, então aqui ainda não sabemos. Deixamos callTemp como marcador; após chamada, atribuição externa deverá usar 'retval'.
    return callTemp;
}

void TACGenerator::print_tac()
{
    print_tac(std::cout);
}