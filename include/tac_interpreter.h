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
    std::unordered_map<std::string, int> env;            // variáveis e temporários
    std::unordered_map<std::string, std::string> envStr; // valores string
    std::unordered_map<std::string, double> envF;        // valores float
    std::unordered_map<std::string, ChannelRuntime> channels;
    std::unordered_map<std::string, std::vector<std::string>> funcParams; // nome -> lista params
    struct CallFrame
    {
        size_t return_ip;
        std::string return_target;
        bool has_target;
    };
    std::vector<CallFrame> callStack;
    // estado para construção de mensagem em envio
    std::string buildingChannel;
    size_t expectedSendArgs = 0;
    std::vector<int> buildingMessage;
    // estado para recepção
    std::string receivingChannel;
    size_t expectedRecvArgs = 0;
    std::vector<int> receivedMessage;
    // armazenamento simples de arrays: nome -> vetor de double (suporta int/float)
    std::unordered_map<std::string, std::vector<double>> arrays;
    // Para arrays aninhados: guarda referências a subarrays (nome do temp de subarray ou vazio se elemento escalar)
    std::unordered_map<std::string, std::vector<std::string>> arraysNested;
    // Alternativa segura para matrizes: armazenamento direto 2D (cada linha é um vetor de double)
    std::unordered_map<std::string, std::vector<std::vector<double>>> matrices;
    // Armazenamento temporário de linhas antes de atribuição final
    std::unordered_map<std::string, std::vector<std::vector<double>>> flattenedRows;
    // Número de colunas por array matricial flatten
    std::unordered_map<std::string, int> arrayCols;

    double valueOf(const std::string &token) const;         // resolve nome ou literal
    std::string strValueOf(const std::string &token) const; // resolve string
    void finalizeSend();
    void finalizeReceive();
};

#endif
