// src/tabs/OutputTab.tsx
import React from 'react';

interface OutputTabProps {
    output: string[];
}

const OutputTab: React.FC<OutputTabProps> = ({ output }) => {
    console.log(output)
    return (
        <div className="output-tab">
            <div className="terminal-header">
                <span className="terminal-title">Program Output</span>
            </div>
            <div className="terminal-content">
                {output.length > 0 ? (
                    output.map((line, index) => (
                        <div key={index} className="output-line">
                            {line}
                        </div>
                    ))
                ) : (
                    <div className="no-output">No output generated</div>
                )}
            </div>
        </div>
    );
};

export default OutputTab;