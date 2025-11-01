#include "tac_interpreter.h"
#include <cstdlib>
#include <iostream>

double TACInterpreter::valueOf(const std::string &token) const
{
    if (token.empty())
        return 0.0;

    // Prioriza variáveis locais (pode ser int ou float)
    auto it_f = envF.find(token);
    if (it_f != envF.end())
        return it_f->second;

    auto it_i = env.find(token);
    if (it_i != env.end())
        return (double)it_i->second;

    // Se não for variável, tenta converter para número literal
    char *end_i = nullptr;
    long v_i = std::strtol(token.c_str(), &end_i, 10);

    char *end_f = nullptr;
    double v_f = std::strtod(token.c_str(), &end_f);

    // Se o token inteiro foi consumido na conversão para float, é um float.
    if (*end_f == '\0')
    {
        return v_f;
    }
    // Senão, se foi consumido na conversão para int, é um int.
    if (*end_i == '\0')
    {
        return (double)v_i;
    }

    return 0.0; // Default se não for nem var nem literal numérico
}

void TACInterpreter::finalizeSend()
{
    if (!buildingChannel.empty() && buildingMessage.size() == expectedSendArgs)
    {
        channels[buildingChannel].messages.push_back(buildingMessage);
    }
    buildingChannel.clear();
    expectedSendArgs = 0;
    buildingMessage.clear();
}

void TACInterpreter::finalizeReceive()
{
    // após mapear todas as variáveis, podemos executar lógica de cálculo se presente
    if (receivingChannel.empty() || receivedMessage.size() != expectedRecvArgs)
    {
        receivingChannel.clear();
        expectedRecvArgs = 0;
        receivedMessage.clear();
        return;
    }
    // Heurística: se temos operacao, valor1, valor2, resultado -> calcula
    if ((env.count("valor1") && env.count("valor2") && env.count("resultado")))
    {
        int a = env["valor1"], b = env["valor2"], res = env["resultado"];
        // Primeiro tenta operação numérica codificada
        if (env.count("operacao"))
        {
            int op = env["operacao"]; // 1=+,2=-,3=*,4=/
            switch (op)
            {
            case 1:
                res = a + b;
                break;
            case 2:
                res = a - b;
                break;
            case 3:
                res = a * b;
                break;
            case 4:
                res = (b != 0 ? a / b : 0);
                break;
            }
        }
        // Se existe operacao string, sobrescreve
        if (envStr.find("operacao") != envStr.end())
        {
            const std::string &opStr = envStr["operacao"];
            if (opStr == "+")
                res = a + b;
            else if (opStr == "-")
                res = a - b;
            else if (opStr == "*")
                res = a * b;
            else if (opStr == "/")
                res = (b != 0 ? a / b : 0);
        }
        env["resultado"] = res;
    }
    receivingChannel.clear();
    expectedRecvArgs = 0;
    receivedMessage.clear();
}

