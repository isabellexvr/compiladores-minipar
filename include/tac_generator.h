#ifndef TAC_GENERATOR_H
#define TAC_GENERATOR_H

#include "parser.h"
#include <vector>
#include <string>

struct TACInstruction {
    std::string result;
    std::string arg1;
    std::string op;
    std::string arg2;
    
    TACInstruction(const std::string& res, const std::string& a1, 
                   const std::string& o = "", const std::string& a2 = "")
        : result(res), arg1(a1), op(o), arg2(a2) {}
};

class TACGenerator {
private:
    std::vector<TACInstruction> instructions;
    int temp_counter;
    
    std::string new_temp();
    
    std::string generate_expression(ASTNode* node);
    
public:
    TACGenerator();
    std::vector<TACInstruction> generate(ProgramNode* program);
    void print_tac();
};

#endif