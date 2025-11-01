#ifndef TAC_GENERATOR_H
#define TAC_GENERATOR_H

#include "parser.h"
#include "ast_nodes.h"
#include <vector>
#include <string>

struct TACInstruction
{
    std::string result;
    std::string op;
    std::string arg1;
    std::string arg2;

    TACInstruction(const std::string &res, const std::string &operation,
                   const std::string &a1 = "", const std::string &a2 = "")
        : result(res), op(operation), arg1(a1), arg2(a2) {}
};

class TACGenerator
{
private:
    std::vector<TACInstruction> instructions;
    int temp_counter;
    int label_counter;

    std::string new_temp();
    std::string new_label(); // ← ADICIONE ESTA LINHA
    void generate_statement(ASTNode *stmt);
    std::string generate_expression(ASTNode *node);

public:
    TACGenerator();
    std::vector<TACInstruction> generate(ProgramNode *program);
    void print_tac();
    void print_tac(std::ostream &out);
    std::vector<TACInstruction> generate_from_seq(SeqNode *seq); // novo
};

#endif