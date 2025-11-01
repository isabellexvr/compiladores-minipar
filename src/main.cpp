#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include "symbol_table.h"
#include "tac_interpreter.h"
#include "semantic_channels.h"

using namespace std;

string read_file(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Erro ao abrir arquivo: " << filename << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::string timestamp_iso_utc()
{
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

static std::string token_category(TokenType t)
{
    switch (t)
    {
    case TokenType::SEQ:
    case TokenType::PAR:
    case TokenType::IF:
    case TokenType::ELSE:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::INPUT:
        return "KEYWORD";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
    case TokenType::STRING_LITERAL:
        return "LITERAL";
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::MULTIPLY:
    case TokenType::DIVIDE:
    case TokenType::ASSIGN:
    case TokenType::EQUAL:
    case TokenType::NOT_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return "OPERATOR";
    case TokenType::LPAREN:
    case TokenType::RPAREN:
    case TokenType::LBRACE:
    case TokenType::RBRACE:
    case TokenType::SEMICOLON:
    case TokenType::COMMA:
        return "DELIMITER";
    case TokenType::END:
        return "END";
    default:
        return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Uso: " << argv[0] << " <arquivo.minipar>" << std::endl;
        return 1;
    }
    std::string source_code = read_file(argv[1]);
    if (source_code.empty())
        return 1;

    // Header estilo phases
    std::cout << "=== COMPILATION METADATA ===\n";
    std::cout << "timestamp: " << timestamp_iso_utc() << "\n";
    std::cout << "compilerVersion: MiniPar 2025.1\n";
    std::cout << "sourceLength: " << source_code.size() << "\n";

    Lexer lexer(source_code);
    auto tokens = lexer.tokenize();

    std::cout << "\n=== LEXICAL ===\n";
    std::cout << "totalTokens: " << tokens.size() << "\n";
    std::cout << "Tokens (type/category/value@line:col)\n";
    for (const auto &tk : tokens)
    {
        std::cout << static_cast<int>(tk.type) << "/" << token_category(tk.type) << "/'" << tk.value << "'@" << tk.line << ':' << tk.column << "\n";
    }

    Parser parser(tokens);
    auto ast = parser.parse();
    bool success = ast != nullptr;

    std::cout << "\n=== SYNTAX ===\nstatus: " << (success ? "SUCCESS" : "ERROR") << "\n";
    if (success)
    {
        ASTPrinter printer;
        std::cout << "AST:\n"
                  << printer.print(*ast) << "\n";
    }

    std::cout << "\n=== SEMANTIC ===\n";
    SymbolTable symtab;
    if (success)
    {
        build_symbol_table(ast.get(), symtab);
    }
    auto all = symtab.get_all_symbols();
    std::cout << "symbols: " << all.size() << "\n";
    for (const auto &s : all)
    {
        std::string stype;
        switch (s.type)
        {
        case SymbolType::VARIABLE:
            stype = "VARIABLE";
            break;
        case SymbolType::FUNCTION:
            stype = "FUNCTION";
            break;
        case SymbolType::CHANNEL:
            stype = "CHANNEL";
            break;
        }
        std::cout << s.name << " | " << stype << " | " << (s.data_type.empty() ? "int" : s.data_type) << " | scope=global\n";
    }
    if (success)
    {
        std::cout << "Channel Arity Analysis:\n";
        analyze_channel_arities(static_cast<ProgramNode *>(ast.get()), std::cout);
    }

    std::cout << "\n=== INTERMEDIATE (TAC) ===\n";
    std::vector<TACInstruction> tac;
    if (success)
    {
        TACGenerator gen;
        tac = gen.generate(static_cast<ProgramNode *>(ast.get()));
    }
    std::cout << "instructions: " << tac.size() << "\n";
    for (const auto &i : tac)
    {
        std::string opType;
        if (i.op == "print")
            opType = "PRINT";
        else if (i.op == "label")
            opType = "LABEL";
        else if (i.op == "if_false")
            opType = "CONDITIONAL_JUMP";
        else if (i.op == "goto")
            opType = "JUMP";
        else if (i.op == "send")
            opType = "CHANNEL_SEND";
        else if (i.op == "send_arg")
            opType = "CHANNEL_SEND_ARG";
        else if (i.op == "receive")
            opType = "CHANNEL_RECEIVE";
        else if (i.op == "recv_arg")
            opType = "CHANNEL_RECV_ARG";
        else if (i.op.empty())
            opType = "ASSIGN";
        else
            opType = "BINARY";
        std::cout << i.result << " = (" << i.op << ") " << i.arg1 << (i.arg2.empty() ? "" : " , ") << i.arg2 << " | type=" << opType << "\n";
    }

    std::cout << "\n=== CODEGEN (ARMv7) ===\n";
    if (success)
    {
        ARMGenerator arm;
        auto code = arm.generate(tac);
        for (const auto &line : code)
            std::cout << line << "\n";
    }

    // Opcional: Execução simulada
    std::cout << "\n=== EXECUTION (SIMULATED) ===\n";
    if (success)
    {
        TACInterpreter interpreter;
        std::stringstream runtimeOut;
        auto finalEnv = interpreter.interpret(tac, runtimeOut);
        std::cout << "output:\n"
                  << runtimeOut.str();
        if (finalEnv.count("resultado"))
        {
            std::cout << "resultado(final)=" << finalEnv["resultado"] << "\n";
        }
    }
    std::cout << "done: " << (success ? "0" : "1") << "\n";
    return success ? 0 : 1;
}