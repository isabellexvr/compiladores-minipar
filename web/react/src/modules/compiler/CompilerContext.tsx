import React, { createContext, useContext, useEffect, useState, useCallback } from 'react';

interface CompilerContextValue {
    ready: boolean;
    compiling: boolean;
    compile: (code: string) => Promise<string>;
    compileTAC: (code: string) => Promise<any>;
    error: string | null;
}

const CompilerContext = createContext<CompilerContextValue | undefined>(undefined);

// global module reference
declare global {
    interface Window {
        createCompilerModule?: (cfg: Record<string, any>) => Promise<any>;
    }
}

export const CompilerProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
    const [ready, setReady] = useState(false);
    const [compiling, setCompiling] = useState(false);
    const [error, setError] = useState<string | null>(null);
    const [moduleRef, setModuleRef] = useState<any>(null);

    useEffect(() => {
        let cancelled = false;
        const load = async () => {
            setError(null);
            try {
                if (!window.createCompilerModule) {
                    console.warn('createCompilerModule não encontrado ainda.');
                }
                const moduleConfig = {
                    onRuntimeInitialized() {
                        if (!cancelled) setReady(true);
                    },
                    print(text: string) { console.log('[wasm]', text); },
                    printErr(text: string) { console.error('[wasm-err]', text); }
                };
                const wasmModule = await window.createCompilerModule?.(moduleConfig);
                if (!wasmModule) throw new Error('Falha ao inicializar módulo WASM');
                if (!cancelled) setModuleRef(wasmModule);
            } catch (e: any) {
                if (!cancelled) setError(e.message || String(e));
            }
        };
        load();
        return () => { cancelled = true; };
    }, []);

    const compile = useCallback(async (code: string) => {
        if (!moduleRef) throw new Error('Módulo não carregado');
        setCompiling(true);
        setError(null);

        try {
            // 🔧 Usa cwrap para converter string JS → C automaticamente
            const compileFn = moduleRef.cwrap('compile_minipar', 'number', ['string']);
            const ptr = compileFn(code); // retorna ponteiro para char*
            const result = moduleRef.UTF8ToString(ptr);
            moduleRef._free_string(ptr); // libera memória alocada no C++
            return result;
        } catch (e: any) {
            setError(e.message || String(e));
            throw e;
        } finally {
            setCompiling(false);
        }
    }, [moduleRef]);


    const compileJson = useCallback(async (code: string) => {
        if (!moduleRef) throw new Error('Módulo não carregado');
        setCompiling(true);
        setError(null);

        try {
            const compileJsonFn = moduleRef.cwrap('compile_minipar_json', 'number', ['string']);
            const ptr = compileJsonFn(code);
            const result = moduleRef.UTF8ToString(ptr);
            moduleRef._free_string(ptr);
            return JSON.parse(result);
        } catch (e: any) {
            setError(e.message || String(e));
            throw e;
        } finally {
            setCompiling(false);
        }
    }, [moduleRef]);

    // No CompilerContext.tsx, adicione esta função:
    // No CompilerContext.tsx, adicione:
    const compileTAC = useCallback(async (code: string) => {
        if (!moduleRef) throw new Error('Módulo não carregado');
        setCompiling(true);
        setError(null);

        try {
            const compileTACFn = moduleRef.cwrap('compile_minipar_tac', 'number', ['string']);
            const ptr = compileTACFn(code);
            const result = moduleRef.UTF8ToString(ptr);
            moduleRef._free_string(ptr);
            const parsed = JSON.parse(result);

            if (parsed.success) {
                return parsed.tac; // Retorna apenas o array de instruções TAC
            } else {
                throw new Error(parsed.error || 'Unknown error in TAC generation');
            }
        } catch (e: any) {
            setError(e.message || String(e));
            throw e;
        } finally {
            setCompiling(false);
        }
    }, [moduleRef]);

    // Atualize o contexto para incluir a nova função
    const value: CompilerContextValue = { ready, compiling, compile, compileTAC, error };
    return <CompilerContext.Provider value={value}>{children}</CompilerContext.Provider>;
};

export function useCompiler() {
    const ctx = useContext(CompilerContext);
    if (!ctx) throw new Error('useCompiler deve ser usado dentro de CompilerProvider');
    return ctx;
}