std::unordered_map<std::string, int> TACInterpreter::interpret(const std::vector<TACInstruction> &instrs, std::ostream &out)
{
    env.clear();
    envStr.clear();
    channels.clear();
    buildingChannel.clear();
    receivingChannel.clear();
    expectedSendArgs = 0;
    expectedRecvArgs = 0;
    buildingMessage.clear();
    receivedMessage.clear();

    // Mapa de labels para índices
    std::unordered_map<std::string, size_t> labelMap;
    for (size_t i = 0; i < instrs.size(); ++i)
    {
        if (instrs[i].op == "label")
            labelMap[instrs[i].result] = i;
    }
    // Pré-pass: coletar params por função (param instruções seguem label)
    for (size_t i = 0; i < instrs.size(); ++i)
    {
        if (instrs[i].op == "label")
        {
            std::string fname = instrs[i].result;
            size_t j = i + 1;
            while (j < instrs.size() && instrs[j].op == "param")
            {
                funcParams[fname].push_back(instrs[j].result); // nome do param
                ++j;
            }
        }
    }

    // Loop manual com ip para permitir saltos
    size_t ip = 0;
    while (ip < instrs.size())
    {
        const auto &ins = instrs[ip];

        // Adicionando log de depuração para cada instrução
        std::cerr << "DEBUG [ip=" << ip << "]: "
                  << ins.result << " = (" << ins.op << ") "
                  << ins.arg1 << (ins.arg2.empty() ? "" : ", " + ins.arg2)
                  << std::endl;

        size_t next_ip = ip + 1; // próximo padrão
        if (ins.op == "=")
        {
            // tentar número; se não for número puro, tratar como string
            char *end = nullptr;
            long v = strtol(ins.arg1.c_str(), &end, 10);
            if (*end == '\0')
            {
                env[ins.result] = (int)v;
            }
            else
            {
                // tenta float
                char *endf = nullptr;
                double vf = strtod(ins.arg1.c_str(), &endf);
                if (*endf == '\0')
                {
                    envF[ins.result] = vf;
                }
                else
                {
                    // pode ser temp que já está em env ou envStr
                    if (env.find(ins.arg1) != env.end())
                    {
                        env[ins.result] = env[ins.arg1];
                    }
                    else if (envF.find(ins.arg1) != envF.end())
                    {
                        envF[ins.result] = envF[ins.arg1];
                    }
                    else if (envStr.find(ins.arg1) != envStr.end())
                    {
                        envStr[ins.result] = envStr[ins.arg1];
                    }
                    else
                    {
                        // literal string
                        envStr[ins.result] = ins.arg1;
                    }
                }
            }
            // copia de array: se RHS for um identificador de array
            if (arrays.find(ins.arg1) != arrays.end())
            {
                arrays[ins.result] = arrays[ins.arg1];
            }
        }
        else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/" ||
                 ins.op == "==" || ins.op == "!=" || ins.op == "<" || ins.op == "<=" ||
                 ins.op == ">" || ins.op == ">=" || ins.op == "&&" || ins.op == "||" || ins.op == "!")
        {
            // Verifica se algum dos operandos é float (seja como variável ou como literal)
            bool isFloat1 = envF.count(ins.arg1) || (ins.arg1.find('.') != std::string::npos);
            bool isFloat2 = envF.count(ins.arg2) || (ins.arg2.find('.') != std::string::npos);
            bool floatOp = isFloat1 || isFloat2;

            if (floatOp && (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/"))
            {
                double left = valueOf(ins.arg1);
                double right = valueOf(ins.arg2);
                double val = 0.0;
                if (ins.op == "+")
                    val = left + right;
                else if (ins.op == "-")
                    val = left - right;
                else if (ins.op == "*")
                    val = left * right;
                else if (ins.op == "/")
                    val = (right != 0.0 ? left / right : 0.0);
                envF[ins.result] = val;
            }
            else
            {
                int left = (int)valueOf(ins.arg1);
                int right = (ins.op == "!") ? 0 : (int)valueOf(ins.arg2);
                int val = 0;
                if (ins.op == "+")
                    val = left + right;
                else if (ins.op == "-")
                    val = left - right;
                else if (ins.op == "*")
                    val = left * right;
                else if (ins.op == "/")
                    val = (right != 0 ? left / right : 0);
                else if (ins.op == "==")
                    val = (left == right);
                else if (ins.op == "!=")
                    val = (left != right);
                else if (ins.op == "<")
                    val = (left < right);
                else if (ins.op == "<=")
                    val = (left <= right);
                else if (ins.op == ">")
                    val = (left > right);
                else if (ins.op == ">=")
                    val = (left >= right);
                else if (ins.op == "&&")
                    val = (left && right);
                else if (ins.op == "||")
                    val = (left || right);
                else if (ins.op == "!")
                    val = (!left);
                env[ins.result] = val;
            }
        }
        else if (ins.op == "print" || ins.op == "print_last")
        {
            if (envStr.find(ins.arg1) != envStr.end())
                out << envStr[ins.arg1];
            else if (envF.find(ins.arg1) != envF.end())
                out << envF[ins.arg1];
            else
                out << env[ins.arg1];
            if (ins.op == "print_last")
                out << "\n";
            else
                out << " ";
        }
        else if (ins.op == "label")
        {
            // nada a fazer
            // Se este label é o destino do salto inicial e estamos retornando de função sem callStack, podemos encerrar
            if (callStack.empty() && ins.result.size() && ins.result[0] == 'L' && ip + 1 == instrs.size())
            {
                break;
            }
        }
        else if (ins.op == "if_false")
        {
            double cond = valueOf(ins.arg1);
            if (cond == 0.0)
            {
                auto it = labelMap.find(ins.arg2);
                if (it != labelMap.end())
                    next_ip = it->second + 1;
            }
        }
        else if (ins.op == "goto")
        {
            // arg1 contém label destino
            auto it = labelMap.find(ins.arg1);
            if (it != labelMap.end())
                next_ip = it->second + 1;
        }
        else if (ins.op == "param")
        {
            // param X = argY; argY may be in env or envF; if missing default 0
            if (env.count(ins.arg1))
                env[ins.result] = env[ins.arg1];
            else if (envF.count(ins.arg1))
                envF[ins.result] = envF[ins.arg1];
            else if (envStr.count(ins.arg1))
                envStr[ins.result] = envStr[ins.arg1];
            else
            {
                env[ins.result] = 0;
            }
        }
        else if (ins.op == "call")
        {
            // ins.arg1 = function name, ins.arg2 = arg count, result = temp for return
            auto it = labelMap.find(ins.arg1);
            if (it != labelMap.end())
            {
                callStack.push_back({next_ip, ins.result, true});
                next_ip = it->second + 1; // after label, params will be processed
            }
        }
        else if (ins.op == "return")
        {
            // ins.arg1 holds temp return value (already a temp or literal)
            if (!callStack.empty())
            {
                auto frame = callStack.back();
                callStack.pop_back();
                // move return value into target temp
                if (frame.has_target)
                {
                    if (envF.count(ins.arg1))
                        envF[frame.return_target] = envF[ins.arg1];
                    else if (env.count(ins.arg1))
                        env[frame.return_target] = env[ins.arg1];
                    else if (envStr.count(ins.arg1))
                        envStr[frame.return_target] = envStr[ins.arg1];
                    else
                    {
                        // literal number fallback
                        char *e = nullptr;
                        long vi = strtol(ins.arg1.c_str(), &e, 10);
                        if (*e == '\0')
                            env[frame.return_target] = (int)vi;
                        else
                        {
                            char *ef = nullptr;
                            double vf = strtod(ins.arg1.c_str(), &ef);
                            if (*ef == '\0')
                                envF[frame.return_target] = vf;
                            else
                                envStr[frame.return_target] = ins.arg1;
                        }
                    }
                }
                next_ip = frame.return_ip;
            }
        }
        else if (ins.op == "send")
        {
            finalizeSend();
            buildingChannel = ins.arg1; // canal
            expectedSendArgs = (size_t)valueOf(ins.arg2);
            buildingMessage.clear();
        }
        else if (ins.op == "send_arg")
        {
            if (buildingChannel == ins.result)
            {
                buildingMessage.push_back(valueOf(ins.arg1));
                if (buildingMessage.size() == expectedSendArgs)
                    finalizeSend();
            }
        }
        else if (ins.op == "receive")
        {
            finalizeReceive();
            receivingChannel = ins.arg1;
            expectedRecvArgs = (size_t)valueOf(ins.arg2);
            receivedMessage.clear();
            // pop mensagem do canal (se existir)
            auto &queue = channels[receivingChannel].messages;
            if (!queue.empty())
            {
                receivedMessage = queue.front();
                queue.erase(queue.begin());
            }
        }
        else if (ins.op == "recv_arg")
        {
            if (receivedMessage.size() == expectedRecvArgs && receivingChannel == ins.arg1)
            {
                size_t pos = (size_t)valueOf(ins.arg2);
                int v = (pos < receivedMessage.size() ? receivedMessage[pos] : 0);
                env[ins.result] = v;
                // se chegou na última variável, finalize para cálculo
                // se pos == expectedRecvArgs-1, última variável
                if (pos == expectedRecvArgs - 1)
                    finalizeReceive();
            }
        }
        else if (ins.op == "array_init")
        {
            size_t sz = (size_t)valueOf(ins.arg1);
            arrays[ins.result] = std::vector<double>(sz, 0.0);
        }
        else if (ins.op == "array_set")
        {
            // result[arg2] = arg1
            auto it = arrays.find(ins.result);
            if (it != arrays.end())
            {
                size_t idx = (size_t)valueOf(ins.arg2);
                double val = valueOf(ins.arg1);
                if (idx < it->second.size())
                    it->second[idx] = val;
            }
        }
        else if (ins.op == "array_get")
        {
            auto it = arrays.find(ins.arg1);
            if (it != arrays.end())
            {
                size_t idx = (size_t)valueOf(ins.arg2);
                double val = (idx < it->second.size() ? it->second[idx] : 0.0);
                // store into envF if fractional part present else env
                if (val != (int)val)
                    envF[ins.result] = val;
                else
                    env[ins.result] = (int)val;
            }
            else
            {
                env[ins.result] = 0; // fallback
            }
        }
        ip = next_ip;
    }
    finalizeSend();
    finalizeReceive();
    return env;
}
