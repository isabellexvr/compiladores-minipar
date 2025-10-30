# MiniPar 2025.1 – Compilador com TAC e ARMv7 (Web + Nativo)

> Tema 1: **Compilador (não Orientado a Objetos) MiniPar 2025.1** com **Geração de Código Intermediário (Código de Três Endereços)** e **Geração de Código Assembly ARMV7** para rodar o código Assembly obtido no Emulador na Web.

## 👥 Participantes
- Isabelle Xavier
- Davi Vieira
- José Milton

## 🎯 Visão Geral
MiniPar é um compilador educacional para uma linguagem imperativa mínima, projetado para demonstrar claramente cada fase da compilação: Léxico → Sintático (AST) → Tabela de Símbolos → Código Intermediário (TAC) → Código Assembly ARMv7. 
Ele roda tanto em modo nativo (CLI) quanto no navegador via **WebAssembly (Emscripten)**, acompanhado de uma interface em **React** que exibe artefatos de compilação.

## 🧩 Features Implementadas
- Lexer com suporte a palavras-chave: `SEQ`, `PAR`, `WHILE`, `PRINT`, `INPUT` etc.
- Tokens para operadores aritméticos e de comparação: `+ - * / == != < <= > >=`.
- Parser recursivo com nós de AST (atribuição, print, while, blocos sequenciais e paralelos básicos).
- Impressão da AST hierárquica (visita e formatação).
- Geração de **TAC (Three-Address Code)** incluindo temporários e controle de fluxo com `while` (labels, `if_false`, `goto`).
- Construção simplificada da **Tabela de Símbolos** a partir da AST.
- Geração inicial de **Assembly ARMv7** para expressões e `print`.
- Exportação dos artefatos para o frontend via funções Emscripten (`compile_minipar`, `compile_minipar_json`, `compile_minipar_tac`).
- Interface web com abas: Tokens, Syntax Tree, Symbol Table, TAC, ARM.

## 🏗 Arquitetura de Pastas
```
src/
  frontend/
    lexer/          → Analisador léxico
    parser/         → Parser + construção da AST
    ast/            → Nós e impressão da AST
    semantic/       → (futuro) verificações semânticas
  middleend/
    symbol_table/   → Tabela de símbolos
    tac/            → Gerador de TAC
  backend/
    arm/            → Geração de código ARMv7
    optimization/   → (futuro) otimizações
  runtime/
    channels/       → (futuro) primitivas de canal
    threads/        → (futuro) suporte a paralelismo
  emscripten_interface.cpp → Wrapper para WebAssembly
  main.cpp                → Entrada nativa (CLI)
web/react/                → Frontend em React + integração wasm
include/                  → Headers públicos
```
Fluxo resumido:
```
Código Fonte → Lexer → Tokens → Parser → AST → SymbolTable
          → TACGenerator → TAC → ARMGenerator → Código ARM
```

## 🔄 Fluxo de Compilação (Nativo)
```bash
make            # compila versão nativa (binário: compilador)
./compilador teste_simples.minipar
```
Saída inclui seções:
```
=== ANALISADOR LÉXICO ===
=== ANALISADOR SINTÁTICO ===
=== TABELA DE SÍMBOLOS ===
=== ÁRVORE SINTÁTICA ===
=== CÓDIGO DE TRÊS ENDEREÇOS ===
=== CÓDIGO ASSEMBLY ARMv7 ===
```

## 🌐 Execução no Navegador (Emscripten + React)
### Build Wasm
```bash
make -f Makefile.emscripten
```
Gera `compilador.wasm` + loader JS (MODULARIZE) usados pelo React em `web/react`. 

### Integração Emscripten
Funções exportadas (via `EXPORTED_FUNCTIONS`):
- `_compile_minipar` → Saída textual completa (todas as seções).
- `_compile_minipar_json` → Artefatos estruturados (tokens, TAC, ARM, símbolos).
- `_compile_minipar_tac` → Apenas TAC em JSON (com classificação de instruções).
- `_free_string` → Liberação de memória alocada do lado C++.

