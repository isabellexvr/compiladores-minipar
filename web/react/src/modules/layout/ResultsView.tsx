import React, { useState } from 'react';
import TokensTab from '../tabs/TokensTab';
import ASTVisualizer from '../tabs/ASTVisualizer';
import './ResultsView.css';

export interface CompilationArtifacts {
    rawOutput: string;
    tokens: string[];
    syntaxTree?: string;
    symbolTable?: string;
    tac?: string;
    arm?: string;
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
    { id: 'arm', label: 'ARMv7 Assembly', icon: '⚙️' }
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
            
            case 'symbols':
                return artifacts.symbolTable ? (
                    <div className="code-output">
                        <pre>{artifacts.symbolTable}</pre>
                    </div>
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">📊</div>
                        <h3>No Symbol Table Available</h3>
                        <p>The symbol table will appear here after compilation.</p>
                    </div>
                );
            
            case 'tac':
                return artifacts.tac ? (
                    <div className="code-output">
                        <pre>{artifacts.tac}</pre>
                    </div>
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">🔢</div>
                        <h3>No 3-Address Code Available</h3>
                        <p>The three-address code will appear here after compilation.</p>
                    </div>
                );
            
            case 'arm':
                return artifacts.arm ? (
                    <div className="code-output">
                        <pre>{artifacts.arm}</pre>
                    </div>
                ) : (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">⚙️</div>
                        <h3>No ARM Assembly Available</h3>
                        <p>The ARMv7 assembly code will appear here after compilation.</p>
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