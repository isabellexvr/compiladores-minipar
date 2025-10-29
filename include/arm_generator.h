#ifndef ARM_GENERATOR_H
#define ARM_GENERATOR_H

#include "tac_generator.h"
#include <vector>
#include <string>
#include <map>

#include <unordered_map>

class ARMGenerator {
private:
    std::vector<std::string> arm_code;
    std::map<std::string, std::string> var_registers;
    int register_counter;
    int label_counter;
    
    std::string get_register(const std::string& var);

public:
    ARMGenerator();
    std::vector<std::string> generate(const std::vector<TACInstruction>& tac);
    void print_arm(); // ← já existe
    void print_arm(std::ostream& out); // ← ADICIONE ESTA LINHA
};

#endif