#include "symbol_table.h"
#include "ast_nodes.h"
#include <iostream>
using namespace std;

bool SymbolTable::add_symbol(const string &name, SymbolType type, const string &data_type)
{
    if (symbol_exists(name))
        return false;
    symbols.emplace(name, Symbol(name, type, data_type));
    return true;
}

Symbol *SymbolTable::get_symbol(const string &name)
{
    auto it = symbols.find(name);
    return (it != symbols.end()) ? &it->second : nullptr;
}

bool SymbolTable::symbol_exists(const string &name) { return symbols.find(name) != symbols.end(); }

vector<Symbol> SymbolTable::get_all_symbols() const
{
    vector<Symbol> out;
    out.reserve(symbols.size());
    for (const auto &entry : symbols)
    {
        out.push_back(entry.second);
    }
    return out;
}

// Helper to register variable-like identifier usage (no redeclaration prevention beyond exists check)
static void register_identifier(const string &name, SymbolTable &table)
{
    if (!table.symbol_exists(name))
    {
        table.add_symbol(name, SymbolType::VARIABLE, "int"); // default type assumption
    }
}

static void walk_node(ASTNode *node, SymbolTable &table)
{
    if (!node)
        return;

    if (auto assign = dynamic_cast<AssignmentNode *>(node))
    {
        register_identifier(assign->identifier, table);
        walk_node(assign->expression.get(), table);
    }
    else if (auto print_n = dynamic_cast<PrintNode *>(node))
    {
        walk_node(print_n->expression.get(), table);
    }
    else if (auto input_n = dynamic_cast<InputNode *>(node))
    {
        register_identifier(input_n->identifier, table);
    }
    else if (auto bin = dynamic_cast<BinaryOpNode *>(node))
    {
        walk_node(bin->left.get(), table);
        walk_node(bin->right.get(), table);
    }
    else if (auto id = dynamic_cast<IdentifierNode *>(node))
    {
        register_identifier(id->name, table);
    }
    else if (auto seq = dynamic_cast<SeqNode *>(node))
    {
        for (auto &s : seq->statements)
            walk_node(s.get(), table);
    }
    else if (auto par = dynamic_cast<ParNode *>(node))
    {
        for (auto &s : par->statements)
            walk_node(s.get(), table);
    }
    else if (auto wh = dynamic_cast<WhileNode *>(node))
    {
        walk_node(wh->condition.get(), table);
        walk_node(wh->body.get(), table);
    }
    else if (auto prog = dynamic_cast<ProgramNode *>(node))
    {
        for (auto &s : prog->statements)
            walk_node(s.get(), table);
    }
}

void build_symbol_table(ProgramNode *program, SymbolTable &table)
{
    walk_node(program, table);
}

void print_symbol_table(const SymbolTable &table, ostream &out)
{
    auto all = table.get_all_symbols();
    out << "=== TABELA DE SÍMBOLOS ===\n";
    if (all.empty())
    {
        out << "<vazia>\n";
        return;
    }
    out << "Nome | TipoSimbolo | TipoDado\n";
    out << "-------------------------------\n";
    for (const auto &sym : all)
    {
        string stype;
        switch (sym.type)
        {
        case SymbolType::VARIABLE:
            stype = "VAR";
            break;
        case SymbolType::FUNCTION:
            stype = "FUN";
            break;
        case SymbolType::CHANNEL:
            stype = "CHAN";
            break;
        }
        out << sym.name << " | " << stype << " | " << (sym.data_type.empty() ? "?" : sym.data_type) << "\n";
    }
}
