#ifndef AST_VISITOR_H
#define AST_VISITOR_H

// Forward declarations de todos os nós
struct ProgramNode;
struct SeqNode;
struct ParNode;
struct VarDeclNode;
struct ChannelDeclNode;
struct AssignmentNode;
struct PrintNode;
struct InputNode;
struct SendNode;
struct ReceiveNode;
struct IfNode;
struct WhileNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct NumberNode;
struct FloatNode;
struct StringNode;
struct IdentifierNode;
struct BooleanNode;
struct FunctionDeclNode;
struct CallNode;
struct ReturnNode;

// Visitor pattern para percorrer a AST
class ASTVisitor
{
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode &node) = 0;
    virtual void visit(SeqNode &node) = 0;
    virtual void visit(ParNode &node) = 0;
    virtual void visit(VarDeclNode &node) = 0;
    virtual void visit(ChannelDeclNode &node) = 0;
    virtual void visit(AssignmentNode &node) = 0;
    virtual void visit(PrintNode &node) = 0;
    virtual void visit(InputNode &node) = 0;
    virtual void visit(SendNode &node) = 0;
    virtual void visit(ReceiveNode &node) = 0;
    virtual void visit(IfNode &node) = 0;
    virtual void visit(WhileNode &node) = 0;
    virtual void visit(BinaryOpNode &node) = 0;
    virtual void visit(UnaryOpNode &node) = 0;
    virtual void visit(NumberNode &node) = 0;
    virtual void visit(FloatNode &node) = 0;
    virtual void visit(StringNode &node) = 0;
    virtual void visit(IdentifierNode &node) = 0;
    virtual void visit(BooleanNode &node) = 0;
    virtual void visit(FunctionDeclNode &node) = 0;
    virtual void visit(CallNode &node) = 0;
    virtual void visit(ReturnNode &node) = 0;
};

#endif