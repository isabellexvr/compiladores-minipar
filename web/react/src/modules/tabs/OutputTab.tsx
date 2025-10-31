// src/tabs/OutputTab.tsx
import React from 'react';
import './OutputTab.css';

interface OutputTabProps {
    output: string[];
}

const OutputTab: React.FC<OutputTabProps> = ({ output }) => {
    console.log('📤 Output recebido:', output);

    // ✅ Processar o output para lidar com strings e arrays
    const processedOutput = Array.isArray(output) ? output : [output];
    const hasOutput = processedOutput.length > 0 && processedOutput.some(line => line && line.trim().length > 0);

    return (
        <div className="output-tab">
            {/* Header do Terminal */}
            <div className="terminal-header">
                <div className="terminal-controls">
                    <div className="control-dots">
                        <span className="control-dot red"></span>
                        <span className="control-dot yellow"></span>
                        <span className="control-dot green"></span>
                    </div>
                </div>
                <span className="terminal-title">Program Output</span>
                <div className="terminal-status">
                    <span className="status-indicator"></span>
                    {hasOutput ? 'Execution Complete' : 'No Output'}
                </div>
            </div>

            {/* Conteúdo do Terminal */}
            <div className="terminal-content">
                {hasOutput ? (
                    <div className="output-lines">
                        {/* Linha de prompt simulada */}
                        <div className="output-line prompt-line">
                            <span className="prompt-symbol">➜</span>
                            <span className="prompt-text">./minipar-program</span>
                        </div>
                        
                        {/* Output do programa */}
                        {processedOutput.map((line, index) => (
                            line && line.trim().length > 0 ? (
                                <div key={index} className="output-line">
                                    <span className="output-text">{line}</span>
                                </div>
                            ) : null
                        ))}
                        
                        {/* Linha de status final */}
                        <div className="output-line status-line">
                            <span className="status-symbol">⏎</span>
                            <span className="status-text">
                                Program exited with {processedOutput.length > 0 ? 'output' : 'no output'}
                            </span>
                        </div>
                    </div>
                ) : (
                    <div className="no-output">
                        <div className="no-output-icon">📭</div>
                        <h3>No Program Output</h3>
                        <p>The program executed but produced no output.</p>
                        <div className="no-output-tips">
                            <strong>Tips:</strong>
                            <ul>
                                <li>Use <code>print</code> statements to display values</li>
                                <li>Check if variables have assigned values</li>
                                <li>Verify program logic and conditions</li>
                            </ul>
                        </div>
                    </div>
                )}
            </div>

            {/* Footer do Terminal */}
            <div className="terminal-footer">
                <div className="footer-stats">
                    <span className="stat">Lines: {processedOutput.filter(line => line && line.trim().length > 0).length}</span>
                    <span className="stat">Status: {hasOutput ? '✅ Success' : '⚠️ No Output'}</span>
                </div>
                <div className="footer-actions">
                    <button 
                        className="copy-btn"
                        onClick={() => {
                            navigator.clipboard.writeText(processedOutput.join('\n'));
                        }}
                        disabled={!hasOutput}
                    >
                        📋 Copy Output
                    </button>
                </div>
            </div>
        </div>
    );
};

export default OutputTab;