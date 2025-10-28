import type { CompilationArtifacts } from '../layout/ResultsView';

// Função melhorada para extrair tokens com mais contexto
export function extractTokens(raw: string): string[] {
    const lines = raw.split('\n');
    const tokens: string[] = [];
    
    lines.forEach(line => {
        if (line.includes('===') && line.includes('TOKENS')) {
            // Pular linha de cabeçalho
            return;
        }
        
        // Extrair tokens da linha (heurística melhorada)
        const lineTokens = line
            .replace(/Token:\s*\d+\s*Valor:\s*'([^']*)'.*/g, '$1') // Extrair valor entre aspas
            .split(/\s+/)
            .map(t => t.trim())
            .filter(t => t && t !== '...' && !t.includes('mais') && !t.includes('tokens'));
        
        tokens.push(...lineTokens);
    });
    
    return tokens.filter(token => 
        token && 
        token !== 'Token:' && 
        !token.match(/^\d+$/) && // Remover números soltos (que são índices)
        token.length > 0
    );
}

export function buildArtifactsFromRaw(code: string, raw: string): CompilationArtifacts {
    return {
        rawOutput: raw,
        tokens: extractTokens(raw),
        syntaxTree: 'Árvore sintática em desenvolvimento...',
        symbolTable: 'Tabela de símbolos em desenvolvimento...', 
        tac: 'Código de três endereços em desenvolvimento...',
        arm: 'Código ARMv7 em desenvolvimento...'
    };
}