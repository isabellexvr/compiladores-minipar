// ARMTab.tsx - VERSÃO FINAL LIMPA
import React, { useState } from 'react';
import './ARMTab.css';

interface ARMTabProps {
    assemblyCode: string;
    className?: string;
}

const ARMTab: React.FC<ARMTabProps> = ({ assemblyCode, className = '' }) => {
    const [copied, setCopied] = useState(false);

    const handleCopyAll = async () => {
        const text = assemblyCode.trim();
        try {
            if (navigator.clipboard && navigator.clipboard.writeText) {
                await navigator.clipboard.writeText(text);
            } else {
                // Fallback
                const ta = document.createElement('textarea');
                ta.value = text;
                document.body.appendChild(ta);
                ta.select();
                document.execCommand('copy');
                document.body.removeChild(ta);
            }
            setCopied(true);
            setTimeout(() => setCopied(false), 1800);
        } catch (e) {
            console.error('Copy failed', e);
        }
    };
    const parseAssemblyCode = (code: string) => {
        if (!code) return [];
        
        const lines = code.split('\n');
        const processedLines: string[] = [];
        
        for (let i = 0; i < lines.length; i++) {
            const currentLine = lines[i].trim();
            if (currentLine.length === 0) continue;
            
            // Juntar linhas que são continuação de strings
            if (processedLines.length > 0) {
                const lastLine = processedLines[processedLines.length - 1];
                if ((lastLine.includes('"') && !lastLine.match(/".*"/)) || 
                    (lastLine.includes('.asciz') && !lastLine.endsWith('"'))) {
                    processedLines[processedLines.length - 1] += ' ' + currentLine;
                    continue;
                }
            }
            
            processedLines.push(currentLine);
        }
        
        return processedLines
            .filter(line => line.length > 0)
            .map((line, index) => {
                let type = 'other';
                let description = '';
                
                if (line.startsWith('.')) {
                    type = 'directive';
                    description = 'Diretiva do assembler';
                } 
                else if (line.endsWith(':')) {
                    type = 'label';
                    description = 'Label/marcador';
                }
                else if (line.startsWith('@') || line.startsWith('//')) {
                    type = 'comment';
                    description = 'Comentário';
                }
                else if (line.includes(':') && !line.startsWith(' ')) {
                    type = 'data';
                    description = 'Definição de dados';
                }
                else if (line.startsWith('mov') || line.startsWith('ldr') || line.startsWith('str')) {
                    type = 'instruction';
                    description = 'Instrução de movimento/memória';
                }
                else if (line.startsWith('add') || line.startsWith('sub') || 
                         line.startsWith('mul') || line.startsWith('div')) {
                    type = 'instruction';
                    description = 'Instrução aritmética';
                }
                else if (line.startsWith('cmp')) {
                    type = 'instruction';
                    description = 'Instrução de comparação';
                }
                else if (line.startsWith('b')) {
                    type = line.startsWith('bl') ? 'branch' : 'instruction';
                    description = line.startsWith('bl') ? 'Chamada de função' : 'Salto condicional';
                }
                else if (line.startsWith('push') || line.startsWith('pop')) {
                    type = 'stack';
                    description = 'Operação de pilha';
                }
                else if (line.includes('svc')) {
                    type = 'syscall';
                    description = 'Chamada de sistema';
                }
                
                return { 
                    line, 
                    type, 
                    description, 
                    number: index + 1
                };
            });
    };

    const getLineColor = (type: string): string => {
        switch (type) {
            case 'directive': return '#9C27B0';
            case 'label': return '#2196F3';
            case 'instruction': return '#4CAF50';
            case 'stack': return '#FF9800';
            case 'branch': return '#F44336';
            case 'memory': return '#009688';
            case 'data': return '#795548';
            case 'comment': return '#607D8B';
            case 'syscall': return '#E91E63';
            default: return '#757575';
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
            case 'syscall': return '⚡';
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
            case 'syscall': return 'Chamada de Sistema';
            default: return 'Outro';
        }
    };

    const assemblyLines = parseAssemblyCode(assemblyCode);

    const stats = {
        total: assemblyLines.length,
        instructions: assemblyLines.filter(l => l.type === 'instruction').length,
        directives: assemblyLines.filter(l => l.type === 'directive').length,
        labels: assemblyLines.filter(l => l.type === 'label').length,
        data: assemblyLines.filter(l => l.type === 'data').length,
        branches: assemblyLines.filter(l => l.type === 'branch').length,
        stackOps: assemblyLines.filter(l => l.type === 'stack').length,
        syscalls: assemblyLines.filter(l => l.type === 'syscall').length
    };

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
                    <span className="stat">{stats.total} linhas</span>
                    <span className="stat">{stats.instructions} instruções</span>
                    <span className="stat">{stats.directives} diretivas</span>
                    <button
                        type="button"
                        className="arm-copy-btn"
                        onClick={handleCopyAll}
                        aria-label="Copiar todo o código ARM"
                        title="Copiar todo o código ARM"
                    >
                        {copied ? '✅ Copiado' : '📋 Copiar'}
                    </button>
                </div>
            </div>

            <div className="arm-legend">
                <div className="legend-title">Legenda do Código ARM:</div>
                <div className="legend-items">
                    <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
                        <span className="legend-icon">🔧</span> Instrução
                    </span>
                    <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
                        <span className="legend-icon">🏷️</span> Label
                    </span>
                    <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
                        <span className="legend-icon">⚙️</span> Diretiva
                    </span>
                    <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
                        <span className="legend-icon">📚</span> Pilha
                    </span>
                    <span className="legend-item" style={{ '--color': '#F44336' } as any}>
                        <span className="legend-icon">↷</span> Branch
                    </span>
                    <span className="legend-item" style={{ '--color': '#E91E63' } as any}>
                        <span className="legend-icon">⚡</span> Syscall
                    </span>
                </div>
            </div>

            <div className="arm-info">
                <div className="info-section">
                    <h4>📋 Sobre o Código ARMv7:</h4>
                    <p>
                        Este código assembly é específico para processadores <strong>ARMv7</strong> e pode ser 
                        executado no emulador <strong>CPUlator</strong>.
                    </p>
                </div>
            </div>

            <div className="arm-code-container">
                <div className="arm-code-header">
                    <span className="code-title">Código Assembly ARMv7</span>
                    <span className="code-subtitle">
                        {stats.instructions} instruções, {stats.labels} labels, {stats.directives} diretivas
                    </span>
                </div>
                <div className="arm-code">
                    {assemblyLines.map((item) => (
                        <div
                            key={item.number}
                            className="arm-line"
                            style={{ 
                                '--line-color': getLineColor(item.type)
                            } as any}
                            title={`${item.description} | Linha ${item.number}`}
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

            <div className="arm-summary">
                <h4>📊 Resumo do Código Gerado:</h4>
                <div className="summary-grid">
                    <div className="summary-item">
                        <span className="summary-label">Total de Linhas:</span>
                        <span className="summary-value">{stats.total}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Instruções ARM:</span>
                        <span className="summary-value">{stats.instructions}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Diretivas:</span>
                        <span className="summary-value">{stats.directives}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Labels:</span>
                        <span className="summary-value">{stats.labels}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Operações Stack:</span>
                        <span className="summary-value">{stats.stackOps}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Syscalls:</span>
                        <span className="summary-value">{stats.syscalls}</span>
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