Uso no frontend (simplificado):
```ts
const compileFn = module.cwrap('compile_minipar', 'number', ['string']);
const ptr = compileFn(source);
const text = module.UTF8ToString(ptr);
module._free_string(ptr);
```

### Frontend React
- Carrega o módulo wasm assíncrono (`createCompilerModule`).
- Após compilação, extrai seções do texto bruto ou usa JSON estruturado.
- Apresenta cada artefato em uma aba (estilo terminal escuro com paleta definida).

## 📦 Código Intermediário (TAC)
Exemplo gerado:
```
x = 10
y = 20
t0 = x + y
resultado = t0
print resultado
```
Controle de fluxo (`while`):
```
L0:
if_false t0 goto L1
... corpo ...
goto L0
L1:
```
Cada instrução possui classificação (`ASSIGNMENT`, `BINARY_OP`, `PRINT`, `LABEL`, `CONDITIONAL_JUMP`, `JUMP`) para facilitar visualização no UI.

## 🗂 Tabela de Símbolos
Construída por travessia simples da AST. Registra identificadores como `VAR` com tipo inferido básico (placeholder `int`). Futuras expansões: escopos, canais, funções, tipos compostos.

## ⚙️ Geração de ARMv7
Protótipo converte operações aritméticas simples e impressão (`printf`). Exemplo:
```
mov r0, #10
mov r1, #20
add r2, r0, r1
mov r3, r2
push {r0, r1}
ldr r0, =msg
mov r1, r3
bl printf
pop {r0, r1}
mov r7, #1
svc #0
```
Roadmap inclui: comparações, saltos condicionais completos, convenções de chamada, suporte a múltiplas funções.

## 🛠 Tecnologias
- C++17 (núcleo do compilador)
- Emscripten (gera Wasm + wrapper JS modular)
- React + TypeScript (visualização de artefatos)
- ARMv7 assembly (saída final executável em emuladores/Web)

## 🚧 Roadmap / Próximos Passos
- [ ] Suporte completo a `IF` / `ELSE` e blocos `{}`
- [ ] Semântica real de `PAR` (threads / channels)
- [ ] Operações de canal (`send` / `receive`)
- [ ] Escopos léxicos na Symbol Table
- [ ] Inferência / verificação de tipos
- [ ] Otimizações básicas de TAC (propagação de constantes, dead code)
- [ ] Mais instruções ARM (comparações, branching condicional)
- [ ] Testes automatizados (unitários para lexer/parser/TAC)
- [ ] Modo demo interativo com animação de fluxo

## 📚 Estrutura da Linguagem (Simplificada)
```
Program       → (SEQ | PAR | Statement)* EOF
Statement     → Assignment | Print | Input | While
Assignment    → IDENTIFIER '=' Expression ';'? (o ';' ainda opcional)
Print         → 'print' Expression ';'? 
While         → 'while' '(' Expression ')' Statement*
Expression    → Term (( '+' | '-' ) Term)* (comparações encadeadas)
Term          → Factor (( '*' | '/' ) Factor)*
Factor        → NUMBER | IDENTIFIER | '(' Expression ')'
```

## 🧪 Teste Rápido
Entrada:
```
SEQ
x = 10
y = 20
resultado = x + y
print resultado
```
Saída (trecho TAC):
```
x = 10
y = 20
t0 = x + y
resultado = t0
print resultado
```

## 🤝 Contribuição
1. Fork + branch (`feature/nova-coisa`).
2. `make` para validar build nativo.
3. `make -f Makefile.emscripten` para garantir sincronização wasm.
4. Pull Request com descrição clara.

## 💬 Contato
Sugestões / dúvidas: abrir Issue ou falar com os participantes.

---
Educacional. Focado em clareza de fases e visualização dos artefatos. Expandir gradualmente mantendo separação limpa entre frontend, middleend e backend.
