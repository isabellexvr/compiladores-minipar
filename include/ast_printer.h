#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast_visitor.h"
#include <string>
#include <sstream>

// Forward declaration
struct ASTNode;

class ASTPrinter : public ASTVisitor
{
private:
    std::ostringstream output;
    int indentLevel = 0;

    void addIndent();
    void printLine(const std::string &text);

public:
    std::string print(ASTNode &node);

    // Implementações do visitor
    void visit(ProgramNode &node) override;
    void visit(SeqNode &node) override;
    void visit(ParNode &node) override;
    void visit(VarDeclNode &node) override;
    void visit(ChannelDeclNode &node) override;
    void visit(AssignmentNode &node) override;
    void visit(PrintNode &node) override;
    void visit(InputNode &node) override;
    void visit(InputCallNode &node) override;
    void visit(SendNode &node) override;
    void visit(ReceiveNode &node) override;
    void visit(IfNode &node) override;
    void visit(WhileNode &node) override;
    void visit(BinaryOpNode &node) override;
    void visit(UnaryOpNode &node) override;
    void visit(NumberNode &node) override;
    void visit(FloatNode &node) override;
    void visit(StringNode &node) override;
    void visit(IdentifierNode &node) override;
    void visit(BooleanNode &node) override;
    void visit(FunctionDeclNode &node) override;
    void visit(CallNode &node) override;
    void visit(ReturnNode &node) override;
    void visit(ArrayLiteralNode &node) override;
    void visit(ArrayAccessNode &node) override;
};

#endif