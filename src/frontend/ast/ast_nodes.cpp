#include "ast_nodes.h"
#include "ast_visitor.h"

// ProgramNode
void ProgramNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string ProgramNode::toString() const
{
    return "Program(" + std::to_string(statements.size()) + " statements)";
}

// SeqNode
void SeqNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string SeqNode::toString() const
{
    return "SEQ(" + std::to_string(statements.size()) + " statements)";
}

// ParNode
void ParNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string ParNode::toString() const
{
    return "PAR(" + std::to_string(statements.size()) + " statements)";
}

// VarDeclNode
void VarDeclNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string VarDeclNode::toString() const
{
    return "VarDecl(" + type + " " + name + ")";
}

// ChannelDeclNode
void ChannelDeclNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string ChannelDeclNode::toString() const
{
    return "Channel(" + name + ": " + comp1 + " <-> " + comp2 + ")";
}

// AssignmentNode
void AssignmentNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string AssignmentNode::toString() const
{
    return "Assign(" + identifier + " = ...)";
}

// PrintNode
void PrintNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string PrintNode::toString() const
{
    return "Print(...)";
}

// InputNode
void InputNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string InputNode::toString() const
{
    return "Input(" + identifier + ")";
}

// SendNode
void SendNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string SendNode::toString() const
{
    return "Send(" + channelName + ", " + std::to_string(arguments.size()) + " args)";
}

// ReceiveNode
void ReceiveNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string ReceiveNode::toString() const
{
    return "Receive(" + channelName + ", " + std::to_string(variables.size()) + " vars)";
}

// IfNode
void IfNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string IfNode::toString() const
{
    return "If(condition ...)";
}

// WhileNode
void WhileNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string WhileNode::toString() const
{
    return "While(condition ...)";
}

// BinaryOpNode
void BinaryOpNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string BinaryOpNode::toString() const
{
    std::string op_str;
    switch (op)
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
    default:
        op_str = "?";
    }
    return "BinaryOp(" + op_str + ")";
}

// UnaryOpNode
void UnaryOpNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string UnaryOpNode::toString() const
{
    return "UnaryOp(" + op + ")";
}

// NumberNode
void NumberNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string NumberNode::toString() const
{
    return "Number(" + std::to_string(value) + ")";
}

// StringNode
void StringNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string StringNode::toString() const
{
    return "String(\"" + value + "\")";
}

// IdentifierNode
void IdentifierNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string IdentifierNode::toString() const
{
    return "Identifier(" + name + ")";
}

// BooleanNode
void BooleanNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string BooleanNode::toString() const
{
    return "Boolean(" + std::string(value ? "true" : "false") + ")";
}

// FunctionDeclNode
void FunctionDeclNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string FunctionDeclNode::toString() const { return "FunctionDecl(" + name + ")"; }

// CallNode
void CallNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string CallNode::toString() const { return "Call(" + name + ")"; }

// ReturnNode
void ReturnNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }
std::string ReturnNode::toString() const { return "Return"; }