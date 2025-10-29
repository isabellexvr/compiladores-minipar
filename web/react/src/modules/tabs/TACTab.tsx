// TACTab.tsx - Componente para Visualização do Código de Três Endereços
import React from 'react';
import './TACTab.css';

export interface TACInstruction {
    result: string;
    op: string;
    arg1: string;
    arg2: string;
    type: string;
}

interface TACTabProps {
    instructions: TACInstruction[];
    className?: string;
}

const TACTab: React.FC<TACTabProps> = ({ instructions, className = '' }) => {
    const getInstructionColor = (type: string): string => {
        switch (type) {
            case 'ASSIGNMENT': return '#4CAF50';      // Verde - atribuições simples
            case 'BINARY_OP': return '#2196F3';       // Azul - operações aritméticas
            case 'PRINT': return '#FF9800';           // Laranja - saída
            case 'LABEL': return '#9C27B0';           // Roxo - labels/marcadores
            case 'JUMP': return '#F44336';            // Vermelho - saltos incondicionais
            case 'CONDITIONAL_JUMP': return '#E91E63';// Rosa - saltos condicionais
            default: return '#607D8B';                // Cinza - outros
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

    const formatInstruction = (instr: TACInstruction): string => {
        switch (instr.type) {
            case 'PRINT':
                return `print ${instr.arg1}`;
            case 'LABEL':
                return `${instr.result}:`;
            case 'CONDITIONAL_JUMP':
                return `if_false ${instr.arg1} goto ${instr.arg2}`;
            case 'JUMP':
                return `goto ${instr.arg2}`;
            case 'ASSIGNMENT':
                return `${instr.result} = ${instr.arg1}`;
            case 'BINARY_OP':
                return `${instr.result} = ${instr.arg1} ${instr.op} ${instr.arg2}`;
            default:
                return `${instr.result} = ${instr.arg1} ${instr.op} ${instr.arg2}`;
        }
    };

    // Estatísticas para análise
    const stats = {
        total: instructions.length,
        assignments: instructions.filter(i => i.type === 'ASSIGNMENT').length,
        operations: instructions.filter(i => i.type === 'BINARY_OP').length,
        jumps: instructions.filter(i => i.type === 'JUMP' || i.type === 'CONDITIONAL_JUMP').length,
        labels: instructions.filter(i => i.type === 'LABEL').length,
        prints: instructions.filter(i => i.type === 'PRINT').length,
        temporaries: new Set(instructions.filter(i => i.result.startsWith('t')).map(i => i.result)).size
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
                        Representação intermediária onde cada instrução tem no máximo três operandos: 
                        <strong> resultado = operando1 operador operando2</strong>
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
                <div className="legend-title">Legenda do Código de Três Endereços:</div>
                <div className="legend-items">
                    <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
                        <span className="legend-icon">←</span> Atribuição (t0 = 10)
                    </span>
                    <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
                        <span className="legend-icon">⚡</span> Operação (t2 = x + y)
                    </span>
                    <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
                        <span className="legend-icon">📤</span> Saída (print resultado)
                    </span>
                    <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
                        <span className="legend-icon">🏷️</span> Label (LOOP:)
                    </span>
                    <span className="legend-item" style={{ '--color': '#F44336' } as any}>
                        <span className="legend-icon">↷</span> Salto (goto LOOP)
                    </span>
                </div>
            </div>

            {/* Informações sobre Temporários */}
            <div className="tac-info">
                <div className="info-section">
                    <h4>Variáveis Temporárias:</h4>
                    <p>
                        As variáveis <code>t0, t1, t2, ...</code> são temporárias geradas automaticamente 
                        para armazenar resultados intermediários das operações.
                    </p>
                </div>
                <div className="info-section">
                    <h4>Estrutura de Três Endereços:</h4>
                    <p>
                        Cada linha segue o formato: <code>resultado = operando1 operador operando2</code><br/>
                        Exemplo: <code>t2 = x + y</code> onde <code>t2</code> é o resultado, <code>x</code> e <code>y</code> são operandos, <code>+</code> é o operador.
                    </p>
                </div>
            </div>

            {/* Lista de Instruções */}
            <div className="tac-instructions">
                {instructions.map((instr, index) => (
                    <div
                        key={index}
                        className="tac-instruction"
                        style={{ 
                            '--instruction-color': getInstructionColor(instr.type)
                        } as any}
                        title={getInstructionDescription(instr.type)}
                    >
                        <div className="instruction-line">
                            <span className="line-number">{index + 1}</span>
                            <span 
                                className="instruction-icon"
                                style={{ backgroundColor: getInstructionColor(instr.type) }}
                            >
                                {getInstructionIcon(instr.type)}
                            </span>
                            <code className="instruction-code">
                                {formatInstruction(instr)}
                            </code>
                        </div>
                        
                        {/* Tooltip informativo para instruções complexas */}
                        {(instr.type === 'CONDITIONAL_JUMP' || instr.type === 'JUMP') && (
                            <div className="instruction-hint">
                                {instr.type === 'CONDITIONAL_JUMP' 
                                    ? 'Salto condicional: se a condição for falsa, pula para o label'
                                    : 'Salto incondicional: sempre pula para o label'
                                }
                            </div>
                        )}
                    </div>
                ))}
            </div>

            {/* Resumo Final */}
            <div className="tac-summary">
                <h4>Resumo da Geração de Código:</h4>
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
                        <span className="summary-label">Operações Aritméticas:</span>
                        <span className="summary-value">{stats.operations}</span>
                    </div>
                    <div className="summary-item">
                        <span className="summary-label">Estruturas de Controle:</span>
                        <span className="summary-value">{stats.jumps} saltos, {stats.labels} labels</span>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default TACTab;