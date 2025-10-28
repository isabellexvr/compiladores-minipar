import React, { createContext, useContext, useEffect, useState, useCallback } from 'react';

interface CompilerContextValue {
    ready: boolean;
    compiling: boolean;
    compile: (code: string) => Promise<string>;
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
            const ptr = moduleRef._compile_minipar(code);
            if (!ptr) throw new Error('Ponteiro nulo retornado');
            const result = moduleRef.UTF8ToString(ptr);
            if (moduleRef._free_string) moduleRef._free_string(ptr);
            return result as string;
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
            const ptr = moduleRef._compile_minipar_json(code);
            if (!ptr) throw new Error('Ponteiro nulo retornado');
            const result = moduleRef.UTF8ToString(ptr);
            if (moduleRef._free_string) moduleRef._free_string(ptr);
            
            // Parse do JSON
            const parsed = JSON.parse(result);
            return parsed;
        } catch (e: any) {
            setError(e.message || String(e));
            throw e;
        } finally {
            setCompiling(false);
        }
    }, [moduleRef]);

    const value: CompilerContextValue = { ready, compiling, compile, error };
    return <CompilerContext.Provider value={value}>{children}</CompilerContext.Provider>;
};

export function useCompiler() {
    const ctx = useContext(CompilerContext);
    if (!ctx) throw new Error('useCompiler deve ser usado dentro de CompilerProvider');
    return ctx;
}

