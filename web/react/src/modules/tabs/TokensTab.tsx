import React from 'react';
import './TokensTab.css';

interface Token {
    type: string;
    value: string;
    line?: number;
    column?: number;
}

interface TokensTabProps {
    tokens: string[];
    sourceCode?: string;
}

const TokensTab: React.FC<TokensTabProps> = ({ tokens, sourceCode }) => {
    const extractTokensFromSource = (code: string): Token[] => {
        const lines = code.split('\n');
        const extracted: Token[] = [];
        let lineNumber = 1;

        lines.forEach(line => {
            const words = line
                .split(/(\s+|\(|\)|;|,|\+|\-|\*|\/|=)/)
                .filter(word => word.trim().length > 0);

            words.forEach(word => {
                const trimmed = word.trim();
                if (!trimmed) return;

                let type = 'UNKNOWN';
                
                if (['SEQ', 'PAR', 'print', 'if', 'else', 'while'].includes(trimmed)) {
                    type = 'KEYWORD';
                } else if (/^\d+$/.test(trimmed)) {
                    type = 'NUMBER';
                } else if (['+', '-', '*', '/'].includes(trimmed)) {
                    type = 'OPERATOR';
                } else if (trimmed === '=') {
                    type = 'ASSIGN';
                } else if (/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(trimmed)) {
                    type = 'IDENTIFIER';
                } else if (['(', ')', '{', '}', ';', ','].includes(trimmed)) {
                    type = 'DELIMITER';
                }

                extracted.push({
                    type,
                    value: trimmed,
                    line: lineNumber
                });
            });
            
            lineNumber++;
        });

        return extracted;
    };

    const parsedTokens: Token[] = sourceCode 
        ? extractTokensFromSource(sourceCode)
        : tokens.map(token => ({ type: 'UNKNOWN', value: token }));

    const getTypeColor = (type: string): string => {
        const colors: { [key: string]: string } = {
            'KEYWORD': '#9C27B0',     // Roxo
            'IDENTIFIER': '#2196F3',  // Azul
            'NUMBER': '#4CAF50',      // Verde
            'OPERATOR': '#FF9800',    // Laranja
            'ASSIGN': '#F44336',      // Vermelho
            'DELIMITER': '#795548',   // Marrom
            'UNKNOWN': '#607D8B'      // Cinza
        };
        return colors[type] || colors['UNKNOWN'];
    };

    const getTypeIcon = (type: string): string => {
        const icons: { [key: string]: string } = {
            'KEYWORD': '🔑',
            'IDENTIFIER': '🏷️',
            'NUMBER': '🔢',
            'OPERATOR': '⚡',
            'ASSIGN': '⇄',
            'DELIMITER': '📌',
            'UNKNOWN': '❓'
        };
        return icons[type] || icons['UNKNOWN'];
    };

    const getTypeDescription = (type: string): string => {
        const descriptions: { [key: string]: string } = {
            'KEYWORD': 'Palavra reservada da linguagem',
            'IDENTIFIER': 'Nome de variável ou função',
            'NUMBER': 'Literal numérico',
            'OPERATOR': 'Operador matemático',
            'ASSIGN': 'Operador de atribuição',
            'DELIMITER': 'Delimitador estrutural',
            'UNKNOWN': 'Token não identificado'
        };
        return descriptions[type] || 'Tipo desconhecido';
    };

    const stats = {
        total: parsedTokens.length,
        types: new Set(parsedTokens.map(t => t.type)).size,
        keywords: parsedTokens.filter(t => t.type === 'KEYWORD').length,
        identifiers: parsedTokens.filter(t => t.type === 'IDENTIFIER').length,
        numbers: parsedTokens.filter(t => t.type === 'NUMBER').length
    };

    return (
        <div className="tokens-container">
            {/* Header */}
            <div className="tokens-header">
                <div className="tokens-title-section">
                    <h3>Token Analysis</h3>
                    <p className="tokens-description">
                        Análise léxica - Tokens identificados no código fonte
                    </p>
                </div>
                <div className="tokens-stats">
                    <span className="stat">{stats.total} tokens</span>
                    <span className="stat">{stats.types} tipos</span>
                    <span className="stat">{stats.keywords} keywords</span>
                </div>
            </div>

            {/* Legenda */}
            <div className="tokens-legend">
                <div className="legend-title">Tipos de Tokens:</div>
                <div className="legend-items">
                    <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
                        <span className="legend-icon">🔑</span> Keyword
                    </span>
                    <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
                        <span className="legend-icon">🏷️</span> Identifier
                    </span>
                    <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
                        <span className="legend-icon">🔢</span> Number
                    </span>
                    <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
                        <span className="legend-icon">⚡</span> Operator
                    </span>
                    <span className="legend-item" style={{ '--color': '#F44336' } as any}>
                        <span className="legend-icon">⇄</span> Assign
                    </span>
                </div>
            </div>

            {/* Informações */}
            <div className="tokens-info">
                <div className="info-section">
                    <h4>🔍 Sobre a Análise Léxica:</h4>
                    <p>
                        A análise léxica é a primeira fase da compilação, onde o código fonte é dividido em tokens -
                        as menores unidades significativas da linguagem de programação.
                    </p>
                </div>
            </div>

            {/* Tabela de Tokens */}
            {parsedTokens.length > 0 ? (
                <div className="tokens-table-container">
                    <div className="table-header">
                        <span className="table-title">Tokens Identificados</span>
                        <span className="table-subtitle">{stats.total} tokens encontrados</span>
                    </div>
                    <div className="tokens-table-wrapper">
                        <table className="tokens-table">
                            <thead>
                                <tr>
                                    <th className="col-index">#</th>
                                    <th className="col-icon"></th>
                                    <th className="col-type">Tipo</th>
                                    <th className="col-value">Valor</th>
                                    <th className="col-line">Linha</th>
                                    <th className="col-description">Descrição</th>
                                </tr>
                            </thead>
                            <tbody>
                                {parsedTokens.map((token, index) => (
                                    <tr key={index} className="token-row">
                                        <td className="token-index">
                                            <span className="index-badge">{index + 1}</span>
                                        </td>
                                        <td className="token-icon">
                                            <span 
                                                className="token-icon-badge"
                                                style={{ backgroundColor: getTypeColor(token.type) }}
                                            >
                                                {getTypeIcon(token.type)}
                                            </span>
                                        </td>
                                        <td className="token-type">
                                            <span 
                                                className="type-badge"
                                                style={{ backgroundColor: getTypeColor(token.type) }}
                                            >
                                                {token.type}
                                            </span>
                                        </td>
                                        <td className="token-value">
                                            <code>{token.value}</code>
                                        </td>
                                        <td className="token-line">
                                            {token.line ? (
                                                <span className="line-badge">Linha {token.line}</span>
                                            ) : (
                                                <span className="line-empty">—</span>
                                            )}
                                        </td>
                                        <td className="token-description">
                                            {getTypeDescription(token.type)}
                                        </td>
                                    </tr>
                                ))}
                            </tbody>
                        </table>
                    </div>
                </div>
            ) : (
                <div className="tokens-empty">
                    <div className="empty-icon">🔍</div>
                    <h3>Nenhum Token Encontrado</h3>
                    <p>Não foi possível identificar tokens no código fonte.</p>
                </div>
            )}

            {/* Resumo */}
            <div className="tokens-summary">
                <h4>📊 Resumo da Análise Léxica:</h4>
                <div className="summary-grid">
                    <div className="summary-item">
                        <span className="summary-label">Total de Tokens:</span>
                        <span className="summary-value">{stats.total}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Tipos Diferentes:</span>
                        <span className="summary-value">{stats.types}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Keywords:</span>
                        <span className="summary-value">{stats.keywords}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Identificadores:</span>
                        <span className="summary-value">{stats.identifiers}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Números:</span>
                        <span className="summary-value">{stats.numbers}</span>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default TokensTab;