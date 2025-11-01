#include "tac_interpreter.h"
#include <cstdlib>

int TACInterpreter::valueOf(const std::string &token) const
{
    if (token.empty())
        return 0;
    auto it = env.find(token);
    if (it != env.end())
        return it->second;
    // tenta número literal
    char *end = nullptr;
    long v = std::strtol(token.c_str(), &end, 10);
    if (*end == '\0')
        return (int)v;
    return 0;
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
    if (env.count("operacao") && env.count("valor1") && env.count("valor2") && env.count("resultado"))
    {
        int op = env["operacao"];
        int a = env["valor1"];
        int b = env["valor2"];
        int res = env["resultado"];
        switch (op)
        { // 1=+,2=-,3=*,4=/
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
        env["resultado"] = res;
    }
    receivingChannel.clear();
    expectedRecvArgs = 0;
    receivedMessage.clear();
}

std::unordered_map<std::string, int> TACInterpreter::interpret(const std::vector<TACInstruction> &instrs, std::ostream &out)
{
    env.clear();
    channels.clear();
    buildingChannel.clear();
    receivingChannel.clear();
    expectedSendArgs = 0;
    expectedRecvArgs = 0;
    buildingMessage.clear();
    receivedMessage.clear();

    for (size_t idx = 0; idx < instrs.size(); ++idx)
    {
        const auto &ins = instrs[idx];
        if (ins.op == "=")
        {
            env[ins.result] = valueOf(ins.arg1);
        }
        else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/" ||
                 ins.op == "==" || ins.op == "!=" || ins.op == "<" || ins.op == "<=" ||
                 ins.op == ">" || ins.op == ">=" || ins.op == "&&" || ins.op == "||" || ins.op == "!")
        {
            int left = valueOf(ins.arg1);
            int right = ins.op == "!" ? 0 : valueOf(ins.arg2);
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
        else if (ins.op == "print")
        {
            out << env[valueOf(ins.arg1) ? ins.arg1 : ins.arg1] << "\n"; // simple
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
                size_t countAssigned = 0;
                // naive: contamos quantas recv_arg para canal atual já ocorreram
                // melhor: se pos == expectedRecvArgs-1
                if (pos == expectedRecvArgs - 1)
                    finalizeReceive();
            }
        }
    }
    finalizeSend();
    finalizeReceive();
    return env;
}
