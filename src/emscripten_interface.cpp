#include "emscripten_interface.h"
#include "lexer.h"
#include "parser.h"
#include "tac_generator.h"
#include "arm_generator.h"
#include "ast_printer.h"
#include "symbol_table.h"
#include "emscripten/json_helpers.h"
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <chrono>
#include <iomanip>

using namespace std;

// Função para obter timestamp atual
string get_current_timestamp()
{
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

// Função para converter tipo de token para string
string token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TokenType::SEQ:
        return "KEYWORD";
    case TokenType::PAR:
        return "KEYWORD";
    case TokenType::WHILE:
        return "KEYWORD";
    case TokenType::PRINT:
        return "KEYWORD";
    case TokenType::INPUT:
        return "KEYWORD";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::ASSIGN:
        return "OPERATOR";
    case TokenType::PLUS:
        return "OPERATOR";
    case TokenType::MINUS:
        return "OPERATOR";
    case TokenType::MULTIPLY:
        return "OPERATOR";
    case TokenType::DIVIDE:
        return "OPERATOR";
    case TokenType::LPAREN:
        return "DELIMITER";
    case TokenType::RPAREN:
        return "DELIMITER";
    case TokenType::LBRACE:
        return "DELIMITER";
    case TokenType::RBRACE:
        return "DELIMITER";
    case TokenType::SEMICOLON:
        return "DELIMITER";
    case TokenType::LESS_EQUAL:
        return "OPERATOR";
    case TokenType::GREATER_EQUAL:
        return "OPERATOR";
    case TokenType::EQUAL:
        return "OPERATOR";
    case TokenType::NOT_EQUAL:
        return "OPERATOR";
    case TokenType::LESS:
        return "OPERATOR";
    case TokenType::GREATER:
        return "OPERATOR";
    case TokenType::END:
        return "END";
    default:
        return "UNKNOWN";
    }
}

