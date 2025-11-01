#include "parser.h"
#include <iostream>
#include <sstream>

using namespace std;

Parser::Parser(const vector<Token> &tokens)
    : tokens(tokens), current_token(0), currentComponent("") {}

Token &Parser::current()
{
    return tokens[current_token];
}

Token &Parser::peek()
{
    static Token eof(TokenType::END, "");
    if (current_token + 1 < tokens.size())
    {
        return tokens[current_token + 1];
    }
    return eof;
}

void Parser::consume()
{
    if (current_token < tokens.size() &&
        tokens[current_token].type != TokenType::END)
    {
        current_token++;
    }
}

bool Parser::match(TokenType type)
{
    return current().type == type;
}

void Parser::setComponent(const std::string &name)
{
    currentComponent = name;
}

unique_ptr<ProgramNode> Parser::parse()
{
    auto program = make_unique<ProgramNode>();

    while (!match(END))
    {
        if (match(COMP))
        {
            consume();
            if (match(IDENTIFIER))
            {
                setComponent(current().value);
                consume();
            }
            continue;
        }
        if (match(C_CHANNEL))
        {
            // c_channel name comp1 comp2
            consume();
            if (match(IDENTIFIER))
            {
                std::string chName = current().value;
                consume();
                std::string c1 = match(IDENTIFIER) ? current().value : "";
                if (match(IDENTIFIER))
                    consume();
                std::string c2 = match(IDENTIFIER) ? current().value : "";
                if (match(IDENTIFIER))
                    consume();
                auto chDecl = make_unique<ChannelDeclNode>();
                chDecl->name = chName;
                chDecl->comp1 = c1;
                chDecl->comp2 = c2;
                program->statements.push_back(std::move(chDecl));
                continue;
            }
        }
        else if (match(FUN))
        {
            consume(); // fun
            if (!match(IDENTIFIER))
            {
                consume();
                continue;
            }
            std::string fname = current().value;
            consume();
            std::vector<std::string> params;
            if (match(LPAREN))
            {
                consume();
                if (!match(RPAREN))
                {
                    while (true)
                    {
                        if (match(IDENTIFIER))
                        {
                            params.push_back(current().value);
                            consume();
                        }
                        if (match(COMMA))
                        {
                            consume();
                            continue;
                        }
                        break;
                    }
                }
                if (match(RPAREN))
                    consume();
            }
            // body: exigir '{' para funções; parse até '}' exclusivo
            std::unique_ptr<ASTNode> bodyNode;
            if (match(LBRACE))
            {
                consume(); // '{'
                auto seq = make_unique<SeqNode>();
                while (!match(RBRACE) && !match(END))
                {
                    if (match(SEQ))
                    {
                        // Bloco SEQ interno dentro da função
                        auto inner = parse_seq_block();
                        for (auto &s : inner->statements)
                            seq->statements.push_back(std::move(s));
                        continue;
                    }
                    if (match(RBRACE) || match(END))
                        break;
                    auto st = parse_statement();
                    if (st)
                    {
                        seq->statements.push_back(std::move(st));
                        continue;
                    }
                    // token desconhecido dentro da função: avançar
                    if (!match(RBRACE) && !match(END))
                        consume();
                }
                if (match(RBRACE))
                    consume(); // consumir '}' fechamento da função
                bodyNode = std::move(seq);
            }
            else
            {
                // Função sem '{' trata próxima statement única como corpo
                bodyNode = parse_statement();
            }
            auto fdecl = make_unique<FunctionDeclNode>();
            fdecl->name = fname;
            fdecl->params = params;
            fdecl->body = std::move(bodyNode);
            program->statements.push_back(std::move(fdecl));
            continue;
        }
        else if (match(SEQ))
        {
            // SEQ block
            auto seq = parse_seq_block();
            program->statements.push_back(std::move(seq));
        }
        else if (match(PAR))
        {
            // PAR block
            consume(); // consumir PAR
            auto par = make_unique<ParNode>();

            // Parse todos os SEQ dentro do PAR
            while (match(SEQ))
            {
                auto seq = parse_seq_block();
                par->statements.push_back(std::move(seq));
            }

            program->statements.push_back(std::move(par));
        }
        else
        {
            auto stmt = parse_statement();
            if (stmt)
            {
                program->statements.push_back(std::move(stmt));
            }
            else
            {
                consume(); // pular token desconhecido
            }
        }
    }

    return program;
}

