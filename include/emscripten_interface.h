#ifndef EMSCRIPTEN_INTERFACE_H
#define EMSCRIPTEN_INTERFACE_H

#include <string>

// Macros para Emscripten - definidas vazias para desenvolvimento local
#ifndef EMSCRIPTEN_KEEPALIVE
    #define EMSCRIPTEN_KEEPALIVE
#endif

#ifndef EMSCRIPTEN
    // Definições locais para desenvolvimento
    #include <iostream>
    #include <cstring>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Funções exportadas para JavaScript
EMSCRIPTEN_KEEPALIVE
char* compile_minipar(const char* source_code);

EMSCRIPTEN_KEEPALIVE
char* compile_minipar_json(const char* source_code);

EMSCRIPTEN_KEEPALIVE
void free_string(char* str);

#ifdef __cplusplus
}
#endif

#endif