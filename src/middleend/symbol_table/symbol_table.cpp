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
        table.add_symbol(name, SymbolType::VARIABLE, "int"); // default until inferred
    }
}

static std::string infer_type(ASTNode *node)
{
    if (!node)
        return "?";
    if (dynamic_cast<NumberNode *>(node))
        return "int";
    if (dynamic_cast<FloatNode *>(node))
        return "float";
    if (dynamic_cast<BooleanNode *>(node))
        return "bool";
    if (dynamic_cast<StringNode *>(node))
        return "string";
    if (auto bin = dynamic_cast<BinaryOpNode *>(node))
    {
        auto lt = infer_type(bin->left.get());
        auto rt = infer_type(bin->right.get());
        if (lt == rt)
            return lt; // simplistic
        return "?";
    }
    if (dynamic_cast<ArrayLiteralNode *>(node))
        return "array";
    if (auto acc = dynamic_cast<ArrayAccessNode *>(node))
    {
        // type of access is element type; for now assume int
        return "int";
    }
    if (auto un = dynamic_cast<UnaryOpNode *>(node))
        return infer_type(un->operand.get());
    if (dynamic_cast<IdentifierNode *>(node))
        return "int"; // fallback
    return "?";
}

static void walk_node(ASTNode *node, SymbolTable &table, Symbol *currentFunction = nullptr)
{
    if (!node)
        return;

    if (auto assign = dynamic_cast<AssignmentNode *>(node))
    {
        register_identifier(assign->identifier, table);
        walk_node(assign->expression.get(), table, currentFunction);
        // refine type
        Symbol *sym = table.get_symbol(assign->identifier);
        if (sym)
        {
            std::string t = infer_type(assign->expression.get());
            if (t != "?")
                sym->data_type = t;
        }
    }
    else if (auto print_n = dynamic_cast<PrintNode *>(node))
    {
        for (auto &expr : print_n->expressions)
            walk_node(expr.get(), table, currentFunction);
    }
    else if (auto input_n = dynamic_cast<InputNode *>(node))
    {
        register_identifier(input_n->identifier, table);
    }
    else if (auto bin = dynamic_cast<BinaryOpNode *>(node))
    {
        walk_node(bin->left.get(), table, currentFunction);
        walk_node(bin->right.get(), table, currentFunction);
    }
    else if (auto send = dynamic_cast<SendNode *>(node))
    {
        // garante registro do canal
        if (!table.symbol_exists(send->channelName))
            table.add_symbol(send->channelName, SymbolType::CHANNEL, "channel");
        for (auto &a : send->arguments)
            walk_node(a.get(), table, currentFunction);
    }
    else if (auto recv = dynamic_cast<ReceiveNode *>(node))
    {
        if (!table.symbol_exists(recv->channelName))
            table.add_symbol(recv->channelName, SymbolType::CHANNEL, "channel");
        for (auto &v : recv->variables)
        {
            register_identifier(v, table);
        }
    }
    else if (auto ch = dynamic_cast<ChannelDeclNode *>(node))
    {
        if (!table.symbol_exists(ch->name))
            table.add_symbol(ch->name, SymbolType::CHANNEL, "channel");
    }
    else if (auto call = dynamic_cast<CallNode *>(node))
    {
        // function symbol registration (assume exists or create placeholder)
        if (!table.symbol_exists(call->name))
            table.add_symbol(call->name, SymbolType::FUNCTION, "func");
        for (auto &a : call->args)
            walk_node(a.get(), table, currentFunction);
    }
    else if (auto fdecl = dynamic_cast<FunctionDeclNode *>(node))
    {
        if (!table.symbol_exists(fdecl->name))
            table.add_symbol(fdecl->name, SymbolType::FUNCTION, "void");
        Symbol *fsym = table.get_symbol(fdecl->name);
        walk_node(fdecl->body.get(), table, fsym);
    }
    else if (auto ret = dynamic_cast<ReturnNode *>(node))
    {
        walk_node(ret->value.get(), table, currentFunction);
        if (currentFunction)
        {
            currentFunction->has_return = true;
            std::string rtype = infer_type(ret->value.get());
            if (!rtype.empty() && rtype != "?")
            {
                currentFunction->return_type = rtype;
                currentFunction->data_type = rtype; // sobrescreve tipo genérico
            }
        }
    }
    else if (auto id = dynamic_cast<IdentifierNode *>(node))
    {
        register_identifier(id->name, table);
    }
    else if (auto seq = dynamic_cast<SeqNode *>(node))
    {
        for (auto &s : seq->statements)
            walk_node(s.get(), table, currentFunction);
    }
    else if (auto par = dynamic_cast<ParNode *>(node))
    {
        for (auto &s : par->statements)
            walk_node(s.get(), table, currentFunction);
    }
    else if (auto wh = dynamic_cast<WhileNode *>(node))
    {
        walk_node(wh->condition.get(), table, currentFunction);
        walk_node(wh->body.get(), table, currentFunction);
    }
    else if (auto prog = dynamic_cast<ProgramNode *>(node))
    {
        for (auto &s : prog->statements)
            walk_node(s.get(), table, currentFunction);
    }
}

void build_symbol_table(ProgramNode *program, SymbolTable &table)
{
    walk_node(program, table, nullptr);
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