// Função simple para interpretar TAC e gerar output (mantida igual)
std::string interpretTAC(const std::string &tacCode)
{
    std::map<std::string, int> variables;
    std::vector<std::string> outputLines;
    std::map<std::string, size_t> labels;
    std::vector<std::string> instructions;

    // Primeira passada: extrair instruções e labels
    std::istringstream iss(tacCode);
    std::string line;

    while (std::getline(iss, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        if (line.back() == ':')
        {
            std::string label = line.substr(0, line.length() - 1);
            labels[label] = instructions.size();
        }
        else
        {
            instructions.push_back(line);
        }
    }

    // Segunda passada: executar instruções
    size_t pc = 0;
    size_t max_iterations = 1000;
    size_t iterations = 0;

    while (pc < instructions.size() && iterations < max_iterations)
    {
        iterations++;
        std::string line = instructions[pc];
        pc++;

        if (line.find("print") == 0)
        {
            std::string varName = line.substr(5);
            varName.erase(0, varName.find_first_not_of(" \t"));

            if (variables.find(varName) != variables.end())
            {
                outputLines.push_back(std::to_string(variables[varName]));
            }
            else
            {
                outputLines.push_back("[undefined variable: " + varName + "]");
            }
        }
        else if (line.find("if_false") == 0)
        {
            size_t pos = line.find("goto");
            if (pos != std::string::npos)
            {
                std::string conditionVar = line.substr(8, pos - 8);
                conditionVar.erase(0, conditionVar.find_first_not_of(" \t"));
                conditionVar.erase(conditionVar.find_last_not_of(" \t") + 1);

                std::string label = line.substr(pos + 4);
                label.erase(0, label.find_first_not_of(" \t"));
                label.erase(label.find_last_not_of(" \t") + 1);

                if (variables.find(conditionVar) != variables.end() && variables[conditionVar] == 0)
                {
                    if (labels.find(label) != labels.end())
                    {
                        pc = labels[label];
                    }
                }
            }
        }
        else if (line.find("goto") == 0)
        {
            std::string label = line.substr(4);
            label.erase(0, label.find_first_not_of(" \t"));
            label.erase(label.find_last_not_of(" \t") + 1);

            if (labels.find(label) != labels.end())
            {
                pc = labels[label];
            }
        }
        else if (line.find('=') != std::string::npos)
        {
            size_t equalPos = line.find('=');
            std::string left = line.substr(0, equalPos);
            std::string right = line.substr(equalPos + 1);

            left.erase(0, left.find_first_not_of(" \t"));
            left.erase(left.find_last_not_of(" \t") + 1);
            right.erase(0, right.find_first_not_of(" \t"));
            right.erase(right.find_last_not_of(" \t") + 1);

            int value = 0;

            if (right.find_first_not_of("0123456789") == std::string::npos)
            {
                value = std::stoi(right);
            }
            else if (right.find("<=") != std::string::npos)
            {
                size_t opPos = right.find("<=");
                std::string leftVar = right.substr(0, opPos);
                std::string rightVar = right.substr(opPos + 2);

                leftVar.erase(0, leftVar.find_first_not_of(" \t"));
                leftVar.erase(leftVar.find_last_not_of(" \t") + 1);
                rightVar.erase(0, rightVar.find_first_not_of(" \t"));
                rightVar.erase(rightVar.find_last_not_of(" \t") + 1);

                int leftVal = 0, rightVal = 0;

                if (variables.find(leftVar) != variables.end())
                {
                    leftVal = variables[leftVar];
                }
                else if (leftVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    leftVal = std::stoi(leftVar);
                }

                if (variables.find(rightVar) != variables.end())
                {
                    rightVal = variables[rightVar];
                }
                else if (rightVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    rightVal = std::stoi(rightVar);
                }

                value = (leftVal <= rightVal) ? 1 : 0;
            }
            else if (right.find('*') != std::string::npos)
            {
                size_t opPos = right.find('*');
                std::string leftVar = right.substr(0, opPos);
                std::string rightVar = right.substr(opPos + 1);

                leftVar.erase(0, leftVar.find_first_not_of(" \t"));
                leftVar.erase(leftVar.find_last_not_of(" \t") + 1);
                rightVar.erase(0, rightVar.find_first_not_of(" \t"));
                rightVar.erase(rightVar.find_last_not_of(" \t") + 1);

                int leftVal = 0, rightVal = 0;

                if (variables.find(leftVar) != variables.end())
                {
                    leftVal = variables[leftVar];
                }
                else if (leftVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    leftVal = std::stoi(leftVar);
                }

                if (variables.find(rightVar) != variables.end())
                {
                    rightVal = variables[rightVar];
                }
                else if (rightVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    rightVal = std::stoi(rightVar);
                }

                value = leftVal * rightVal;
            }
            else if (right.find('+') != std::string::npos)
            {
                size_t opPos = right.find('+');
                std::string leftVar = right.substr(0, opPos);
                std::string rightVar = right.substr(opPos + 1);

                leftVar.erase(0, leftVar.find_first_not_of(" \t"));
                leftVar.erase(leftVar.find_last_not_of(" \t") + 1);
                rightVar.erase(0, rightVar.find_first_not_of(" \t"));
                rightVar.erase(rightVar.find_last_not_of(" \t") + 1);

                int leftVal = 0, rightVal = 0;

                if (variables.find(leftVar) != variables.end())
                {
                    leftVal = variables[leftVar];
                }
                else if (leftVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    leftVal = std::stoi(leftVar);
                }

                if (variables.find(rightVar) != variables.end())
                {
                    rightVal = variables[rightVar];
                }
                else if (rightVar.find_first_not_of("0123456789") == std::string::npos)
                {
                    rightVal = std::stoi(rightVar);
                }

                value = leftVal + rightVal;
            }
            else if (variables.find(right) != variables.end())
            {
                value = variables[right];
            }
            else if (right.find_first_not_of("0123456789") == std::string::npos)
            {
                value = std::stoi(right);
            }

            variables[left] = value;
        }
    }

    if (iterations >= max_iterations)
    {
        outputLines.push_back("[Execution stopped: possible infinite loop]");
    }

    std::string result;
    for (const auto &line : outputLines)
    {
        result += line + "\n";
    }

    return result.empty() ? "No output generated" : result;
}

// Função principal - NOVA VERSÃO JSON
EMSCRIPTEN_KEEPALIVE
char *compile_minipar(const char *source_code)
{
    // Helper lambdas to modularize phases without changing output schema
    auto escape_json = [](const string &in)
    {
        string out = in;
        for (size_t pos = 0; (pos = out.find('"', pos)) != string::npos;)
        {
            out.replace(pos, 1, "\\\"");
            pos += 2;
        }
        for (size_t pos = 0; (pos = out.find('\n', pos)) != string::npos;)
        {
            out.replace(pos, 1, "\\n");
        }
        return out;
    };

    auto build_metadata = [&](stringstream &json, bool success, const string &source)
    {
        json << "\"compilation\":{";
        json << "\"success\":" << (success ? "true" : "false") << ",";
        json << "\"timestamp\":\"" << get_current_timestamp() << "\",";
        json << "\"compilerVersion\":\"MiniPar 2025.1\",";
        json << "\"source\":\"" << escape_json(source) << "\"}";
    };

    auto build_lexical = [&](stringstream &json, const vector<Token> &tokens)
    {
        json << "\"lexical\":{\"tokens\":"; build_tokens_array(json, tokens, true); json << "}"; };

    auto build_syntax = [&](stringstream &json, ASTNode *ast)
    {
        json << "\"syntax\":{";
        if (ast)
        {
            json << "\"status\":\"SUCCESS\",";
            ASTPrinter printer;
            string astStr = printer.print(*ast);
            astStr = escape_json(astStr);
            json << "\"ast\":\"" << astStr << "\"";
        }
        else
        {
            json << "\"status\":\"ERROR\"";
        }
        json << "}";
    };

    auto build_semantic = [&](stringstream &json, ASTNode *ast)
    {
        json << "\"semantic\":{\"symbols\":[";
        if (ast)
        {
            SymbolTable symtab;
            build_symbol_table(static_cast<ProgramNode *>(ast), symtab);
            auto symbols = symtab.get_all_symbols();
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                const auto &s = symbols[i];
                string stype;
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
                json << "{\"name\":\"" << escape_json(s.name) << "\",\"type\":\"" << stype
                     << "\",\"dataType\":\"" << (s.data_type.empty() ? "int" : escape_json(s.data_type)) << "\",\"scope\":\"global\"}";
                if (i + 1 < symbols.size())
                    json << ",";
            }
        }
        json << "]}";
    };

    struct IntermediateResult
    {
        vector<TACInstruction> tac;
        string tacText;
    };
    auto build_intermediate = [&](stringstream &json, ASTNode *ast)
    {
        json << "\"intermediate\":{";
        IntermediateResult res;
        if (ast)
        {
            TACGenerator gen;
            res.tac = gen.generate(static_cast<ProgramNode *>(ast));
            stringstream ss;
            gen.print_tac(ss);
            res.tacText = ss.str();
            json << "\"tac\":[";
            for (size_t i = 0; i < res.tac.size(); ++i)
            {
                const auto &instr = res.tac[i];
                string operation;
                if (instr.op == "print")
                    operation = "PRINT";
                else if (instr.op == "label")
                    operation = "LABEL";
                else if (instr.op == "if_false")
                    operation = "CONDITIONAL_JUMP";
                else if (instr.op == "goto")
                    operation = "JUMP";
                else if (instr.op.empty())
                    operation = "ASSIGN";
                else
                    operation = instr.op;
                json << "{\"id\":" << i << ",\"result\":\"" << escape_json(instr.result) << "\",\"operation\":\"" << operation << "\",\"operands\":[";
                bool first = true;
                if (!instr.arg1.empty())
                {
                    json << "\"" << escape_json(instr.arg1) << "\"";
                    first = false;
                }
                if (!instr.arg2.empty())
                {
                    if (!first)
                        json << ",";
                    json << "\"" << escape_json(instr.arg2) << "\"";
                }
                bool isTemp = (instr.result.find("t") == 0) && (instr.result.find_first_not_of("0123456789", 1) == string::npos);
                json << "],\"isTemporary\":" << (isTemp ? "true" : "false") << "}";
                if (i + 1 < res.tac.size())
                    json << ",";
            }
            json << "]";
        }
        else
        {
            json << "\"tac\":[]";
        }
        json << "}";
        return res;
    };

    auto build_codegen = [&](stringstream &json, ASTNode *ast, const vector<TACInstruction> &tac)
    {
        json << "\"codegen\":{";
        json << "\"target\":\"ARMv7\",";
        if (ast)
        {
            ARMGenerator arm;
            auto code = arm.generate(tac);
            json << "\"code\":[";
            for (size_t i = 0; i < code.size(); ++i)
            {
                string line = escape_json(code[i]);
                json << "\"" << line << "\"";
                if (i + 1 < code.size())
                    json << ",";
            }
            json << "]";
        }
        else
        {
            json << "\"code\":[]";
        }
        json << "}";
    };

    auto build_execution = [&](stringstream &json, const string &tacText)
    {
        json << "\"execution\":{";
        if (!tacText.empty())
        {
            string out = interpretTAC(tacText);
            out = escape_json(out);
            json << "\"output\":\"" << out << "\",\"exitCode\":0";
        }
        else
        {
            json << "\"output\":\"\",\"exitCode\":1";
        }
        json << "}";
    };

    try
    {
        string source(source_code);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();
        bool success = ast != nullptr;
        stringstream json;
        json << "{";
        build_metadata(json, success, source);
        json << ",\"phases\":{";
        build_lexical(json, tokens);
        json << ",";
        build_syntax(json, ast.get());
        json << ",";
        build_semantic(json, ast.get());
        json << ",";
        IntermediateResult interm = build_intermediate(json, ast.get());
        json << ",";
        build_codegen(json, ast.get(), interm.tac);
        json << "},"; // close phases
        build_execution(json, interm.tacText);
        json << "}"; // root
        string output = json.str();
        char *output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
    }
    catch (const exception &e)
    {
        string error = string("{\"compilation\":{\"success\":false,\"error\":\"") + e.what() + "\"}}";
        char *err = new char[error.length() + 1];
        strcpy(err, error.c_str());
        return err;
    }
}

