#ifndef SEMANTIC_CHANNELS_H
#define SEMANTIC_CHANNELS_H

#include "ast_nodes.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <ostream>

struct ChannelArityInfo
{
    std::vector<int> sendArities;
    std::vector<int> recvArities;
};

// Analisa a AST coletando aridades de send/receive por canal e reporta inconsistências.
void analyze_channel_arities(ProgramNode *program, std::ostream &out);

#endif
