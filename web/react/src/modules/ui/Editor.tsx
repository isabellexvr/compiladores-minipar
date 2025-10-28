import React from 'react';

interface EditorProps {
    value: string;
    onChange: (val: string) => void;
}

const Editor: React.FC<EditorProps> = ({ value, onChange }) => {
    return (
        <textarea
            className="code-editor"
            value={value}
            onChange={e => onChange(e.target.value)}
            spellCheck={false}
        />
    );
};

export default Editor;
