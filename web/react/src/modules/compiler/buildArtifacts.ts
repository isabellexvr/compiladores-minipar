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

// buildArtifacts.ts - versão alternativa
export function extractARM(raw: string): string {
    // Buscar por "=== CÓDIGO ASSEMBLY ARMv7 ==="
    const armHeader = '=== CÓDIGO ASSEMBLY ARMv7 ===';
    const startIndex = raw.indexOf(armHeader);
    
    if (startIndex === -1) return '';
    
    // Pegar tudo depois do header
    const afterHeader = raw.substring(startIndex + armHeader.length);
    
    // Encontrar o próximo header ou fim do arquivo
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
    
    // Limpar linhas de debug
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

// buildArtifacts.ts - função atualizada
export function buildArtifactsFromRaw(code: string, raw: string): CompilationArtifacts {
    // Tenta parsear como JSON primeiro
    try {
        const jsonData = JSON.parse(raw);
        return {
            rawOutput: raw,
            tokens: jsonData.tokens ? jsonData.tokens.map((t: any) => t.value) : [],
            syntaxTree: jsonData.ast || '',
            symbolTable: jsonData.symbols ? JSON.stringify(jsonData.symbols, null, 2) : '',
            tac: jsonData.tac ? JSON.stringify(jsonData.tac, null, 2) : '', // TAC estruturado
            arm: jsonData.arm ? jsonData.arm.join('\n') : ''
        };
    } catch {
        // Fallback para o formato textual antigo
        return {
            rawOutput: raw,
            tokens: extractTokens(raw),
            syntaxTree: extractSyntaxTree(raw),
            symbolTable: extractSection(raw, '=== TABELA DE SÍMBOLOS ===', '=== ÁRVORE SINTÁTICA ==='),
            tac: extractTAC(raw),
            arm: extractARM(raw)
        };
    }
}