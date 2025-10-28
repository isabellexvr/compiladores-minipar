import React, { useState } from 'react';
import { CompilerProvider } from './compiler/CompilerContext';
import Header from './layout/Header';
import Home from './layout/Home';
import ResultsView, { CompilationArtifacts } from './layout/ResultsView';
import '../styles/globals.css';

const App: React.FC = () => {
    const [code, setCode] = useState<string>('SEQ\nx = 10\ny = 20\nresultado = x + y\nprint resultado');
    const [artifacts, setArtifacts] = useState<CompilationArtifacts | null>(null);
    const [view, setView] = useState<'home' | 'results'>('home');

    return (
        <CompilerProvider>
            <div className="app-container">
                <Header onLogoClick={() => setView('home')} />
                {view === 'home' && (
                    <Home
                        code={code}
                        onChange={setCode}
                        onCompile={(a) => { setArtifacts(a); setView('results'); }}
                    />
                )}
                {view === 'results' && artifacts && (
                    <ResultsView code={code} artifacts={artifacts} onBack={() => setView('home')} />
                )}
            </div>
        </CompilerProvider>
    );
};

export default App;