unique_ptr<SeqNode> Parser::parse_seq_block()
{
    consume(); // consumir 'SEQ'
    auto seq = make_unique<SeqNode>();
    bool hasBrace = false;
    if (match(LBRACE))
    {
        consume();
        hasBrace = true;
    }

    // Loop de coleta de statements dentro do bloco SEQ.
    // Caso haja '{', continuamos até '}' correspondente.
    // Caso não haja '{', paramos ao encontrar início de outro bloco estrutural ou fim.
    while (!match(END))
    {
        if (hasBrace && match(RBRACE))
            break; // fim do bloco com chaves
        if (!hasBrace && (match(SEQ) || match(PAR) || match(ELSE)))
            break; // início de outro bloco estrutural sem chaves
        // Se estivermos dentro de uma função (heurística: próximo token é '}' seguido de possível SEQ principal), parse_seq_block não deve atravessar '}'
        if (!hasBrace && match(RBRACE))
            break;

        auto stmt = parse_statement();
        if (stmt)
        {
            seq->statements.push_back(std::move(stmt));
            continue;
        }
        // Se não reconheceu a statement mas ainda não é fim de bloco, consumir token para evitar loop infinito.
        if (!match(END))
        {
            // Se encontrar '}', sair (caso tenha sido não tratado como stmt)
            if (match(RBRACE))
                break;
            consume();
        }
    }

    if (hasBrace && match(RBRACE))
        consume(); // consumir '}' de fechamento

    return seq;
}

unique_ptr<ASTNode> Parser::parse_statement()
{
    // Operações de canal: canal.send(...); canal.receive(...);
    if (match(TokenType::IDENTIFIER) && peek().type == TokenType::DOT)
    {
        std::string channelName = current().value;
        consume(); // canal
        if (match(TokenType::DOT))
            consume();
        if (match(TokenType::IDENTIFIER))
        {
            std::string opName = current().value;
            consume();
            if (opName == "send")
            {
                auto sendNode = make_unique<SendNode>();
                sendNode->channelName = channelName;
                sendNode->component = currentComponent;
                if (match(LPAREN))
                {
                    consume();
                    if (!match(RPAREN))
                    {
                        while (true)
                        {
                            auto arg = parse_expression();
                            if (arg)
                                sendNode->arguments.push_back(std::move(arg));
                            if (match(COMMA))
                            {
                                consume();
                                continue;
                            }
                            break;
                        }
                    }
                    if (match(RPAREN))
                        consume();
                }
                if (match(SEMICOLON))
                    consume();
                return sendNode;
            }
            else if (opName == "receive")
            {
                auto recvNode = make_unique<ReceiveNode>();
                recvNode->channelName = channelName;
                recvNode->component = currentComponent;
                if (match(LPAREN))
                {
                    consume();
                    if (!match(RPAREN))
                    {
                        while (true)
                        {
                            if (match(IDENTIFIER))
                            {
                                recvNode->variables.push_back(current().value);
                                consume();
                            }
                            if (match(COMMA))
                            {
                                consume();
                                continue;
                            }
                            break;
                        }
                    }
                    if (match(RPAREN))
                        consume();
                }
                if (match(SEMICOLON))
                    consume();
                return recvNode;
            }
        }
    }
    // Array assignment: IDENT '[' expr ']' '=' expr
    if (match(TokenType::IDENTIFIER) && peek().type == TokenType::LBRACKET)
    {
        // lookahead to see if pattern IDENT '[' ... ']' '='
        size_t save = current_token;
        std::string baseName = current().value;
        consume(); // IDENT
        if (match(LBRACKET))
        {
            consume();
            auto idxExpr = parse_expression();
            if (match(RBRACKET))
            {
                consume();
                if (match(ASSIGN))
                {
                    consume();
                    auto valExpr = parse_expression();
                    if (match(SEMICOLON))
                        consume();
                    auto arrAssign = make_unique<ArrayAssignmentNode>();
                    auto idNode = make_unique<IdentifierNode>();
                    idNode->name = baseName;
                    arrAssign->array = std::move(idNode);
                    arrAssign->index = std::move(idxExpr);
                    arrAssign->value = std::move(valExpr);
                    return arrAssign;
                }
            }
        }
        // rollback if not proper pattern
        current_token = save;
    }
    if (match(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN)
    {
        // Assignment
        string identifier = current().value;
        consume(); // identifier
        consume(); // '='

        auto assignment = make_unique<AssignmentNode>();
        assignment->identifier = identifier;
        assignment->expression = parse_expression();

        if (match(TokenType::SEMICOLON))
        {
            consume();
        }

        return assignment;
    }
    else if (match(TokenType::PRINT))
    {
        consume();
        auto print_node = make_unique<PrintNode>();
        auto first = parse_expression();
        if (first)
            print_node->expressions.push_back(std::move(first));
        while (match(TokenType::COMMA))
        {
            consume();
            auto more = parse_expression();
            if (more)
                print_node->expressions.push_back(std::move(more));
            else
                break;
        }
        if (match(TokenType::SEMICOLON))
            consume();
        return print_node;
    }
    else if (match(RETURN))
    {
        consume();
        auto ret = make_unique<ReturnNode>();
        if (!match(SEMICOLON))
            ret->value = parse_expression();
        if (match(SEMICOLON))
            consume();
        return ret;
    }
    else if (match(TokenType::INPUT))
    {
        // Input
        consume();

        if (match(TokenType::IDENTIFIER))
        {
            auto input_node = make_unique<InputNode>();
            input_node->identifier = current().value;
            consume();

            if (match(TokenType::SEMICOLON))
            {
                consume();
            }

            return input_node;
        }
    }
    else if (match(TokenType::WHILE))
    {
        return parse_while_statement();
    }
    else if (match(IF))
    {
        return parse_if_statement();
    }

    // Se não reconhecer, pular token
    consume();
    return nullptr;
}

unique_ptr<ASTNode> Parser::parse_comparison()
{
    auto left = parse_term();

    while (match(LESS_EQUAL) || match(LESS) || match(GREATER) || match(GREATER_EQUAL) ||
           match(EQUAL) || match(NOT_EQUAL))
    {
        TokenType op = current().type;
        consume();

        auto right = parse_term();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);

        left = std::move(bin_op);
    }

    return left;
}

