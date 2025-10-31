// TokensTab.tsx - versão atualizada
import React from 'react';
import './TokensTab.css';

interface Token {
    type: string;
    value: string;
    line: number;
    column: number;
}

interface TokensTabProps {
    tokens: Token[]; // ✅ MUDOU: agora é Token[] em vez de string[]
    sourceCode?: string;
}

const TokensTab: React.FC<TokensTabProps> = ({ tokens, sourceCode }) => {
    console.log('Tokens recebidos:', tokens);

    // ✅ REMOVA a função extractTokensFromSource - não é mais necessária
    // pois agora recebemos tokens já estruturados do backend

    const getTypeColor = (type: string): string => {
        const colors: { [key: string]: string } = {
            'KEYWORD': '#9C27B0',     // Roxo
            'IDENTIFIER': '#2196F3',  // Azul
            'NUMBER': '#4CAF50',      // Verde
            'OPERATOR': '#FF9800',    // Laranja
            'ASSIGN': '#F44336',      // Vermelho - Note: no JSON virá como OPERATOR
            'DELIMITER': '#795548',   // Marrom
            'END': '#607D8B',         // Cinza para END
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
            'END': '🏁',
            'UNKNOWN': '❓'
        };
        return icons[type] || icons['UNKNOWN'];
    };

    const getTypeDescription = (type: string): string => {
        const descriptions: { [key: string]: string } = {
            'KEYWORD': 'Palavra reservada da linguagem',
            'IDENTIFIER': 'Nome de variável ou função',
            'NUMBER': 'Literal numérico',
            'OPERATOR': 'Operador matemático ou de atribuição',
            'DELIMITER': 'Delimitador estrutural',
            'END': 'Fim do código',
            'UNKNOWN': 'Token não identificado'
        };
        return descriptions[type] || 'Tipo desconhecido';
    };

    // ✅ Use os tokens diretamente, sem precisar fazer parsing
    const parsedTokens: Token[] = tokens;

    const stats = {
        total: parsedTokens.length,
        types: new Set(parsedTokens.map(t => t.type)).size,
        keywords: parsedTokens.filter(t => t.type === 'KEYWORD').length,
        identifiers: parsedTokens.filter(t => t.type === 'IDENTIFIER').length,
        numbers: parsedTokens.filter(t => t.type === 'NUMBER').length,
        operators: parsedTokens.filter(t => t.type === 'OPERATOR').length
    };

    return (
        <div className="tokens-container">
            {/* Header */}
            <div className="tokens-header">
                <div className="tokens-title-section">
                    <h3>Token Analysis</h3>
                    <p className="tokens-description">
                        Análise léxica - {stats.total} tokens identificados no código fonte
                    </p>
                </div>
                <div className="tokens-stats">
                    <span className="stat">{stats.total} tokens</span>
                    <span className="stat">{stats.types} tipos</span>
                    <span className="stat">{stats.keywords} keywords</span>
                </div>
            </div>

            {/* Legenda Atualizada */}
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
                    <span className="legend-item" style={{ '--color': '#795548' } as any}>
                        <span className="legend-icon">📌</span> Delimiter
                    </span>
                    <span className="legend-item" style={{ '--color': '#607D8B' } as any}>
                        <span className="legend-icon">🏁</span> End
                    </span>
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
                                    <th className="col-column">Coluna</th>
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
                                            <code>{token.value || '(vazio)'}</code>
                                        </td>
                                        <td className="token-line">
                                            <span className="line-badge">Linha {token.line}</span>
                                        </td>
                                        <td className="token-column">
                                            <span className="column-badge">Col {token.column}</span>
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

            {/* Resumo Atualizado */}
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
                    <div className="summary-item">
                        <span className="summary-label">Operadores:</span>
                        <span className="summary-value">{stats.operators}</span>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default TokensTab;