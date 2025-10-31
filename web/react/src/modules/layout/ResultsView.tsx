import React, { useState } from 'react';
import TokensTab from '../tabs/TokensTab';
import ASTVisualizer from '../tabs/ASTVisualizer';
import './ResultsView.css';
import TACTab from '../tabs/TACTab';
import SymbolTableTab from '../tabs/SymbolTableTab';
import ARMTab from '../tabs/ARMTab';
import OutputTab from '../tabs/OutputTab';
import { parseTACFromText } from '../compiler/TACParser';

export interface Token {
    type: string;
    value: string;
    line: number;
    column: number;
}

export interface CompilationArtifacts {
    rawOutput: string;
    tokens: Token[];
    syntaxTree?: string;
    symbolTable?: string;
    tac?: string;
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

            case 'symbols': {
                if (!artifacts.symbolTable) {
                    return (
                        <div className="placeholder-content">
                            <div className="placeholder-icon">📊</div>
                            <h3>No Symbol Table Available</h3>
                        </div>
                    );
                }
                // Parse lines ignoring header bar separators
                const lines = artifacts.symbolTable.split('\n').map(l => l.trim()).filter(l => l.length > 0);
                const dataStart = lines.findIndex(l => l.startsWith('Nome')); // header line
                const entries = lines.slice(dataStart + 2) // skip header + separator
                    .map(line => {
                        const parts = line.split('|').map(p => p.trim());
                        return { name: parts[0], type: parts[1], scope: 'global', value: parts[2] };
                    })
                    .filter(e => e.name && e.type);
                return <SymbolTableTab table={entries} />;
            }

            case 'tac':
                return artifacts.tac ? (
                    <TACTab 
                        instructions={parseTACFromText(artifacts.tac)}
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