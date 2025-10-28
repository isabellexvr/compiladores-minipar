import React from 'react';

const OutputPanel: React.FC<{ text: string }> = ({ text }) => {
    return <pre className="output-panel">{text}</pre>;
};

export default OutputPanel;
