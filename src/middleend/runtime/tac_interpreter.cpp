#include "tac_interpreter.h"
#include <cstdlib>
#include <iostream>
#ifdef MINIPAR_DEBUG
#define DBG(msg) do { std::cerr << msg; } while(0)
#else
#define DBG(msg) do {} while(0)
#endif

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
    arrays.clear();
    arraysStr.clear();
    arraysNested.clear();
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
    DBG("DEBUG [ip=" << ip << "]: "
         << ins.result << " = (" << ins.op << ") "
         << ins.arg1 << (ins.arg2.empty() ? "" : ", " + ins.arg2)
         << "\n");

        size_t next_ip = ip + 1; // próximo padrão
        if (ins.op == "=")
        {
            // Se RHS já é variável string, copia direto
            if (envStr.count(ins.arg1))
            {
                envStr[ins.result] = envStr[ins.arg1];
            }
            else if (envF.count(ins.arg1))
            {
                envF[ins.result] = envF[ins.arg1];
            }
            else if (env.count(ins.arg1))
            {
                env[ins.result] = env[ins.arg1];
            }
            else
            {
                // tentar número literal; se não for, tratar como string literal
                char *end = nullptr;
                long v = strtol(ins.arg1.c_str(), &end, 10);
                if (*end == '\0')
                {
                    env[ins.result] = (int)v;
                }
                else
                {
                    char *endf = nullptr;
                    double vf = strtod(ins.arg1.c_str(), &endf);
                    if (*endf == '\0')
                    {
                        envF[ins.result] = vf;
                    }
                    else
                    {
                        envStr[ins.result] = ins.arg1; // literal string crua
                    }
                }
            }
            // copia de array: se RHS for um identificador de array
            if (arrays.find(ins.arg1) != arrays.end())
            {
                arrays[ins.result] = arrays[ins.arg1];
                if (arraysNested.count(ins.arg1))
                    arraysNested[ins.result] = arraysNested[ins.arg1];
                if (arraysStr.count(ins.arg1))
                    arraysStr[ins.result] = arraysStr[ins.arg1];
            }
        }
        else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/" ||
                 ins.op == "==" || ins.op == "!=" || ins.op == "<" || ins.op == "<=" ||
                 ins.op == ">" || ins.op == ">=" || ins.op == "&&" || ins.op == "||" || ins.op == "!")
        {
            // Concatenacao de arrays usando '+' se ambos operandos forem arrays
            if (ins.op == "+" && arrays.find(ins.arg1) != arrays.end() && arrays.find(ins.arg2) != arrays.end())
            {
                const auto &leftArr = arrays[ins.arg1];
                const auto &rightArr = arrays[ins.arg2];
                std::vector<double> newArr;
                newArr.reserve(leftArr.size() + rightArr.size());
                newArr.insert(newArr.end(), leftArr.begin(), leftArr.end());
                newArr.insert(newArr.end(), rightArr.begin(), rightArr.end());
                arrays[ins.result] = std::move(newArr);
                // Concatenar strings paralelas
                std::vector<std::string> newStrs;
                const auto &leftStrs = arraysStr[ins.arg1];
                const auto &rightStrs = arraysStr[ins.arg2];
                newStrs.reserve(leftStrs.size() + rightStrs.size());
                newStrs.insert(newStrs.end(), leftStrs.begin(), leftStrs.end());
                newStrs.insert(newStrs.end(), rightStrs.begin(), rightStrs.end());
                arraysStr[ins.result] = std::move(newStrs);
                // Concatenar referencias nested
                if (arraysNested.count(ins.arg1) || arraysNested.count(ins.arg2))
                {
                    std::vector<std::string> newNested;
                    auto leftN = arraysNested.count(ins.arg1) ? arraysNested[ins.arg1] : std::vector<std::string>(leftArr.size(), "");
                    auto rightN = arraysNested.count(ins.arg2) ? arraysNested[ins.arg2] : std::vector<std::string>(rightArr.size(), "");
                    newNested.reserve(leftN.size() + rightN.size());
                    newNested.insert(newNested.end(), leftN.begin(), leftN.end());
                    newNested.insert(newNested.end(), rightN.begin(), rightN.end());
                    arraysNested[ins.result] = std::move(newNested);
                }
                // Limpa env numérico para evitar impressão incorreta
                env.erase(ins.result);
                envF.erase(ins.result);
                // já tratou concatenação; seguir para próxima instrução
                next_ip = ip + 1;
                ip = next_ip - 1;
                // Avança sem executar demais blocos
                ip = next_ip - 1;
            }
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
            // Nova prioridade: arrays primeiro (para evitar imprimir temp de referência), depois string, float, int, literal fallback.
            if (arrays.find(ins.arg1) != arrays.end())
            {
                bool hasString = arraysStr.count(ins.arg1) && !arraysStr[ins.arg1].empty();
                size_t sz = arrays[ins.arg1].size();
                out << "[";
                for (size_t k = 0; k < sz; ++k)
                {
                    if (hasString && k < arraysStr[ins.arg1].size() && !arraysStr[ins.arg1][k].empty())
                    {
                        out << arraysStr[ins.arg1][k];
                    }
                    else if (arraysNested.count(ins.arg1) && k < arraysNested[ins.arg1].size() && !arraysNested[ins.arg1][k].empty())
                    {
                        // Representar subarray por nome entre '<>' para depuração
                        out << "<" << arraysNested[ins.arg1][k] << ">";
                    }
                    else
                    {
                        double v = arrays[ins.arg1][k];
                        if (v == (int)v)
                            out << (int)v;
                        else
                            out << v;
                    }
                    if (k + 1 < sz)
                        out << ", ";
                }
                out << "]";
            }
            else if (envStr.find(ins.arg1) != envStr.end())
            {
                out << envStr[ins.arg1];
            }
            else if (envF.find(ins.arg1) != envF.end())
            {
                out << envF[ins.arg1];
            }
            else if (env.find(ins.arg1) != env.end())
            {
                out << env[ins.arg1];
            }
            else
            {
                out << ins.arg1;
            }
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
        else if (ins.op == "array_concat")
        {
            // Concatenação explícita emitida pelo gerador
            if (arrays.count(ins.arg1) && arrays.count(ins.arg2))
            {
                const auto &A = arrays[ins.arg1];
                const auto &B = arrays[ins.arg2];
                std::vector<double> merged;
                merged.reserve(A.size() + B.size());
                merged.insert(merged.end(), A.begin(), A.end());
                merged.insert(merged.end(), B.begin(), B.end());
                arrays[ins.result] = std::move(merged);
                // Strings
                std::vector<std::string> mergedStr;
                const auto &AS = arraysStr[ins.arg1];
                const auto &BS = arraysStr[ins.arg2];
                mergedStr.reserve(AS.size() + BS.size());
                mergedStr.insert(mergedStr.end(), AS.begin(), AS.end());
                mergedStr.insert(mergedStr.end(), BS.begin(), BS.end());
                arraysStr[ins.result] = std::move(mergedStr);
                // Nested references
                if (arraysNested.count(ins.arg1) || arraysNested.count(ins.arg2))
                {
                    std::vector<std::string> mergedN;
                    auto AN = arraysNested.count(ins.arg1) ? arraysNested[ins.arg1] : std::vector<std::string>(A.size(), "");
                    auto BN = arraysNested.count(ins.arg2) ? arraysNested[ins.arg2] : std::vector<std::string>(B.size(), "");
                    mergedN.reserve(AN.size() + BN.size());
                    mergedN.insert(mergedN.end(), AN.begin(), AN.end());
                    mergedN.insert(mergedN.end(), BN.begin(), BN.end());
                    arraysNested[ins.result] = std::move(mergedN);
                }
                // Limpa env numérico para não confundir com escalar
                env.erase(ins.result);
                envF.erase(ins.result);
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
            // Inicializa estrutura nested apenas; manter vazio para evitar acessos inválidos se não usado
            arraysNested[ins.result] = std::vector<std::string>(sz, "");
            arraysStr[ins.result] = std::vector<std::string>(sz, "");
        }
        else if (ins.op == "array_set")
        {
            // result[arg2] = arg1
            auto it = arrays.find(ins.result);
            if (it != arrays.end())
            {
                size_t idx = (size_t)valueOf(ins.arg2);
                bool rhsIsArray = arrays.find(ins.arg1) != arrays.end();
                if (idx < it->second.size())
                {
                    if (rhsIsArray)
                    {
                        // Apenas registra referência, não sobrescreve vetor numérico (mantém valor dummy)
                        auto &nestVec = arraysNested[ins.result];
                        if (nestVec.empty())
                            nestVec = std::vector<std::string>(it->second.size(), "");
                        nestVec[idx] = ins.arg1;
                        // strings aninhadas são tratadas copiando vetor de strings correspondente
                        if (arraysStr.count(ins.arg1))
                        {
                            if (arraysStr[ins.result].size() < it->second.size())
                                arraysStr[ins.result].resize(it->second.size());
                            // Não é coleção de strings diretas, mas manter vazio.
                        }
                    }
                    else
                    {
                        // Decide se é string ou numérico (heurística semelhante à atribuição '=')
                        // 1. Se já é variável string
                        if (envStr.count(ins.arg1))
                        {
                            arraysStr[ins.result][idx] = envStr[ins.arg1];
                        }
                        // 2. Se é variável float
                        else if (envF.count(ins.arg1))
                        {
                            it->second[idx] = envF[ins.arg1];
                        }
                        // 3. Se é variável int
                        else if (env.count(ins.arg1))
                        {
                            it->second[idx] = (double)env[ins.arg1];
                        }
                        else
                        {
                            // 4. Tenta literal numérico (int ou float). Se não for, trata como string literal.
                            char *endInt = nullptr;
                            long vi = strtol(ins.arg1.c_str(), &endInt, 10);
                            char *endF = nullptr;
                            double vf = strtod(ins.arg1.c_str(), &endF);
                            if (*endF == '\0')
                            {
                                it->second[idx] = vf; // literal float ou int consumido como float
                            }
                            else if (*endInt == '\0')
                            {
                                it->second[idx] = (double)vi; // literal int
                            }
                            else
                            {
                                // 5. String literal crua (ex: Joao, Smartphone, etc.)
                                arraysStr[ins.result][idx] = ins.arg1;
                            }
                        }
                    }
                }
            }
        }
        else if (ins.op == "array_get")
        {
            // Suporte a acesso sequencial: se elemento for subarray referenciado em arraysNested, copia subarray
            auto it = arrays.find(ins.arg1);
            if (it == arrays.end())
            {
                env[ins.result] = 0;
                envF[ins.result] = 0.0;
            }
            else
            {
                size_t idx = (size_t)valueOf(ins.arg2);
                double val = 0.0;
                bool handledSubarray = false;
                // Verifica referência aninhada
                auto nestedIt = arraysNested.find(ins.arg1);
                if (nestedIt != arraysNested.end() && idx < nestedIt->second.size())
                {
                    const std::string &subName = nestedIt->second[idx];
                    if (!subName.empty() && arrays.count(subName))
                    {
                        // Copia conteúdo da linha/subarray para novo temp
                        arrays[ins.result] = arrays[subName];
                        // Copia estrutura nested adicional (permite 3D futuramente)
                        if (arraysNested.count(subName))
                            arraysNested[ins.result] = arraysNested[subName];
                        if (arraysStr.count(subName))
                            arraysStr[ins.result] = arraysStr[subName];
                        handledSubarray = true;
                        env[ins.result] = 0;
                        envF[ins.result] = 0.0;
                    }
                }
                if (!handledSubarray)
                {
                    if (idx < it->second.size())
                        val = it->second[idx];
                    // Se houver string nesse índice, prioriza string
                    if (arraysStr.count(ins.arg1) && idx < arraysStr[ins.arg1].size() && !arraysStr[ins.arg1][idx].empty())
                    {
                        envStr[ins.result] = arraysStr[ins.arg1][idx];
                        // não define env/int placeholder para evitar sobrescrever string em atribuições futuras
                        envF.erase(ins.result);
                        env.erase(ins.result);
                    }
                    else
                    {
                        // Preserva tanto int quanto float; se valor tem parte fracionária manter envF.
                        envF[ins.result] = val;
                        env[ins.result] = (int)val; // ainda armazena inteiro para operações booleanas
                    }
                }
            }
        }
        ip = next_ip;
    }
    finalizeSend();
    finalizeReceive();
    return env;
}