unique_ptr<ASTNode> Parser::parse_expression()
{
    // lógica: comparação encadeada por AND / OR
    auto left = parse_comparison();
    while (match(AND) || match(OR))
    {
        TokenType op = current().type;
        consume();
        auto right = parse_comparison();
        auto bin = make_unique<BinaryOpNode>();
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }
    return left;
    // REMOVER o código antigo abaixo:
    /*
    auto left = parse_term();

    while (match(PLUS) || match(MINUS)) {
        TokenType op = current().type;
        consume();

        auto right = parse_term();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);

        left = std::move(bin_op);
    }

    return left;
    */
}

unique_ptr<ASTNode> Parser::parse_term()
{
    auto left = parse_factor();

    while (match(PLUS) || match(MINUS))
    { // + e - vão AQUI no term
        TokenType op = current().type;
        consume();

        auto right = parse_factor();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);

        left = std::move(bin_op);
    }

    return left;
}

unique_ptr<ASTNode> Parser::parse_factor()
{
    auto left = parse_primary();

    while (match(MULTIPLY) || match(DIVIDE))
    { // * e / vão AQUI no factor
        TokenType op = current().type;
        consume();

        auto right = parse_primary();
        auto bin_op = make_unique<BinaryOpNode>();
        bin_op->op = op;
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);

        left = std::move(bin_op);
    }

    return left;
}

