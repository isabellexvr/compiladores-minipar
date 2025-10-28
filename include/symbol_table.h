#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>

enum class SymbolType {
    VARIABLE,
    FUNCTION,
    CHANNEL
};

struct Symbol {
    std::string name;
    SymbolType type;
    std::string data_type; // "int", "bool", "string", etc.
    
    Symbol(const std::string& n, SymbolType t, const std::string& dt = "")
        : name(n), type(t), data_type(dt) {}
};

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> symbols;
    
public:
    bool add_symbol(const std::string& name, SymbolType type, const std::string& data_type = "");
    Symbol* get_symbol(const std::string& name);
    bool symbol_exists(const std::string& name);
};

#endif