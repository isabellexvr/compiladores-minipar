#include "ast_printer.h"
#include "ast_nodes.h"
#include <iostream>
#include <unordered_set>

// Simple cycle guard (defensive) to avoid infinite recursion if malformed AST links appear.

class VisitGuard
{
public:
    std::unordered_set<const ASTNode *> seen;
    bool check(const ASTNode *n)
    {
        if (!n)
            return false;
        if (seen.count(n))
            return true;
        seen.insert(n);
        return false;
    }
};

static VisitGuard gVisitGuard; // global for this translation unit (printer runs once per program)

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
        if (gVisitGuard.check(stmt.get()))
        {
            printLine("<cycle detected>");
            continue;
        }
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
        if (gVisitGuard.check(stmt.get()))
        {
            printLine("<cycle detected>");
            continue;
        }
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
        if (gVisitGuard.check(stmt.get()))
        {
            printLine("<cycle detected>");
            continue;
        }
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
    if (node.expression)
    {
        if (!gVisitGuard.check(node.expression.get()))
            node.expression->accept(*this);
        else
            printLine("<cycle expr>");
    }
    else
        printLine("<null expr>");
    indentLevel--;
}

void ASTPrinter::visit(PrintNode &node)
{
    printLine("Print(" + std::to_string(node.expressions.size()) + "):");
    indentLevel++;
    for (auto &e : node.expressions)
    {
        if (!e)
        {
            printLine("<null print expr>");
            continue;
        }
        if (gVisitGuard.check(e.get()))
        {
            printLine("<cycle print expr>");
            continue;
        }
        e->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(InputNode &node)
{
    printLine("Input: " + node.identifier);
}

void ASTPrinter::visit(InputCallNode &node)
{
    printLine("InputCall()");
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
    if (node.condition)
    {
        if (!gVisitGuard.check(node.condition.get()))
            node.condition->accept(*this);
        else
            printLine("<cycle condition>");
    }
    else
        printLine("<null condition>");
    indentLevel--;

    printLine("Then:");
    indentLevel++;
    if (node.thenBranch)
    {
        if (!gVisitGuard.check(node.thenBranch.get()))
            node.thenBranch->accept(*this);
        else
            printLine("<cycle then>");
    }
    else
        printLine("<null then>");
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
    if (node.condition)
    {
        if (!gVisitGuard.check(node.condition.get()))
            node.condition->accept(*this);
        else
            printLine("<cycle condition>");
    }
    else
        printLine("<null condition>");
    indentLevel--;

    printLine("Body:");
    indentLevel++;
    if (node.body)
    {
        if (!gVisitGuard.check(node.body.get()))
            node.body->accept(*this);
        else
            printLine("<cycle body>");
    }
    else
        printLine("<null body>");
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
    if (node.left && !gVisitGuard.check(node.left.get()))
        node.left->accept(*this);
    else if (!node.left)
        printLine("<null left>");
    else
        printLine("<cycle left>");
    indentLevel--;
    printLine("Right:");
    indentLevel++;
    if (node.right && !gVisitGuard.check(node.right.get()))
        node.right->accept(*this);
    else if (!node.right)
        printLine("<null right>");
    else
        printLine("<cycle right>");
    indentLevel--;
    indentLevel--;
}

void ASTPrinter::visit(UnaryOpNode &node)
{
    printLine("UnaryOp: " + node.op);
    indentLevel++;
    if (node.operand && !gVisitGuard.check(node.operand.get()))
        node.operand->accept(*this);
    else if (!node.operand)
        printLine("<null operand>");
    else
        printLine("<cycle operand>");
    indentLevel--;
}

void ASTPrinter::visit(NumberNode &node)
{
    printLine("Number: " + std::to_string(node.value));
}

void ASTPrinter::visit(FloatNode &node)
{
    printLine("Float: " + std::to_string(node.value));
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
        if (!gVisitGuard.check(node.body.get()))
            node.body->accept(*this);
        else
            printLine("<cycle body>");
        indentLevel--;
    }
    else
    {
        indentLevel++;
        printLine("<null body>");
        indentLevel--;
    }
}

void ASTPrinter::visit(CallNode &node)
{
    printLine("Call: " + node.name);
    indentLevel++;
    for (auto &a : node.args)
    {
        if (!a)
        {
            printLine("<null arg>");
            continue;
        }
        if (gVisitGuard.check(a.get()))
        {
            printLine("<cycle arg>");
            continue;
        }
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
        if (!gVisitGuard.check(node.value.get()))
            node.value->accept(*this);
        else
            printLine("<cycle return value>");
        indentLevel--;
    }
}

void ASTPrinter::visit(ArrayLiteralNode &node)
{
    printLine("ArrayLiteral(" + std::to_string(node.elements.size()) + "):");
    indentLevel++;
    for (auto &el : node.elements)
    {
        if (!el)
        {
            printLine("<null element>");
            continue;
        }
        if (gVisitGuard.check(el.get()))
        {
            printLine("<cycle element>");
            continue;
        }
        el->accept(*this);
    }
    indentLevel--;
}

void ASTPrinter::visit(ArrayAccessNode &node)
{
    printLine("ArrayAccess:");
    indentLevel++;
    printLine("Base:");
    indentLevel++;
    if (node.base && !gVisitGuard.check(node.base.get()))
        node.base->accept(*this);
    else if (!node.base)
        printLine("<null base>");
    else
        printLine("<cycle base>");
    indentLevel--;
    printLine("Index:");
    indentLevel++;
    if (node.index && !gVisitGuard.check(node.index.get()))
        node.index->accept(*this);
    else if (!node.index)
        printLine("<null index>");
    else
        printLine("<cycle index>");
    indentLevel--;
    indentLevel--;
}