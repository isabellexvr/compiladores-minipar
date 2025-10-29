import type { CompilationArtifacts } from '../layout/ResultsView';

function extractSection(raw: string, start: string, end: string): string {
    const startIndex = raw.indexOf(start);
    const endIndex = raw.indexOf(end);

    if (startIndex === -1) return '';
    const section = raw.substring(startIndex + start.length, endIndex === -1 ? raw.length : endIndex);
    return section.trim();
}

// Extrai tokens
export function extractTokens(raw: string): string[] {
    const tokenLines = extractSection(raw, '=== ANALISADOR LÉXICO ===', '=== ANALISADOR SINTÁTICO ===')
        .split('\n')
        .map(line => line.trim())
        .filter(line => line.startsWith('Token:'));

    return tokenLines.map(line => {
        const match = line.match(/Valor:\s*'([^']*)'/);
        return match ? match[1] : '';
    }).filter(Boolean);
}

// Extrai árvore sintática
export function extractSyntaxTree(raw: string): string {
    console.log("raw: ", raw);
    return extractSection(raw, '=== ÁRVORE SINTÁTICA ===', '=== CÓDIGO DE TRÊS ENDEREÇOS ===');
}

// Extrai TAC e ARM se quiser
export function extractTAC(raw: string): string {
    return extractSection(raw, '=== CÓDIGO DE TRÊS ENDEREÇOS ===', '=== CÓDIGO ASSEMBLY ARMv7 ===');
}

export function extractARM(raw: string): string {
    return extractSection(raw, '=== CÓDIGO ASSEMBLY ARMv7 ===', '');
}

export function buildArtifactsFromRaw(code: string, raw: string): CompilationArtifacts {
    return {
        rawOutput: raw,
        tokens: extractTokens(raw),
        syntaxTree: extractSyntaxTree(raw),
        symbolTable: 'Tabela de símbolos em desenvolvimento...', 
        tac: extractTAC(raw),
        arm: extractARM(raw)
    };
}
