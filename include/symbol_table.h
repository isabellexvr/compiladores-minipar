#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration to avoid circular include; AST nodes defined elsewhere
struct ASTNode;
struct ProgramNode;

enum class SymbolType
{
    VARIABLE,
    FUNCTION,
    CHANNEL
};

struct Symbol
{
    std::string name;
    SymbolType type;
    std::string data_type;   // "int", "bool", "string", etc.
    bool has_return = false; // marca se função possui retorno
    std::string return_type; // tipo inferido do retorno

    Symbol(const std::string &n, SymbolType t, const std::string &dt = "")
        : name(n), type(t), data_type(dt) {}
};

class SymbolTable
{
private:
    std::unordered_map<std::string, Symbol> symbols;

public:
    bool add_symbol(const std::string &name, SymbolType type, const std::string &data_type = "");
    Symbol *get_symbol(const std::string &name);
    bool symbol_exists(const std::string &name);
    std::vector<Symbol> get_all_symbols() const; // iteration helper
};

// Build a symbol table walking the AST. Very simple heuristics for now.
void build_symbol_table(ProgramNode *program, SymbolTable &table);

// Print symbol table to an output stream.
void print_symbol_table(const SymbolTable &table, std::ostream &out);

#endif