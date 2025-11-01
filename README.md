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

## 🧩 Features Implementadas

Léxico / Sintático

- Palavras‑chave: `seq`, `par`, `while`, `if`, `else`, `print`, `input`, `fun`, `return`, `true`, `false`, `c_channel`, tipos básicos (`int`, `bool`, `string`).
### Palavras‑chave Reconhecidas (Lexer)
Lista exata das keywords mapeadas no lexer (case‑insensitive):
`seq`, `par`, `if`, `else`, `while`, `print`, `input`, `fun`, `return`, `true`, `false`, `comp`, `int`, `bool`, `string`, `c_channel`.

Observação: o lexer converte para minúsculas; identificadores não coincidentes permanecem como `IDENTIFIER`.
- Literais: inteiros, floats (`d+.d+`), strings com escape de aspas (`"`), booleanos, arrays literais (`[1, 2, 3]`, aninhados `[[1,2],[3,4]]`).
- Operadores: aritméticos `+ - * /`, comparação `== != < <= > >=`, lógicos `&& || !`, unário `-`.
- Identificadores case‑insensitive para palavras‑chave (normalização para minúsculas no lexer).

AST / Linguagem

- Atribuição, múltiplos `print` na mesma linha (separados e `print_last` no final), `while`, `if / else`, blocos `SEQ { ... }` e listas após `SEQ` sem chaves, bloco paralelo `PAR` (placeholder sem semântica de concorrência ainda), funções (`fun nome(params){ ... }`) com `return` explícito ou implícito.
- Arrays heterogêneos (mistura de ints, floats, strings e sub‑arrays) com acesso encadeado `matriz[i][j]` e atribuição de elemento `arr[i] = valor`.
- Declaração de canais: `c_channel nome compA compB` e primitivas `canal.send(expr1, expr2, ...)` / `canal.receive(a, b, ...)` já produzindo TAC (execução ainda simulada heurísticamente no interpretador).

Código Intermediário (TAC)

- Instruções suportadas (além das aritméticas / controle): `array_init`, `array_set`, `array_get`, `array_concat`, `call`, `return`, `param`, `send`, `send_arg`, `receive`, `recv_arg`, `print`, `print_last`.
- Heurística de detecção de concatenação de arrays via `+` entre resultados de construções de array gera `array_concat` para visualização explícita.
- Estrutura de funções gera label de entrada, parâmetros (`param`), corpo, label determinístico de retorno e instrução `return` final consolidada.

Tabela de Símbolos

- Registra variáveis, funções e tenta inferir tipo básico (`int`, `float`, `string`, `bool`) a partir das expressões / literais encontrados (placeholder para futura análise semântica aprofundada e escopos léxicos).

Interpretador de TAC (Runtime Educacional)

- Executa instruções TAC imprimindo saída, avaliando expressões, loops, condicionais, funções (pilha de chamadas), arrays (numéricos, strings, aninhados) e concatenação.
- Simulação parcial de canais: acumula mensagens em filas por canal (`send` / `receive`) e faz binding dos valores recebidos às variáveis listadas (com pequena heurística de operação exemplo).

Strings & Arrays

- Armazenamento paralelo de valores numéricos e strings por índice em arrays; preserva referências de sub‑arrays para permitir acesso multidimensional e impressão legível.

Emscripten / Web

- Exportação das funções: `_compile_minipar` (texto completo), `_compile_minipar_json` (artefatos estruturados em JSON: tokens categorizados, AST, tabela, TAC, ARM), `_compile_minipar_tac` (somente TAC JSON), `_free_string` (liberação de memória alocada).
- JSON de tokens inclui versão “única” categorizada para reduzir ruído.

Debug & DX

- Macro condicional `MINIPAR_DEBUG` silencia logs de depuração de parser, gerador de TAC e interpretador por padrão (ativar com `CXXFLAGS+=-DMINIPAR_DEBUG`).
- Makefile para build nativo e Makefile.emscripten alinhados (incluindo subdiretórios de middleend/runtime).

Frontend React

- Abas: Tokens, Syntax Tree (AST), Symbol Table, TAC (com classificação de instruções), ARM, além de visualização textual consolidada.

```
=== ANALISADOR SINTÁTICO ===
=== TABELA DE SÍMBOLOS ===
## 📦 Código Intermediário (TAC)
Exemplos (anotados):
```

// Aritmética & atribuição
t0 = 10
t1 = 20
t2 = t0 + t1
resultado = t2

// While
L0:
if_false t3 goto L1
... corpo ...
goto L0
L1:

// If / Else
if_false t4 goto L_else
... then ...
goto L_end
L_else:
... else ...
L_end:

// Função
goto L_afterFuncs
f:
param a
param b
... corpo ...
L_return_f:
return retval
L_afterFuncs:

// Arrays
t5 = array_init 3
t5[0] = t0
t5[1] = t1
t5[2] = t2
t6 = array_get t5[1]
t7 = concat t5, t8 // via array_concat

// Canais (exemplo)
send canal count=2
canal[0] <= t9 // send_arg
canal[1] <= t10
receive canal count=2
x = recv canal[0] // recv_arg
y = recv canal[1]

