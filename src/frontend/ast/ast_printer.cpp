#include "ast_printer.h"
#include "ast_nodes.h"
#include <iostream>

void ASTPrinter::addIndent()
{
    for (int i = 0; i < indentLevel; ++i)
    {
        output << "  ";
    }
}

void ASTPrinter::printLine(const std::string &text)
{
    addIndent();
    output << text << "\n";
}

std::string ASTPrinter::print(ASTNode &node)
{
    output.str("");
    indentLevel = 0;
    node.accept(*this);
    return output.str();
}

void ASTPrinter::visit(ProgramNode &node)
{
    printLine("Program:");
    indentLevel++;
    for (auto &stmt : node.statements)
    {
        stmt->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(SeqNode &node)
{
    printLine("SEQ:");
    indentLevel++;
    for (auto &stmt : node.statements)
    {
        stmt->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(ParNode &node)
{
    printLine("PAR:");
    indentLevel++;
    for (auto &stmt : node.statements)
    {
        stmt->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(VarDeclNode &node)
{
    printLine("VarDecl: " + node.type + " " + node.name);
}

void ASTPrinter::visit(ChannelDeclNode &node)
{
    printLine("Channel: " + node.name + " (" + node.comp1 + " <-> " + node.comp2 + ")");
}

void ASTPrinter::visit(AssignmentNode &node)
{
    printLine("Assignment: " + node.identifier + " = ");
    indentLevel++;
    node.expression->accept(*this);
    indentLevel--;
}

void ASTPrinter::visit(PrintNode &node)
{
    printLine("Print:");
    indentLevel++;
    node.expression->accept(*this);
    indentLevel--;
}

void ASTPrinter::visit(InputNode &node)
{
    printLine("Input: " + node.identifier);
}

void ASTPrinter::visit(SendNode &node)
{
    printLine("Send: " + node.channelName + (node.component.empty() ? "" : " from=" + node.component));
    indentLevel++;
    for (auto &arg : node.arguments)
    {
        arg->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(ReceiveNode &node)
{
    printLine("Receive: " + node.channelName + (node.component.empty() ? "" : " from=" + node.component));
    for (const auto &var : node.variables)
    {
        printLine("  -> " + var);
    }
}

void ASTPrinter::visit(IfNode &node)
{
    printLine("If:");
    indentLevel++;
    printLine("Condition:");
    indentLevel++;
    node.condition->accept(*this);
    indentLevel--;

    printLine("Then:");
    indentLevel++;
    node.thenBranch->accept(*this);
    indentLevel--;

    if (node.elseBranch)
    {
        printLine("Else:");
        indentLevel++;
        node.elseBranch->accept(*this);
        indentLevel--;
    }
    indentLevel--;
}

void ASTPrinter::visit(WhileNode &node)
{
    printLine("While:");
    indentLevel++;
    printLine("Condition:");
    indentLevel++;
    node.condition->accept(*this);
    indentLevel--;

    printLine("Body:");
    indentLevel++;
    node.body->accept(*this);
    indentLevel--;
    indentLevel--;
}

void ASTPrinter::visit(BinaryOpNode &node)
{
    std::string op_str;
    switch (node.op)
    {
    case TokenType::PLUS:
        op_str = "+";
        break;
    case TokenType::MINUS:
        op_str = "-";
        break;
    case TokenType::MULTIPLY:
        op_str = "*";
        break;
    case TokenType::DIVIDE:
        op_str = "/";
        break;
    case TokenType::AND:
        op_str = "&&";
        break;
    case TokenType::OR:
        op_str = "||";
        break;
    case TokenType::EQUAL:
        op_str = "==";
        break;
    case TokenType::NOT_EQUAL:
        op_str = "!=";
        break;
    case TokenType::LESS:
        op_str = "<";
        break;
    case TokenType::LESS_EQUAL:
        op_str = "<=";
        break;
    case TokenType::GREATER:
        op_str = ">";
        break;
    case TokenType::GREATER_EQUAL:
        op_str = ">=";
        break;
    default:
        op_str = "?";
        break;
    }
    printLine("BinaryOp: " + op_str);
    indentLevel++;
    printLine("Left:");
    indentLevel++;
    node.left->accept(*this);
    indentLevel--;
    printLine("Right:");
    indentLevel++;
    node.right->accept(*this);
    indentLevel--;
    indentLevel--;
}

void ASTPrinter::visit(UnaryOpNode &node)
{
    printLine("UnaryOp: " + node.op);
    indentLevel++;
    node.operand->accept(*this);
    indentLevel--;
}

void ASTPrinter::visit(NumberNode &node)
{
    printLine("Number: " + std::to_string(node.value));
}

void ASTPrinter::visit(StringNode &node)
{
    printLine("String: \"" + node.value + "\"");
}

void ASTPrinter::visit(IdentifierNode &node)
{
    printLine("Identifier: " + node.name);
}

void ASTPrinter::visit(BooleanNode &node)
{
    printLine("Boolean: " + std::string(node.value ? "true" : "false"));
}

void ASTPrinter::visit(FunctionDeclNode &node)
{
    printLine("FunctionDecl: " + node.name);
    if (node.body)
    {
        indentLevel++;
        node.body->accept(*this);
        indentLevel--;
    }
}

void ASTPrinter::visit(CallNode &node)
{
    printLine("Call: " + node.name);
    indentLevel++;
    for (auto &a : node.args)
    {
        a->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(ReturnNode &node)
{
    printLine("Return:");
    if (node.value)
    {
        indentLevel++;
        node.value->accept(*this);
        indentLevel--;
    }
}