import React from 'react';

interface Props {
    ready: boolean;
    compiling: boolean;
    error: string | null;
}

const StatusBar: React.FC<Props> = ({ ready, compiling, error }) => {
    return (
        <div className="status-bar">
            {!ready && !error && 'Carregando WebAssembly...'}
            {ready && !compiling && !error && 'Pronto para compilar'}
            {compiling && 'Compilando...'}
            {error && <span className="error">Erro: {error}</span>}
        </div>
    );
};

export default StatusBar;
