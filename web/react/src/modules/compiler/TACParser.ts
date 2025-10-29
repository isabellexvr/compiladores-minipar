// TACParser.ts - novo arquivo
export interface TACInstruction {
    result: string;
    op: string;
    arg1: string;
    arg2: string;
    type: string;
}

export function parseTACFromText(tacText: string): TACInstruction[] {
    if (!tacText) return [];
    
    const lines = tacText.split('\n')
        .map(line => line.trim())
        .filter(line => line.length > 0);
    
    const instructions: TACInstruction[] = [];
    
    for (const line of lines) {
        if (line.endsWith(':')) {
            // Label: L1:
            instructions.push({
                result: line.replace(':', ''),
                op: 'label',
                arg1: '',
                arg2: '',
                type: 'LABEL'
            });
        } else if (line.startsWith('print ')) {
            // Print: print resultado
            const arg1 = line.replace('print', '').trim();
            instructions.push({
                result: '',
                op: 'print',
                arg1: arg1,
                arg2: '',
                type: 'PRINT'
            });
        } else if (line.startsWith('if_false ')) {
            // Conditional jump: if_false t1 goto L2
            const match = line.match(/if_false\s+(\S+)\s+goto\s+(\S+)/);
            if (match) {
                instructions.push({
                    result: '',
                    op: 'if_false',
                    arg1: match[1],
                    arg2: match[2],
                    type: 'CONDITIONAL_JUMP'
                });
            }
        } else if (line.startsWith('goto ')) {
            // Jump: goto L1
            const arg2 = line.replace('goto', '').trim();
            instructions.push({
                result: '',
                op: 'goto',
                arg1: '',
                arg2: arg2,
                type: 'JUMP'
            });
        } else if (line.includes(' = ')) {
            const [left, right] = line.split(' = ').map(s => s.trim());
            
            if (right.includes(' + ') || right.includes(' - ') || 
                right.includes(' * ') || right.includes(' / ') ||
                right.includes(' == ') || right.includes(' != ') ||
                right.includes(' < ') || right.includes(' <= ') ||
                right.includes(' > ') || right.includes(' >= ')) {
                
                // Binary operation: t0 = x + y
                const operators = ['+', '-', '*', '/', '==', '!=', '<', '<=', '>', '>='];
                let operator = '';
                let parts: string[] = [];
                
                for (const op of operators) {
                    if (right.includes(` ${op} `)) {
                        operator = op;
                        parts = right.split(` ${op} `).map(s => s.trim());
                        break;
                    }
                }
                
                if (operator && parts.length === 2) {
                    instructions.push({
                        result: left,
                        op: operator,
                        arg1: parts[0],
                        arg2: parts[1],
                        type: 'BINARY_OP'
                    });
                }
            } else {
                // Simple assignment: x = 10 or resultado = t0
                instructions.push({
                    result: left,
                    op: '',
                    arg1: right,
                    arg2: '',
                    type: 'ASSIGNMENT'
                });
            }
        }
    }
    
    return instructions;
}