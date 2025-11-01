#include "semantic_channels.h"
#include <unordered_map>

static void walk(ASTNode *node, std::unordered_map<std::string, ChannelArityInfo> &map)
{
    if (!node)
        return;
    if (auto prog = dynamic_cast<ProgramNode *>(node))
    {
        for (auto &s : prog->statements)
            walk(s.get(), map);
    }
    else if (auto seq = dynamic_cast<SeqNode *>(node))
    {
        for (auto &s : seq->statements)
            walk(s.get(), map);
    }
    else if (auto par = dynamic_cast<ParNode *>(node))
    {
        for (auto &s : par->statements)
            walk(s.get(), map);
    }
    else if (auto send = dynamic_cast<SendNode *>(node))
    {
        map[send->channelName].sendArities.push_back((int)send->arguments.size());
        map[send->channelName].sendComponents.push_back(send->component);
        for (auto &a : send->arguments)
            walk(a.get(), map);
    }
    else if (auto recv = dynamic_cast<ReceiveNode *>(node))
    {
        map[recv->channelName].recvArities.push_back((int)recv->variables.size());
        map[recv->channelName].recvComponents.push_back(recv->component);
    }
    else if (auto f = dynamic_cast<FunctionDeclNode *>(node))
    {
        walk(f->body.get(), map);
    }
    else if (auto w = dynamic_cast<WhileNode *>(node))
    {
        walk(w->condition.get(), map);
        walk(w->body.get(), map);
    }
    else if (auto ifn = dynamic_cast<IfNode *>(node))
    {
        walk(ifn->condition.get(), map);
        walk(ifn->thenBranch.get(), map);
        walk(ifn->elseBranch.get(), map);
    }
    else if (auto bin = dynamic_cast<BinaryOpNode *>(node))
    {
        walk(bin->left.get(), map);
        walk(bin->right.get(), map);
    }
    else if (auto un = dynamic_cast<UnaryOpNode *>(node))
    {
        walk(un->operand.get(), map);
    }
    // leaf nodes ignored
}

void analyze_channel_arities(ProgramNode *program, std::ostream &out)
{
    if (!program)
    {
        out << "<no program>\n";
        return;
    }
    std::unordered_map<std::string, ChannelArityInfo> info;
    walk(program, info);
    if (info.empty())
    {
        out << "Nenhum canal com operações send/receive.\n";
        return;
    }
    for (auto &entry : info)
    {
        const auto &name = entry.first;
        const auto &ci = entry.second;
        out << "Canal '" << name << "': sends=";
        if (ci.sendArities.empty())
            out << "-";
        else
        {
            for (size_t i = 0; i < ci.sendArities.size(); ++i)
            {
                if (i)
                    out << ",";
                out << ci.sendArities[i];
            }
        }
        out << " receives=";
        if (ci.recvArities.empty())
            out << "-";
        else
        {
            for (size_t i = 0; i < ci.recvArities.size(); ++i)
            {
                if (i)
                    out << ",";
                out << ci.recvArities[i];
            }
        }
        // Consistência
        bool sendCons = true;
        for (int v : ci.sendArities)
            if (v != ci.sendArities[0])
            {
                sendCons = false;
                break;
            }
        bool recvCons = true;
        for (int v : ci.recvArities)
            if (v != ci.recvArities[0])
            {
                recvCons = false;
                break;
            }
        bool match = (!ci.sendArities.empty() && !ci.recvArities.empty() && sendCons && recvCons && ci.sendArities[0] == ci.recvArities[0]);
        out << " comps_send=";
        if (ci.sendComponents.empty())
            out << "-";
        else
        {
            for (size_t i = 0; i < ci.sendComponents.size(); ++i)
            {
                if (i)
                    out << ",";
                out << ci.sendComponents[i];
            }
        }
        out << " comps_recv=";
        if (ci.recvComponents.empty())
            out << "-";
        else
        {
            for (size_t i = 0; i < ci.recvComponents.size(); ++i)
            {
                if (i)
                    out << ",";
                out << ci.recvComponents[i];
            }
        }
        out << " => ";
        if (match)
            out << "OK (arity=" << ci.sendArities[0] << ")";
        else
        {
            if (ci.sendArities.empty() || ci.recvArities.empty())
                out << "INCOMPLETO (faltando send ou receive)";
            else if (!sendCons || !recvCons)
                out << "INCONSISTENTE (variação nas aridades)";
            else
                out << "MISMATCH (send=" << ci.sendArities[0] << " recv=" << ci.recvArities[0] << ")";
        }
        out << "\n";
    }
}