unique_ptr<ASTNode> Parser::parse_primary()
{
    // unary minus support: if leading '-' followed by number/float/identifier/paren
    if (match(MINUS))
    {
        consume();
        auto inner = parse_primary();
        if (inner)
        {
            auto un = make_unique<UnaryOpNode>();
            un->op = "-";
            un->operand = std::move(inner);
            return un;
        }
    }
    if (match(NUMBER))
    {
        auto num_node = make_unique<NumberNode>();
        stringstream ss(current().value);
        ss >> num_node->value;
        consume();
        return num_node;
    }
    else if (match(FLOAT))
    {
        auto f_node = make_unique<FloatNode>();
        stringstream ss(current().value);
        ss >> f_node->value;
        consume();
        return f_node;
    }
    else if (match(STRING_LITERAL))
    {
        auto str_node = make_unique<StringNode>();
        str_node->value = current().value;
        consume();
        return str_node;
    }
    else if (match(TRUE) || match(FALSE))
    {
        auto bool_node = make_unique<BooleanNode>();
        bool_node->value = match(TRUE);
        consume();
        return bool_node;
    }
    else if (match(BOOL))
    { // treat 'bool' literal? skip
    }
    else if (match(NOT))
    {
        consume();
        auto operand = parse_primary();
        auto un = make_unique<UnaryOpNode>();
        un->op = "!";
        un->operand = std::move(operand);
        return un;
    }
    else if (match(IDENTIFIER))
    {
        // Could be function call: IDENTIFIER '(' args ')' OR plain identifier
        std::string name = current().value;
        consume();
        if (match(LPAREN))
        {
            consume();
            auto call = make_unique<CallNode>();
            call->name = name;
            if (!match(RPAREN))
            {
                while (true)
                {
                    auto arg = parse_expression();
                    if (arg)
                        call->args.push_back(std::move(arg));
                    if (match(COMMA))
                    {
                        consume();
                        continue;
                    }
                    break;
                }
            }
            if (match(RPAREN))
                consume();
            // potential chained array access after call result not supported; return call directly for now
            return call;
        }
        std::unique_ptr<ASTNode> base;
        auto id_node = make_unique<IdentifierNode>();
        id_node->name = name;
        base = std::move(id_node);
        // handle chained array access: identifier '[' expression ']'
        while (match(LBRACKET))
        {
            consume(); // [
            auto idxExpr = parse_expression();
            if (match(RBRACKET))
                consume();
            auto access = make_unique<ArrayAccessNode>();
            access->base = std::move(base);
            access->index = std::move(idxExpr);
            base = std::move(access);
        }
        return base;
    }
    else if (match(LPAREN))
    {
        consume(); // Consumir '('
        auto expr = parse_expression();
        if (match(RPAREN))
        {
            consume(); // Consumir ')'
        }
        // after parenthesized expression allow array access chaining e.g. (arr)[i]
        std::unique_ptr<ASTNode> base = std::move(expr);
        while (match(LBRACKET))
        {
            consume();
            auto idxExpr = parse_expression();
            if (match(RBRACKET))
                consume();
            auto access = make_unique<ArrayAccessNode>();
            access->base = std::move(base);
            access->index = std::move(idxExpr);
            base = std::move(access);
        }
        return base;
    }
    else if (match(LBRACKET))
    {
        consume(); // [
        auto arr = make_unique<ArrayLiteralNode>();
        if (!match(RBRACKET))
        {
            while (true)
            {
                auto elem = parse_expression();
                if (elem)
                    arr->elements.push_back(std::move(elem));
                if (match(COMMA))
                {
                    consume();
                    continue;
                }
                break;
            }
        }
        if (match(RBRACKET))
            consume();
        // array literal can be followed by access e.g. [1,2,3][0]
        std::unique_ptr<ASTNode> base = std::move(arr);
        while (match(LBRACKET))
        {
            consume();
            auto idxExpr = parse_expression();
            if (match(RBRACKET))
                consume();
            auto access = make_unique<ArrayAccessNode>();
            access->base = std::move(base);
            access->index = std::move(idxExpr);
            base = std::move(access);
        }
        return base;
    }

    // Erro - retornar nullptr
    consume();
    return nullptr;
}

unique_ptr<ASTNode> Parser::parse_while_statement()
{
    consume(); // consumir 'while'

    auto while_node = make_unique<WhileNode>();

    // Consumir '('
    if (match(LPAREN))
    {
        consume();
    }

    // Parse condition (expressão completa)
    while_node->condition = parse_expression();

    // Consumir ')'
    if (match(RPAREN))
    {
        consume();
    }

    // Novo corpo: se próximo token for SEQ, usar bloco completo
    if (match(SEQ))
    {
        while_node->body = parse_seq_block();
    }
    else
    {
        auto body_seq = make_unique<SeqNode>();
        // fallback antigo limitado
        for (int i = 0; i < 2 && !match(END) && !match(SEQ) && !match(PAR); i++)
        {
            auto stmt = parse_statement();
            if (stmt)
                body_seq->statements.push_back(std::move(stmt));
            else
                break;
        }
        while_node->body = std::move(body_seq);
    }
    return while_node;
}

unique_ptr<ASTNode> Parser::parse_if_statement()
{
    consume(); // IF
    auto ifNode = make_unique<IfNode>();
    // opcional parênteses
    if (match(LPAREN))
        consume();
    ifNode->condition = parse_expression();
    if (match(RPAREN))
        consume();
    // then branch: se próximo for SEQ consumir bloco, senão única instrução
    if (match(SEQ))
    {
        ifNode->thenBranch = parse_seq_block();
    }
    else
    {
        ifNode->thenBranch = parse_statement();
    }
    // else opcional
    if (match(ELSE))
    {
        consume();
        if (match(SEQ))
            ifNode->elseBranch = parse_seq_block();
        else
            ifNode->elseBranch = parse_statement();
    }
    return ifNode;
}