```
Principais instruções: `label`, `goto`, `if_false`, `=`, operadores binários, `print` / `print_last`, `array_init`, `array_set`, `array_get`, `array_concat`, `call`, `param`, `return`, `send`, `send_arg`, `receive`, `recv_arg`.

Classificação exibida na UI auxilia em filtros e coloração (ex.: controle de fluxo, operação binária, array, chamada, E/S de canal).
### Integração Emscripten
## 📚 Estrutura da Linguagem (Atualizada e Simplificada)
```

Program → (ComponentDecl | ChannelDecl | FunctionDecl | Block | Statement)_ EOF
ComponentDecl → 'comp' IDENT
ChannelDecl → 'c_channel' IDENT IDENT IDENT
FunctionDecl → 'fun' IDENT '(' ParamList? ')' ( '{' BlockItems '}' | Statement )
ParamList → IDENT (',' IDENT)_
Block → 'SEQ' ('{' BlockItems '}' | BlockItemsNoBrace)
ParallelBlock → 'PAR' ( 'SEQ' ... )+
BlockItems → (Statement | Block | If | While)_
Statement → Assignment | Print | Input | While | If | Return | Call | ArrayAssignment | ChannelSend | ChannelReceive
Assignment → IDENT '=' Expression ';'?
ArrayAssignment→ IDENT '[' Expression ']' '=' Expression ';'?
Print → 'print' Expression (',' Expression)_ ';'?
Input → 'input' IDENT ';'?
While → 'while' '(' Expression ')' (Block | Statement)
If → 'if' '('? Expression ')'?(Block | Statement) ('else' (Block | Statement))?
Return → 'return' Expression? ';'?
Call → IDENT '(' ArgList? ')' ';'?
ChannelSend → IDENT '.' 'send' '(' ArgList? ')' ';'?
ChannelReceive → IDENT '.' 'receive' '(' IdentList? ')' ';'?
ArgList → Expression (',' Expression)_
IdentList → IDENT (',' IDENT)_
Expression → OrExpr
OrExpr → AndExpr ( '||' AndExpr )_
AndExpr → CmpExpr ( '&&' CmpExpr )_
CmpExpr → AddExpr ( ( '==' | '!=' | '<' | '<=' | '>' | '>=' ) AddExpr )_
AddExpr → MulExpr ( ( '+' | '-' ) MulExpr )_
MulExpr → Unary ( ( '_' | '/' ) Unary )_
Unary → ( '!' | '-' ) Unary | Primary
Primary → NUMBER | FLOAT | STRING_LITERAL | 'true' | 'false' | IDENT ( CallArgs | ArrayAccessChain )? | ArrayLiteral | '(' Expression ')' ArrayAccessChain?
ArrayLiteral → '[' (Expression (',' Expression)*)? ']'
ArrayAccessChain → ('[' Expression ']')\*

```

Observações:
- Palavras‑chave em minúsculas; identificadores de usuário preservam caixa original apenas para exibição.
- `;` ainda opcional em muitos finais de instrução (tolerante para experimentação em ambiente educacional).
- Concatenação de arrays por `+` detectada quando ambos operandos são construções de array (heurística atual).
const text = module.UTF8ToString(ptr);
- Carrega o módulo wasm assíncrono (`createCompilerModule`).
## 🧪 Teste Rápido
Entrada:
```

SEQ {
x = 10
y = 20
arr = [x, y, 30]
z = (x + y) \* 2
print x, y, z
}

```
Trecho TAC resultante (resumido):
```

t0 = 10
t1 = 20
arr = array_init 3
arr[0] = t0
arr[1] = t1
arr[2] = t2
t3 = arr[1]
t4 = t0 + t1
t5 = t4 \* t6
print t0
print t1
print_last t5

```
Execução imprime: `10 20 <valor>`.

Ative logs de depuração (opcional) adicionando a flag:
```

make CXXFLAGS="-DMINIPAR_DEBUG -O0 -g"

```

```

## 🚧 Roadmap / Próximos Passos

- [x] `IF / ELSE` básico
- [x] Funções + retorno
- [x] Arrays heterogêneos e aninhados
- [x] Strings literais & booleans
- [x] Concatenação de arrays (heurística)
- [x] Operações de canal (parser + TAC stub)
- [ ] Semântica de execução real para `PAR` (threads / scheduling) & canais bloqueantes
- [ ] Escopos léxicos / shadowing / ambientes por função
- [ ] Tipagem mais forte e inferência estática (numérica vs string vs array)
- [ ] Otimizações TAC (propagação de constantes, DCE, folding, eliminação de temporários redundantes)
- [ ] Expansão ARM: comparações, saltos condicionais, chamadas de função (stack frame), arrays
- [ ] Testes automatizados (lexer, parser, TAC, interpreter) e CI
- [ ] Visualização gráfica de fluxo (CFG) e animação de execução
- [ ] Erros léxicos/sintáticos estruturados (diagnostics JSON)
- [ ] Token de erro explícito para caracteres desconhecidos
      Educacional. Focado em clareza de fases e visualização dos artefatos. Expandir gradualmente mantendo separação limpa entre frontend, middleend e backend. Para novas contribuições priorize: (1) correção / cobertura de testes, (2) semântica de canais / PAR, (3) otimizações, (4) estender backend ARM.

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
