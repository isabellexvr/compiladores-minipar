#ifndef ARM_GENERATOR_H
#define ARM_GENERATOR_H

#include "tac_generator.h"
#include <vector>
#include <string>
#include <unordered_map>

class ARMGenerator {
private:
    std::vector<std::string> arm_code;
    int register_counter;
    int label_counter;  // ADICIONAR ESTA LINHA
    std::unordered_map<std::string, std::string> var_registers;
    
    std::string get_register(const std::string& var);
    std::string allocate_register();
    
public:
    ARMGenerator();
    std::vector<std::string> generate(const std::vector<TACInstruction>& tac);
    void print_arm();
};

#endif