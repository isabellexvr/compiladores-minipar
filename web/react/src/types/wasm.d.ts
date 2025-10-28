declare global {
    interface Window {
        createCompilerModule?: (cfg: Record<string, any>) => Promise<any>;
    }
}
export { };
