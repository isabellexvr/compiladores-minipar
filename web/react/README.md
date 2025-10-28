# MiniPar IDE (React)

Frontend moderno em React + Vite para o compilador MiniPar.

Scripts principais:

- `npm run dev` inicia servidor de desenvolvimento (http://localhost:5173)
- `npm run build` gera versão de produção em `dist/`
- `npm run preview` pré-visualiza build

Integração WebAssembly:
Coloque `compilador.js` e `compilador.wasm` dentro de `public/`. O carregamento ocorre antes do bootstrap do React em `index.html`.

Arquitetura:

- `src/modules/compiler/CompilerContext.tsx` provê contexto para chamadas ao módulo wasm
- `src/modules/layout/IDELayout.tsx` estrutura principal (editor + saída)
- `src/modules/ui/*` componentes de UI básicos

Melhorias futuras sugeridas:

1. Sintaxe highlight (Monaco ou CodeMirror)
2. Autocomplete baseado na symbol table
3. Exibição incremental de erros durante digitação
4. Download de código e resultado
5. Tema claro/escuro com toggle

Licença: Projeto acadêmico.
