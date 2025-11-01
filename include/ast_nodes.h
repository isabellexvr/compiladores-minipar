#ifndef AST_NODES_H
#define AST_NODES_H

#include <memory>
#include <vector>
#include <string>
#include <lexer.h>

// Forward declarations
class ASTVisitor;

// Classe base para todos os nós da AST
struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
    virtual std::string toString() const = 0;
};

// Programa completo
struct ProgramNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Blocos SEQ e PAR
struct SeqNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct ParNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> statements;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Declarações
struct VarDeclNode : public ASTNode
{
    std::string type;
    std::string name;
    std::unique_ptr<ASTNode> initializer;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct ChannelDeclNode : public ASTNode
{
    std::string name;
    std::string comp1;
    std::string comp2;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Comandos
struct AssignmentNode : public ASTNode
{
    std::string identifier;
    std::unique_ptr<ASTNode> expression;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct PrintNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> expressions; // múltiplos argumentos
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct InputNode : public ASTNode
{
    std::string identifier;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct SendNode : public ASTNode
{
    std::string channelName;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    std::string component;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct ReceiveNode : public ASTNode
{
    std::string channelName;
    std::vector<std::string> variables;
    std::string component;
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct FunctionDeclNode : public ASTNode
{
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<ASTNode> body; // SeqNode or single
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct CallNode : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct ReturnNode : public ASTNode
{
    std::unique_ptr<ASTNode> value;
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Estruturas de controle
struct IfNode : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct WhileNode : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Expressões
struct BinaryOpNode : public ASTNode
{
    TokenType op; // operador como enum
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct UnaryOpNode : public ASTNode
{
    std::string op;
    std::unique_ptr<ASTNode> operand;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct NumberNode : public ASTNode
{
    int value;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct FloatNode : public ASTNode
{
    double value;
    void accept(ASTVisitor &visitor) override; // placeholder visiting not yet implemented
    std::string toString() const override { return "Float:" + std::to_string(value); }
};

struct StringNode : public ASTNode
{
    std::string value;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct IdentifierNode : public ASTNode
{
    std::string name;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

struct BooleanNode : public ASTNode
{
    bool value;

    void accept(ASTVisitor &visitor) override;
    std::string toString() const override;
};

// Array literal
struct ArrayLiteralNode : public ASTNode
{
    std::vector<std::unique_ptr<ASTNode>> elements;
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override { return "ArrayLiteral(" + std::to_string(elements.size()) + " elements)"; }
};

// Array access: base[index]
struct ArrayAccessNode : public ASTNode
{
    std::unique_ptr<ASTNode> base;   // typically IdentifierNode or another ArrayAccessNode
    std::unique_ptr<ASTNode> index;  // expression yielding integer
    void accept(ASTVisitor &visitor) override;
    std::string toString() const override { return "ArrayAccess"; }
};

#endif