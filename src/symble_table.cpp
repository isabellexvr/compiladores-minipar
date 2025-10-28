#include "symbol_table.h"

using namespace std;

bool SymbolTable::add_symbol(const string& name, SymbolType type, const string& data_type) {
    if (symbol_exists(name)) {
        return false; // Símbolo já existe
    }
    
    symbols.emplace(name, Symbol(name, type, data_type));
    return true;
}

Symbol* SymbolTable::get_symbol(const string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SymbolTable::symbol_exists(const string& name) {
    return symbols.find(name) != symbols.end();
}