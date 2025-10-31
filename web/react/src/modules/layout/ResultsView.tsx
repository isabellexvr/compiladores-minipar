import React, { useState } from 'react';
import TokensTab from '../tabs/TokensTab';
import ASTVisualizer from '../tabs/ASTVisualizer';
import './ResultsView.css';
import TACTab from '../tabs/TACTab';
import SymbolTableTab from '../tabs/SymbolTableTab';
import ARMTab from '../tabs/ARMTab';
import OutputTab from '../tabs/OutputTab';

export interface Token {
    type: string;
    value: string;
    line: number;
    column: number;
}

export interface SymbolEntry {
  name: string;
  symbolType: string; // ✅ Mudei de 'type' para 'symbolType' para bater com o JSON
  dataType: string;
  scope: string;
  value?: string; // Opcional
}

export interface TACInstruction {
    id?: number; // ✅ Adicionar id do JSON
    result: string;
    op: string;
    arg1: string;
    arg2: string;
    type: string;
    isTemporary?: boolean; // ✅ Nova propriedade do JSON
    operation?: string; // Add missing property to match TACTab
    operands?: string[]; // Add missing property to match TACTab
}

export interface CompilationArtifacts {
    rawOutput: string;
    tokens: Token[];
    syntaxTree?: string;
    symbolTable?: SymbolEntry[];
    tac?: TACInstruction[];
    arm?: string;
    output?: string;
}

interface ResultsViewProps {
    code: string;
    artifacts: CompilationArtifacts;
    onBack: () => void;
}

const tabs = [
    { id: 'tokens', label: 'Tokens', icon: '🔤' },
    { id: 'syntax', label: 'Syntax Tree', icon: '🌳' },
    { id: 'symbols', label: 'Symbol Table', icon: '📊' },
    { id: 'tac', label: '3 Address Code', icon: '🔢' },
    { id: 'arm', label: 'ARMv7 Assembly', icon: '⚙️' },
    { id: 'output', label: 'Program Output', icon: '📺' } // NOVA ABA
];

const ResultsView: React.FC<ResultsViewProps> = ({ code, artifacts, onBack }) => {
    const [active, setActive] = useState<string>('tokens');

    const renderTabContent = () => {
        switch (active) {
            case 'tokens':
                return <TokensTab tokens={artifacts.tokens} sourceCode={code} />;

            case 'syntax':
                return artifacts.syntaxTree ? (
                    <ASTVisualizer astString={artifacts.syntaxTree} />
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">🌳</div>
                        <h3>No Syntax Tree Available</h3>
                        <p>The syntax tree will appear here after compilation.</p>
                    </div>
                );

                // ResultsView.tsx - na função renderTabContent
            case 'symbols': {
                if (!artifacts.symbolTable || artifacts.symbolTable.length === 0) {
                    return (
                        <div className="placeholder-content">
                            <div className="placeholder-icon">📊</div>
                            <h3>No Symbol Table Available</h3>
                            <p>The symbol table will appear here after compilation.</p>
                        </div>
                    );
                }
                
                // ✅ AGORA artifacts.symbolTable já é SymbolEntry[]
                return <SymbolTableTab table={artifacts.symbolTable} />;
            }

        // ResultsView.tsx - na função renderTabContent
            case 'tac':
                return artifacts.tac ? (
                    <TACTab 
                        instructions={artifacts.tac} // ✅ AGORA já é TACInstruction[]
                        className="tac-tab"
                    />
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">🔢</div>
                        <h3>No 3-Address Code Available</h3>
                        <p>The three-address code will appear here after compilation.</p>
                    </div>
                );

            case 'arm':
                return artifacts.arm ? (
                    <ARMTab 
                        assemblyCode={artifacts.arm}
                        className="arm-tab"
                    />
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">⚙️</div>
                        <h3>No ARM Assembly Available</h3>
                        <p>The ARMv7 assembly code will appear here after compilation.</p>
                    </div>
                );

            case 'output':
                return artifacts.output ? (
                    <OutputTab output={artifacts.output.split('\n')} />
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">📺</div>
                        <h3>No Program Output</h3>
                        <p>The program output will appear here after execution.</p>
                    </div>
                );

            default:
                return null;
        }
    };

    return (
        <div className="results-view">
            <div className="results-header">
                <button className="back-btn" onClick={onBack}>
                    <span className="back-arrow">←</span>
                    Back to Terminal
                </button>
                <h2>Compilation Results</h2>
            </div>

            <div className="code-preview">
                <div className="preview-header">Source Code</div>
                <pre className="source-code">{code}</pre>
            </div>

            <div className="tabs-container">
                <div className="tabs-scroll">
                    {tabs.map(t => (
                        <button
                            key={t.id}
                            className={`tab-btn ${active === t.id ? 'active' : ''}`}
                            onClick={() => setActive(t.id)}
                        >
                            <span className="tab-icon">{t.icon}</span>
                            {t.label}
                        </button>
                    ))}
                </div>
            </div>

            <div className="tab-content">
                {renderTabContent()}
            </div>
        </div>
    );
};

export default ResultsView;