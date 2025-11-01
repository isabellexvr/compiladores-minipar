#ifndef TAC_INTERPRETER_H
#define TAC_INTERPRETER_H

#include "tac_generator.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <ostream>

// Estrutura simples para simular canais: cada canal mantém fila de mensagens (vetor de vetores de ints)
struct ChannelRuntime
{
    std::vector<std::vector<int>> messages; // FIFO
};

class TACInterpreter
{
public:
    // Executa TAC, imprime efeitos (prints) no stream e retorna ambiente final de variáveis
    std::unordered_map<std::string, int> interpret(const std::vector<TACInstruction> &instrs, std::ostream &out);

private:
    std::unordered_map<std::string, int> env; // variáveis e temporários
    std::unordered_map<std::string, std::string> envStr; // valores string
    std::unordered_map<std::string, ChannelRuntime> channels;
    // estado para construção de mensagem em envio
    std::string buildingChannel;
    size_t expectedSendArgs = 0;
    std::vector<int> buildingMessage;
    // estado para recepção
    std::string receivingChannel;
    size_t expectedRecvArgs = 0;
    std::vector<int> receivedMessage;

    int valueOf(const std::string &token) const; // resolve nome ou literal
    std::string strValueOf(const std::string &token) const; // resolve string
    void finalizeSend();
    void finalizeReceive();
};

#endif
