#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H
#include <sstream>
#include <vector>
#include "lexer.h"
// Constrói array de tokens em JSON. Se categorized=true usa categorias textuais, caso contrário enum numérico.
void build_tokens_array(std::stringstream &json, const std::vector<Token> &tokens, bool categorized);
#endif