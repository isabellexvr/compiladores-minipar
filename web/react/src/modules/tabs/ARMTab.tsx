// ARMTab.tsx - Componente para Visualização do Código ARMv7 Assembly
import React from 'react';
import './ARMTab.css';

interface ARMTabProps {
    assemblyCode: string;
    className?: string;
}

const ARMTab: React.FC<ARMTabProps> = ({ assemblyCode, className = '' }) => {
    const parseAssemblyCode = (code: string) => {
        if (!code) return [];
        
        return code.split('\n')
            .map(line => line.trim())
            .filter(line => line.length > 0)
            .map((line, index) => {
                // Classificar o tipo de linha
                let type = 'other';
                let description = '';
                
                if (line.startsWith('.')) {
                    type = 'directive';
                    description = 'Diretiva do assembler';
                } else if (line.endsWith(':')) {
                    type = 'label';
                    description = 'Label/marcador';
                } else if (line.startsWith('@')) {
                    type = 'comment';
                    description = 'Comentário';
                } else if (line.includes(':')) {
                    type = 'data';
                    description = 'Definição de dados';
                } else if (line.startsWith('mov') || line.startsWith('add') || 
                           line.startsWith('sub') || line.startsWith('mul') ||
                           line.startsWith('cmp') || line.startsWith('b')) {
                    type = 'instruction';
                    description = 'Instrução ARM';
                } else if (line.startsWith('push') || line.startsWith('pop')) {
                    type = 'stack';
                    description = 'Operação de pilha';
                } else if (line.startsWith('bl') || line.startsWith('bx')) {
                    type = 'branch';
                    description = 'Chamada de função/branch';
                } else if (line.startsWith('ldr') || line.startsWith('str')) {
                    type = 'memory';
                    description = 'Acesso à memória';
                }
                
                return { line, type, description, number: index + 1 };
            });
    };

    const getLineColor = (type: string): string => {
        switch (type) {
            case 'directive': return '#9C27B0';    // Roxo - diretivas
            case 'label': return '#2196F3';        // Azul - labels
            case 'instruction': return '#4CAF50';  // Verde - instruções
            case 'stack': return '#FF9800';        // Laranja - stack
            case 'branch': return '#F44336';       // Vermelho - branches
            case 'memory': return '#009688';       // Teal - memória
            case 'data': return '#795548';         // Marrom - dados
            case 'comment': return '#607D8B';      // Cinza - comentários
            default: return '#757575';             // Cinza escuro - outros
        }
    };

    const getLineIcon = (type: string): string => {
        switch (type) {
            case 'directive': return '⚙️';
            case 'label': return '🏷️';
            case 'instruction': return '🔧';
            case 'stack': return '📚';
            case 'branch': return '↷';
            case 'memory': return '💾';
            case 'data': return '📊';
            case 'comment': return '💬';
            default: return '•';
        }
    };

    const getLineDescription = (type: string): string => {
        switch (type) {
            case 'directive': return 'Diretiva do Assembler';
            case 'label': return 'Label/Marcador';
            case 'instruction': return 'Instrução ARM';
            case 'stack': return 'Operação com Pilha';
            case 'branch': return 'Salto/Chamada';
            case 'memory': return 'Acesso à Memória';
            case 'data': return 'Definição de Dados';
            case 'comment': return 'Comentário';
            default: return 'Outro';
        }
    };

    const assemblyLines = parseAssemblyCode(assemblyCode);

    if (!assemblyCode || assemblyLines.length === 0) {
        return (
            <div className={`arm-container ${className}`}>
                <div className="arm-empty">
                    <div className="empty-icon">⚙️</div>
                    <h3>No ARM Assembly Available</h3>
                    <p>The ARMv7 assembly code will appear here after compilation.</p>
                </div>
            </div>
        );
    }

    return (
        <div className={`arm-container ${className}`}>
            <div className="arm-header">
                <div className="arm-title-section">
                    <h3>ARMv7 Assembly Code</h3>
                    <p className="arm-description">
                        Código de máquina gerado para arquitetura ARMv7 - Pronto para execução no CPUlator
                    </p>
                </div>
                <div className="arm-stats">
                    <span className="stat">{assemblyLines.length} linhas</span>
                    <span className="stat">
                        {assemblyLines.filter(l => l.type === 'instruction').length} instruções
                    </span>
                </div>
            </div>

            {/* Legenda Educativa */}
            <div className="arm-legend">
                <div className="legend-title">Legenda do Código ARM:</div>
                <div className="legend-items">
                    <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
                        <span className="legend-icon">🔧</span> Instrução (mov, add)
                    </span>
                    <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
                        <span className="legend-icon">🏷️</span> Label (_start:)
                    </span>
                    <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
                        <span className="legend-icon">⚙️</span> Diretiva (.global, .data)
                    </span>
                    <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
                        <span className="legend-icon">📚</span> Pilha (push, pop)
                    </span>
                    <span className="legend-item" style={{ '--color': '#F44336' } as any}>
                        <span className="legend-icon">↷</span> Branch (bl, b)
                    </span>
                </div>
            </div>

            {/* Informações sobre ARM */}
            <div className="arm-info">
                <div className="info-section">
                    <h4>📋 Sobre o Código ARMv7:</h4>
                    <p>
                        Este código assembly é específico para processadores <strong>ARMv7</strong> e pode ser 
                        executado no emulador <strong>CPUlator</strong>. Cada instrução corresponde diretamente 
                        às operações do processador.
                    </p>
                </div>
                <div className="info-section">
                    <h4>🧮 Registradores ARM:</h4>
                    <p>
                        <code>r0-r6</code> - Registradores de uso geral | 
                        <code> r7</code> - Syscall | 
                        <code> sp</code> - Stack Pointer | 
                        <code> lr</code> - Link Register
                    </p>
                </div>
            </div>

            {/* Código Assembly */}
            <div className="arm-code-container">
                <div className="arm-code-header">
                    <span className="code-title">Código Assembly ARMv7</span>
                    <span className="code-subtitle">Pronto para CPUlator</span>
                </div>
                <div className="arm-code">
                    {assemblyLines.map((item) => (
                        <div
                            key={item.number}
                            className="arm-line"
                            style={{ 
                                '--line-color': getLineColor(item.type)
                            } as any}
                            title={item.description}
                        >
                            <div className="line-number">{item.number}</div>
                            <div 
                                className="line-icon"
                                style={{ backgroundColor: getLineColor(item.type) }}
                            >
                                {getLineIcon(item.type)}
                            </div>
                            <code className="line-code">
                                {item.line}
                            </code>
                            <div className="line-type">
                                {getLineDescription(item.type)}
                            </div>
                        </div>
                    ))}
                </div>
            </div>

            {/* Resumo Final */}
            <div className="arm-summary">
                <h4>📊 Resumo do Código Gerado:</h4>
                <div className="summary-grid">
                    <div className="summary-item">
                        <span className="summary-label">Total de Linhas:</span>
                        <span className="summary-value">{assemblyLines.length}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Instruções ARM:</span>
                        <span className="summary-value">
                            {assemblyLines.filter(l => l.type === 'instruction').length}
                        </span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Diretivas:</span>
                        <span className="summary-value">
                            {assemblyLines.filter(l => l.type === 'directive').length}
                        </span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Labels:</span>
                        <span className="summary-value">
                            {assemblyLines.filter(l => l.type === 'label').length}
                        </span>
                    </div>
                </div>
                <div className="execution-note">
                    <strong>💡 Pronto para execução:</strong> Este código pode ser copiado e colado diretamente no CPUlator
                </div>
            </div>
        </div>
    );
};

export default ARMTab;