EMSCRIPTEN_KEEPALIVE
void free_string(char *str)
{
    delete[] str;
}

EMSCRIPTEN_KEEPALIVE
char *compile_minipar_json(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream json_result;

        json_result << "{";

        // 1. Análise Léxica - Retornar tokens estruturados
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        json_result << "\"tokens\":";
        build_tokens_array(json_result, tokens, false);
        json_result << ",";

        // 2. Análise Sintática
        Parser parser(tokens);
        auto ast = parser.parse();

        json_result << "\"ast\":\"" << (ast ? "SUCCESS" : "ERROR") << "\",";

        // 3. Código de 3 Endereços
        if (ast)
        {
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());

            // Symbol table JSON
            SymbolTable symtab;
            build_symbol_table(ast.get(), symtab);
            json_result << "\"symbols\":[";
            auto all = symtab.get_all_symbols();
            for (size_t i = 0; i < all.size(); ++i)
            {
                const auto &sym = all[i];
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
                json_result << "{\"name\":\"" << sym.name << "\",\"symbolType\":\"" << stype
                            << "\",\"dataType\":\"" << (sym.data_type.empty() ? "?" : sym.data_type) << "\"}";
                if (i < all.size() - 1)
                    json_result << ",";
            }
            json_result << "],";

            // TAC estruturado
            json_result << "\"tac\":[";
            for (size_t i = 0; i < tac.size(); ++i)
            {
                const auto &instr = tac[i];
                json_result << "{"
                            << "\"result\":\"" << instr.result << "\","
                            << "\"arg1\":\"" << instr.arg1 << "\","
                            << "\"op\":\"" << instr.op << "\","
                            << "\"arg2\":\"" << instr.arg2 << "\","
                            << "\"type\":\"";

                // Classificação de tipos
                if (instr.op == "print")
                {
                    json_result << "PRINT";
                }
                else if (instr.op == "label")
                {
                    json_result << "LABEL";
                }
                else if (instr.op == "if_false")
                {
                    json_result << "CONDITIONAL_JUMP";
                }
                else if (instr.op == "goto")
                {
                    json_result << "JUMP";
                }
                else if (instr.op.empty())
                {
                    json_result << "ASSIGNMENT";
                }
                else
                {
                    json_result << "BINARY_OP";
                }

                json_result << "\"}";

                if (i < tac.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "],";

            // 4. Código ARM
            ARMGenerator arm_gen;
            auto arm_code = arm_gen.generate(tac);

            json_result << "\"arm\":[";
            for (size_t i = 0; i < arm_code.size(); ++i)
            {
                json_result << "\"" << arm_code[i] << "\"";
                if (i < arm_code.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "]";
        }
        else
        {
            json_result << "\"error\":\"Syntactic analysis failed\"";
        }

        json_result << "}";

        string output = json_result.str();
        char *output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
    }
    catch (const exception &e)
    {
        string error = "{\"error\":\"Exception: " + string(e.what()) + "\"}";
        char *error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}

EMSCRIPTEN_KEEPALIVE
char *compile_minipar_tac(const char *source_code)
{
    try
    {
        string source(source_code);
        stringstream json_result;

        json_result << "{";

        // Análise Léxica e Sintática
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        if (ast)
        {
            // Gerar TAC
            TACGenerator tac_gen;
            auto tac = tac_gen.generate(ast.get());

            json_result << "\"success\": true,";
            json_result << "\"tac\":[";

            for (size_t i = 0; i < tac.size(); ++i)
            {
                const auto &instr = tac[i];
                json_result << "{"
                            << "\"result\":\"" << instr.result << "\","
                            << "\"op\":\"" << instr.op << "\","
                            << "\"arg1\":\"" << instr.arg1 << "\","
                            << "\"arg2\":\"" << instr.arg2 << "\","
                            << "\"type\":\"";

                // Classificar o tipo de instrução para facilitar no frontend
                if (instr.op == "print")
                {
                    json_result << "PRINT";
                }
                else if (instr.op == "label")
                {
                    json_result << "LABEL";
                }
                else if (instr.op == "if_false")
                {
                    json_result << "CONDITIONAL_JUMP";
                }
                else if (instr.op == "goto")
                {
                    json_result << "JUMP";
                }
                else if (instr.op.empty())
                {
                    json_result << "ASSIGNMENT";
                }
                else
                {
                    json_result << "BINARY_OP";
                }

                json_result << "\"}";

                if (i < tac.size() - 1)
                {
                    json_result << ",";
                }
            }
            json_result << "]";
        }
        else
        {
            json_result << "\"success\": false,";
            json_result << "\"error\":\"Syntactic analysis failed\"";
        }

        json_result << "}";

        string output = json_result.str();
        char *output_cstr = new char[output.length() + 1];
        strcpy(output_cstr, output.c_str());
        return output_cstr;
    }
    catch (const exception &e)
    {
        string error = "{\"success\":false,\"error\":\"Exception: " + string(e.what()) + "\"}";
        char *error_cstr = new char[error.length() + 1];
        strcpy(error_cstr, error.c_str());
        return error_cstr;
    }
}