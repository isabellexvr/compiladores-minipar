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
    sourceCode?: string; // ← ADICIONE ISSO
}

const TokensTab: React.FC<TokensTabProps> = ({ tokens, sourceCode }) => {
    // Se temos o código fonte, vamos extrair tokens dele diretamente
    const extractTokensFromSource = (code: string): Token[] => {
        const lines = code.split('\n');
        const extracted: Token[] = [];
        let lineNumber = 1;

        lines.forEach(line => {
            const words = line
                .split(/(\s+|\(|\)|;|,|\+|\-|\*|\/|=)/) // Split por operadores e delimitadores
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

    // Use o código fonte se disponível, senão use os tokens do backend
    const parsedTokens: Token[] = sourceCode 
        ? extractTokensFromSource(sourceCode)
        : tokens.map(token => ({ type: 'UNKNOWN', value: token }));

    const getTypeColor = (type: string): string => {
        const colors: { [key: string]: string } = {
            'KEYWORD': 'var(--primary)',
            'IDENTIFIER': 'var(--secondary)', 
            'NUMBER': '#F2B950',
            'OPERATOR': '#8C4C3E',
            'ASSIGN': '#76D95B',
            'DELIMITER': '#9B59B6',
            'UNKNOWN': '#95A5A6'
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

    return (
        <div className="tokens-tab">
            <div className="tokens-header">
                <h3>Token Analysis {sourceCode && "(from source code)"}</h3>
                <div className="tokens-stats">
                    <span className="stat">{parsedTokens.length} tokens</span>
                    <span className="stat">
                        {new Set(parsedTokens.map(t => t.type)).size} types
                    </span>
                </div>
            </div>

            {parsedTokens.length > 0 ? (
                <div className="tokens-table-container">
                    <table className="tokens-table">
                        <thead>
                            <tr>
                                <th className="col-index">#</th>
                                <th className="col-type">Type</th>
                                <th className="col-value">Value</th>
                                <th className="col-line">Line</th>
                                <th className="col-description">Description</th>
                            </tr>
                        </thead>
                        <tbody>
                            {parsedTokens.map((token, index) => (
                                <tr key={index} className="token-row">
                                    <td className="token-index">
                                        <span className="index-badge">{index + 1}</span>
                                    </td>
                                    <td className="token-type">
                                        <span 
                                            className="type-badge"
                                            style={{ 
                                                backgroundColor: getTypeColor(token.type),
                                                color: ['KEYWORD', 'NUMBER'].includes(token.type) ? 'var(--dark)' : 'var(--light)'
                                            }}
                                        >
                                            <span className="type-icon">{getTypeIcon(token.type)}</span>
                                            {token.type}
                                        </span>
                                    </td>
                                    <td className="token-value">
                                        <code>{token.value}</code>
                                    </td>
                                    <td className="token-line">
                                        {token.line ? `Line ${token.line}` : '-'}
                                    </td>
                                    <td className="token-description">
                                        {getTypeDescription(token.type)}
                                    </td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </div>
            ) : (
                <div className="no-tokens">
                    <div className="no-tokens-icon">🔍</div>
                    <h4>No tokens found</h4>
                    <p>The source code doesn't contain any recognizable tokens.</p>
                </div>
            )}

            <div className="tokens-legend">
                <h4>Token Types Legend</h4>
                <div className="legend-items">
                    {['KEYWORD', 'IDENTIFIER', 'NUMBER', 'OPERATOR', 'ASSIGN', 'DELIMITER'].map(type => (
                        <div key={type} className="legend-item">
                            <span 
                                className="legend-color" 
                                style={{ backgroundColor: getTypeColor(type) }}
                            ></span>
                            <span>
                                <strong>{type}</strong> - {getTypeDescription(type)}
                            </span>
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
};

export default TokensTab;