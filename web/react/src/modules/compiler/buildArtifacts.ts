import type { CompilationArtifacts } from '../layout/ResultsView';
import { Token } from '../layout/ResultsView';
import { SymbolEntry } from '../layout/ResultsView';

function extractSection(raw: string, start: string, end: string): string {
    const startIndex = raw.indexOf(start);
    const endIndex = raw.indexOf(end);

    if (startIndex === -1) return '';
    const section = raw.substring(startIndex + start.length, endIndex === -1 ? raw.length : endIndex);
    return section.trim();
}

// Extrai tokens
// buildArtifacts.tsx - função extractTokens atualizada
export function extractTokens(raw: string): Token[] {
    const tokenLines = extractSection(raw, '=== ANALISADOR LÉXICO ===', '=== ANALISADOR SINTÁTICO ===')
        .split('\n')
        .map(line => line.trim())
        .filter(line => line.startsWith('Token:'));

    // ✅ AGORA retorna Token[] em vez de string[]
    console.log("raw: ", raw);
    return tokenLines.map(line => {
        const tokenMatch = line.match(/Token:\s*(\d+)\s*Valor:\s*'([^']*)'\s*Linha:\s*(\d+)/);
        if (tokenMatch) {
            const [, typeNum, value, lineNum] = tokenMatch;

            // Converter o tipo numérico para string descritiva
            let type = 'UNKNOWN';
            const typeInt = parseInt(typeNum);

            // Mapeamento baseado nos tipos do seu lexer
            switch (typeInt) {
                case 0: type = 'KEYWORD'; break;      // SEQ
                case 4: type = 'KEYWORD'; break;      // while
                case 5: type = 'KEYWORD'; break;      // print
                case 11: type = 'IDENTIFIER'; break;  // n, fat, i, etc.
                case 12: type = 'NUMBER'; break;      // 5, 1, etc.
                case 14: type = 'OPERATOR'; break;    // +
                case 16: type = 'OPERATOR'; break;    // *
                case 18: type = 'OPERATOR'; break;    // =
                case 22: type = 'OPERATOR'; break;    // <=
                case 25: type = 'DELIMITER'; break;   // (
                case 26: type = 'DELIMITER'; break;   // )
                case 32: type = 'END'; break;         // fim
                default: type = 'UNKNOWN';
            }

            return {
                type,
                value: value,
                line: parseInt(lineNum),
                column: 1 // ✅ Coluna padrão para fallback
            };
        }

        // Fallback para linha mal formatada
        return {
            type: 'UNKNOWN',
            value: '',
            line: 1,
            column: 1
        };
    }).filter(token => token.value !== ''); // Filtrar tokens vazios
}

// Extrai árvore sintática
// buildArtifacts.tsx - adicionar função para fallback
export function extractSymbolTable(raw: string): SymbolEntry[] {
    const symbolSection = extractSection(raw, '=== TABELA DE SÍMBOLOS ===', '=== ÁRVORE SINTÁTICA ===');

    if (!symbolSection) return [];

    const lines = symbolSection.split('\n')
        .map(line => line.trim())
        .filter(line => line.length > 0 && !line.includes('---')); // Remover linhas de separador

    console.log('📊 Linhas da tabela de símbolos:', lines);

    const symbols: SymbolEntry[] = [];

    // Pular cabeçalho e processar linhas de dados
    for (let i = 2; i < lines.length; i++) { // Pular "Nome | TipoSimbolo | TipoDado" e "---"
        const line = lines[i];
        const parts = line.split('|').map(part => part.trim());

        if (parts.length >= 3) {
            symbols.push({
                name: parts[0] || '',
                symbolType: parts[1] || 'VAR', // No formato textual, assume VAR
                dataType: parts[2] || 'int',
                scope: 'global' // No formato textual, assume escopo global
            });
        }
    }

    console.log('📋 Símbolos extraídos:', symbols);
    return symbols;
}

// Extrai TAC
export function extractTAC(raw: string): string {
    return extractSection(raw, '=== CÓDIGO DE TRÊS ENDEREÇOS ===', '=== CÓDIGO ASSEMBLY ARMv7 ===');
}

// Extrai ARM
export function extractARM(raw: string): string {
    const armHeader = '=== CÓDIGO ASSEMBLY ARMv7 ===';
    const startIndex = raw.indexOf(armHeader);

    if (startIndex === -1) return '';

    const afterHeader = raw.substring(startIndex + armHeader.length);

    const nextHeaders = [
        '\n=== ', // Próxima seção
        '\n\n',   // Fim das seções
        '\n//',   // Comentários de debug
    ];

    let endIndex = afterHeader.length;
    for (const header of nextHeaders) {
        const idx = afterHeader.indexOf(header);
        if (idx !== -1 && idx < endIndex) {
            endIndex = idx;
        }
    }

    let armCode = afterHeader.substring(0, endIndex).trim();

    armCode = armCode.split('\n')
        .filter(line => {
            return !line.includes('DEBUG -') &&
                !line.includes('linhas ARM geradas:') &&
                !line.startsWith('// Código ARM via');
        })
        .join('\n')
        .trim();

    return armCode;
}

// ✅ NOVA FUNÇÃO: Extrai o output do programa
export function extractProgramOutput(raw: string): string {
    const outputHeader = '=== PROGRAM OUTPUT ===';
    const startIndex = raw.indexOf(outputHeader);

    if (startIndex === -1) return '';

    const afterHeader = raw.substring(startIndex + outputHeader.length);

    // Encontrar o próximo header ou fim do arquivo
    const nextHeaders = [
        '\n=== ', // Próxima seção (caso tenha mais coisas depois)
        '\n\n',   // Fim das seções
    ];

    let endIndex = afterHeader.length;
    for (const header of nextHeaders) {
        const idx = afterHeader.indexOf(header);
        if (idx !== -1 && idx < endIndex) {
            endIndex = idx;
        }
    }

    let output = afterHeader.substring(0, endIndex).trim();

    // Limpar possíveis linhas vazias no início/fim
    output = output.split('\n')
        .map(line => line.trim())
        .filter(line => line.length > 0)
        .join('\n');

    return output;
}

// buildArtifacts.tsx - na função buildArtifactsFromRaw
// buildArtifacts.tsx - na função buildArtifactsFromRaw
// buildArtifacts.tsx - na função buildArtifactsFromRaw
export function buildArtifactsFromRaw(code: string, raw: string): CompilationArtifacts {
    const jsonData = JSON.parse(raw); // Garantido que é JSON

    const tokens = jsonData.phases?.lexical?.tokens || [];
    const symbols = jsonData.phases?.semantic?.symbols || [];
    const tac = jsonData.phases?.intermediate?.tac || [];
    const armLines: string[] = jsonData.phases?.codegen?.code || [];

        const armArray = jsonData.phases?.codegen?.code || [];
        const arm = Array.isArray(armArray) ? armArray.join('\n') : '';

    return {
        rawOutput: raw,
        tokens,
        syntaxTree: jsonData.phases?.syntax?.ast || '',
        symbolTable: symbols,
        tac,
        arm,
        output: jsonData.execution?.output || ''
    };
}