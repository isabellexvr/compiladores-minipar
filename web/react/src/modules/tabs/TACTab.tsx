// TACTab.tsx - VERSÃO FINAL CORRIGIDA
import React from 'react';
import './TACTab.css';

export interface TACInstruction {
    id?: number;
    result: string;
    operation: string;
    operands: string[];
    type: string;
    isTemporary?: boolean;
}

interface TACTabProps {
    instructions: TACInstruction[];
    className?: string;
}

const TACTab: React.FC<TACTabProps> = ({ instructions, className = '' }) => {
    console.log('🔢 TAC Instructions recebidas:', instructions);

    const getInstructionColor = (type: string): string => {
        switch (type) {
            case 'ASSIGNMENT': return '#4CAF50';
            case 'BINARY_OP': return '#2196F3';
            case 'PRINT': return '#FF9800';
            case 'LABEL': return '#9C27B0';
            case 'JUMP': return '#F44336';
            case 'CONDITIONAL_JUMP': return '#E91E63';
            default: return '#607D8B';
        }
    };

    const getInstructionIcon = (type: string): string => {
        switch (type) {
            case 'ASSIGNMENT': return '←';
            case 'BINARY_OP': return '⚡';
            case 'PRINT': return '📤';
            case 'LABEL': return '🏷️';
            case 'JUMP': return '↷';
            case 'CONDITIONAL_JUMP': return '↷?';
            default: return '•';
        }
    };

    const getInstructionDescription = (type: string): string => {
        switch (type) {
            case 'ASSIGNMENT': return 'Atribuição de valor';
            case 'BINARY_OP': return 'Operação aritmética/lógica';
            case 'PRINT': return 'Instrução de saída';
            case 'LABEL': return 'Marcador para saltos';
            case 'JUMP': return 'Salto incondicional';
            case 'CONDITIONAL_JUMP': return 'Salto condicional';
            default: return 'Instrução geral';
        }
    };

    // ✅ FUNÇÃO FINAL CORRIGIDA
    const formatInstruction = (instr: TACInstruction): { text: string; type: string } => {
        const op1 = instr.operands[0] || '';
        const op2 = instr.operands[1] || '';
        
        // ✅ DETECTAR TIPO automaticamente se necessário
        let actualType = instr.type;
        if (!actualType || actualType === 'undefined') {
            if (instr.operation === 'PRINT') {
                actualType = 'PRINT';
            } else if (instr.result.endsWith(':')) {
                actualType = 'LABEL';
            } else if (instr.operation === 'if_false') {
                actualType = 'CONDITIONAL_JUMP';
            } else if (instr.operation === 'goto') {
                actualType = 'JUMP';
            } else if (op2 && ['+', '-', '*', '/', '==', '!=', '<', '<=', '>', '>='].includes(instr.operation)) {
                actualType = 'BINARY_OP';
            } else {
                actualType = 'ASSIGNMENT';
            }
        }

        let text = '';
        
        switch (actualType) {
            case 'PRINT':
                text = `print ${op1}`;
                break;
            case 'LABEL':
                text = `${instr.result}:`;
                break;
            case 'CONDITIONAL_JUMP':
                text = `if_false ${op1} goto ${op2}`;
                break;
            case 'JUMP':
                text = `goto ${op2}`;
                break;
            case 'ASSIGNMENT':
                text = `${instr.result} = ${op1}`;
                break;
            case 'BINARY_OP':
                text = `${instr.result} = ${op1} ${instr.operation} ${op2}`;
                break;
            default:
                if (instr.operation && op2) {
                    text = `${instr.result} = ${op1} ${instr.operation} ${op2}`;
                } else if (op1) {
                    text = `${instr.result} = ${op1}`;
                } else {
                    text = `${instr.result}`;
                }
        }

        return { text, type: actualType };
    };

    // ✅ ESTATÍSTICAS USANDO TIPO CORRETO
    const stats = {
        total: instructions.length,
        assignments: instructions.filter(i => {
            const { type } = formatInstruction(i);
            return type === 'ASSIGNMENT';
        }).length,
        operations: instructions.filter(i => {
            const { type } = formatInstruction(i);
            return type === 'BINARY_OP';
        }).length,
        jumps: instructions.filter(i => {
            const { type } = formatInstruction(i);
            return type === 'JUMP' || type === 'CONDITIONAL_JUMP';
        }).length,
        labels: instructions.filter(i => {
            const { type } = formatInstruction(i);
            return type === 'LABEL';
        }).length,
        prints: instructions.filter(i => {
            const { type } = formatInstruction(i);
            return type === 'PRINT';
        }).length,
        temporaries: new Set(instructions
            .filter(i => i.isTemporary || i.result.startsWith('t'))
            .map(i => i.result)
        ).size
    };

    if (!instructions || instructions.length === 0) {
        return (
            <div className={`tac-container ${className}`}>
                <div className="tac-empty">
                    <div className="empty-icon">🔢</div>
                    <h3>Código de Três Endereços Não Gerado</h3>
                    <p>O código intermediário de três endereços aparecerá aqui após a compilação.</p>
                </div>
            </div>
        );
    }

    return (
        <div className={`tac-container ${className}`}>
            <div className="tac-header">
                <div className="tac-title-section">
                    <h3>Código Intermediário - Três Endereços</h3>
                    <p className="tac-description">
                        Representação intermediária onde cada instrução tem no máximo três operandos
                    </p>
                </div>
                <div className="tac-stats">
                    <span className="stat">{stats.total} instruções</span>
                    <span className="stat">{stats.temporaries} temporários</span>
                    <span className="stat">{stats.operations} operações</span>
                </div>
            </div>

            {/* Legenda Educativa */}
            <div className="tac-legend">
                <div className="legend-title">Tipos de Instruções:</div>
                <div className="legend-items">
                    <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
                        <span className="legend-icon">←</span> Atribuição
                    </span>
                    <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
                        <span className="legend-icon">⚡</span> Operação
                    </span>
                    <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
                        <span className="legend-icon">📤</span> Saída
                    </span>
                    <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
                        <span className="legend-icon">🏷️</span> Label
                    </span>
                    <span className="legend-item" style={{ '--color': '#F44336' } as any}>
                        <span className="legend-icon">↷</span> Salto
                    </span>
                </div>
            </div>

            {/* Lista de Instruções */}
            <div className="tac-instructions">
                {instructions.map((instr, index) => {
                    const { text, type } = formatInstruction(instr);
                    const displayType = type || instr.type || 'ASSIGNMENT';
                    
                    return (
                        <div
                            key={instr.id || index}
                            className={`tac-instruction ${instr.isTemporary ? 'temporary' : ''}`}
                            style={{ 
                                '--instruction-color': getInstructionColor(displayType)
                            } as any}
                            title={`${getInstructionDescription(displayType)}${instr.isTemporary ? ' (temporária)' : ''}`}
                        >
                            <div className="instruction-line">
                                <span className="line-number">{instr.id !== undefined ? instr.id + 1 : index + 1}</span>
                                <span 
                                    className="instruction-icon"
                                    style={{ backgroundColor: getInstructionColor(displayType) }}
                                >
                                    {getInstructionIcon(displayType)}
                                </span>
                                <code className="instruction-code">
                                    {text}
                                </code>
                                {instr.isTemporary && (
                                    <span className="temporary-badge" title="Variável temporária">
                                        🆕
                                    </span>
                                )}
                            </div>
                            
                            {/* Debug - pode remover depois */}
                            <div className="instruction-debug" style={{ fontSize: '0.7rem', color: '#666', marginLeft: '66px' }}>
                                Type: {displayType} | Operation: "{instr.operation}" | Operands: [{instr.operands.join(', ')}]
                            </div>
                        </div>
                    );
                })}
            </div>

            {/* Resumo Final */}
            <div className="tac-summary">
                <h4>📊 Resumo do Código Intermediário:</h4>
                <div className="summary-grid">
                    <div className="summary-item">
                        <span className="summary-label">Total de Instruções:</span>
                        <span className="summary-value">{stats.total}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Variáveis Temporárias:</span>
                        <span className="summary-value">{stats.temporaries}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Operações:</span>
                        <span className="summary-value">{stats.operations}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Atribuições:</span>
                        <span className="summary-value">{stats.assignments}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Saídas:</span>
                        <span className="summary-value">{stats.prints}</span>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default TACTab;