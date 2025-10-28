import React, { useState } from 'react';
import { useCompiler } from '../compiler/CompilerContext';
import Editor from '../ui/Editor';
import OutputPanel from '../ui/OutputPanel';
import StatusBar from '../ui/StatusBar';

const starter = `SEQ\nx = 10\ny = 20\nresultado = x + y\nprint resultado`;

const IDELayout: React.FC = () => {
    const { ready, compile, compiling, error } = useCompiler();
    const [code, setCode] = useState(starter);
    const [result, setResult] = useState('Aguardando compilação...');

    const handleCompile = async () => {
        if (!ready) return;
        try {
            setResult('Compilando...');
            const out = await compile(code);
            setResult(out || 'Sem saída');
        } catch (e: any) {
            setResult('Erro: ' + (e.message || String(e)));
        }
    };

    return (
        <div className="ide-root">
            <header>
                <h1>MiniPar IDE</h1>
            </header>
            <StatusBar ready={ready} compiling={compiling} error={error} />
            <div className="panes">
                <div className="pane editor-pane">
                    <Editor value={code} onChange={setCode} />
                    <button className="compile-btn" disabled={!ready || compiling} onClick={handleCompile}>Compilar</button>
                </div>
                <div className="pane output-pane">
                    <OutputPanel text={result} />
                </div>
            </div>
        </div>
    );
};

export default IDELayout;
