import React, { useState } from 'react';
import TokensTab from '../tabs/TokensTab';
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
    { id: 'syntax', label: 'Syntax', icon: '🌳' },
    { id: 'symbols', label: 'Symbols', icon: '📊' },
    { id: 'tac', label: '3 Address', icon: '🔢' },
    { id: 'arm', label: 'ARMv7', icon: '⚙️' }
];

const ResultsView: React.FC<ResultsViewProps> = ({ code, artifacts, onBack }) => {
    const [active, setActive] = useState<string>('tokens');

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
                {active === 'tokens' && <TokensTab tokens={artifacts.tokens} sourceCode={code} />}
                {active !== 'tokens' && (
                    <div className="placeholder-content">
                        <div className="placeholder-icon">🚧</div>
                        <h3>Under Construction</h3>
                        <p>This compilation phase visualization is coming soon!</p>
                    </div>
                )}
            </div>
        </div>
    );
};

export default ResultsView;