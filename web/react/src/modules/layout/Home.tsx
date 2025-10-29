import React, { useState } from 'react';
import { Zap, FileText, TreePine, Table, Binary, Cpu } from 'lucide-react';
import { useCompiler } from '../compiler/CompilerContext';
import { buildArtifactsFromRaw } from '../compiler/buildArtifacts';
import type { CompilationArtifacts } from './ResultsView';
import './Home.css';

interface HomeProps {
    code: string;
    onChange: (c: string) => void;
    onCompile: (artifacts: CompilationArtifacts) => void;
}

const Home: React.FC<HomeProps> = ({ code, onChange, onCompile }) => {
    const { ready, compile, compiling, error } = useCompiler();
    const [localError, setLocalError] = useState<string | null>(null);

    const handleRun = async () => {
        if (!ready) return;
        setLocalError(null);
        try {
            const raw = await compile(code);
            console.log("RAW OUTPUT:", raw); // ← ADICIONE ESTA LINHA
            const artifacts = buildArtifactsFromRaw(code, raw);
            onCompile(artifacts);
        } catch (e: any) {
            setLocalError(e.message || String(e));
        }
    };

    const handleKeyDown = (e: React.KeyboardEvent) => {
        if (e.ctrlKey && e.key === 'Enter') {
            handleRun();
        }
    };

    return (
        <main className="home-container">
            <div className="terminal-section">
                <div className="terminal-window">
                    <div className="terminal-header">
                        <div className="terminal-dots">
                            <div className="dot red"></div>
                            <div className="dot yellow"></div>
                            <div className="dot green"></div>
                        </div>
                        <div className="terminal-title">minipar-terminal</div>
                    </div>
                    <div className="terminal-body">
                        <div className="prompt-row">
                            <span className="prompt-prefix">user@minipar:$</span>
                            <div className="prompt-input-wrapper">
                                <textarea
                                    className="terminal-input inline"
                                    value={code}
                                    onChange={e => onChange(e.target.value)}
                                    onKeyDown={handleKeyDown}
                                    placeholder="Digite seu código MiniPar aqui..."
                                    spellCheck={false}
                                    rows={12}
                                />
                            </div>
                        </div>
                        <div className="terminal-actions">
                            <button
                                className={`compile-btn ${compiling ? 'compiling' : ''}`}
                                disabled={!ready || compiling}
                                onClick={handleRun}
                            >
                                <span className="btn-icon">⚡</span>
                                {compiling ? 'COMPILING...' : 'COMPILE'}
                                <span className="btn-hint">Ctrl+Enter</span>
                            </button>
                        </div>
                    </div>
                </div>

                {(error || localError) && (
                    <div className="error-terminal">
                        <div className="error-header">ERROR</div>
                        <div className="error-message">{error || localError}</div>
                    </div>
                )}
            </div>

            <div className="info-section">
                <div className="info-card">
                    <h2 className="info-title"><Zap size={20} className="icon accent" /> MINIPAR COMPILER</h2>
                    <p className="info-description">
                        Insira código na linguagem <span className="highlight">MiniPar</span> e compile para ver todo o processo de compilação em ação.
                    </p>

                    <div className="features-grid">
                        <div className="feature">
                            <FileText size={18} className="feature-icon" />
                            <div className="feature-text">Análise Léxica</div>
                        </div>
                        <div className="feature">
                            <TreePine size={18} className="feature-icon" />
                            <div className="feature-text">Árvore Sintática</div>
                        </div>
                        <div className="feature">
                            <Table size={18} className="feature-icon" />
                            <div className="feature-text">Tabela de Símbolos</div>
                        </div>
                        <div className="feature">
                            <Binary size={18} className="feature-icon" />
                            <div className="feature-text">Código 3 Endereços</div>
                        </div>
                        <div className="feature">
                            <Cpu size={18} className="feature-icon" />
                            <div className="feature-text">ARMv7 Assembly</div>
                        </div>
                    </div>

                    <div className="quick-tips">
                        <h3>💡 Quick Start</h3>
                        <code>SEQ</code>
                        <code>x = 10 + 5</code>
                        <code>print x</code>
                    </div>
                </div>
            </div>
        </main>
    );
};

export default Home;