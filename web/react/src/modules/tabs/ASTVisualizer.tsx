// ASTVisualizer.tsx - VERSÃO CORRIGIDA
import React, { useCallback, useMemo, useRef, useLayoutEffect, useState } from 'react';
import ReactFlow, {
  Node,
  Edge,
  addEdge,
  Connection,
  useNodesState,
  useEdgesState,
  Controls,
  Background,
  MiniMap,
  NodeTypes,
  Handle,
  Position,
  ReactFlowProvider
} from 'reactflow';
import 'reactflow/dist/style.css';
import './ASTVisualizer.css';

interface ASTVisualizerProps {
  astString: string;
}

// ✅ NodeTypes DEFINIDO FORA DO COMPONENTE (para evitar recriação)
const AstNode = ({ data }: { data: any }) => (
  <div className={`ast-node ast-node-${data.nodeType}`}>
    <Handle type="target" position={Position.Top} id="input" />
    <div className="ast-node-label">{data.label}</div>
    <Handle type="source" position={Position.Bottom} id="output" />
  </div>
);

// ✅ NODETYPES MEMOIZADO FORA DO COMPONENTE
const nodeTypes: NodeTypes = {
  astNode: AstNode,
};

// Componente interno do Flow
const FlowWrapper: React.FC<{ 
  nodes: Node[]; 
  edges: Edge[]; 
  onNodesChange: any;
  onEdgesChange: any;
  onConnect: any;
}> = ({ nodes, edges, onNodesChange, onEdgesChange, onConnect }) => {
  const flowRef = useRef<HTMLDivElement>(null);
  const [dimensions, setDimensions] = useState({ width: 0, height: 0 });

  useLayoutEffect(() => {
    if (!flowRef.current) return;
    
    const updateDimensions = () => {
      const rect = flowRef.current!.getBoundingClientRect();
      setDimensions({
        width: rect.width,
        height: Math.max(rect.height, 500) // Altura mínima de 500px
      });
    };

    updateDimensions();
    
    const resizeObserver = new ResizeObserver(updateDimensions);
    resizeObserver.observe(flowRef.current);
    
    return () => resizeObserver.disconnect();
  }, []);

  return (
    <div 
      ref={flowRef} 
      className="flow-container"
      style={{ width: '100%', height: '100%' }}
    >
      {dimensions.width > 0 && dimensions.height > 0 && (
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onConnect={onConnect}
          nodeTypes={nodeTypes}
          fitView
          minZoom={0.2}
          maxZoom={1.5}
          style={{ width: dimensions.width, height: dimensions.height }}
        >
          <Controls />
          <MiniMap
            nodeColor={(node) => {
              const colors: { [key: string]: string } = {
                'program': '#6366f1',
                'seq': '#10b981', 
                'while': '#f59e0b',
                'assignment': '#ef4444',
                'binaryop': '#8b5cf6',
                'identifier': '#3b82f6',
                'number': '#22c55e',
                'default': '#94a3b8'
              };
              return colors[node.data.nodeType] || colors.default;
            }}
            maskColor="rgba(0, 0, 0, 0.1)"
          />
          <Background gap={25} size={1} />
        </ReactFlow>
      )}
    </div>
  );
};

const ASTVisualizer: React.FC<ASTVisualizerProps> = ({ astString }) => {
  const [nodes, setNodes, onNodesChange] = useNodesState([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState([]);

  const onConnect = useCallback(
    (params: Connection) => setEdges((eds) => addEdge(params, eds)),
    [setEdges]
  );

  // ✅ FUNÇÃO CORRIGIDA para parse da AST
  const parseASTToFlow = useCallback((astStr: string) => {
    if (!astStr?.trim()) {
      setNodes([]);
      setEdges([]);
      return;
    }


    // ✅ CORREÇÃO: Manter os espaços originais para debug
    const lines = astStr
      .split('\n')
      .filter(line => line.trim().length > 0);


    const newNodes: Node[] = [];
    const newEdges: Edge[] = [];
    let nodeId = 0;

    const stack: { node: Node; level: number }[] = [];
    let currentY = 0;

    lines.forEach((originalLine) => {
      // ✅ CORREÇÃO: Contar espaços ANTES do trim()
      const leadingSpaces = originalLine.match(/^ */)?.[0].length || 0;
      const level = Math.floor(leadingSpaces / 2); // 2 espaços por nível
      
      const content = originalLine.trim();


      // Determinar tipo de nó
      let nodeType = 'default';
      let label = content;

      if (content.endsWith(':')) {
        nodeType = content.slice(0, -1).toLowerCase().replace(/\s+/g, '-');
        label = content.slice(0, -1);
      } else if (content.includes(':')) {
        const colonIndex = content.indexOf(':');
        const type = content.substring(0, colonIndex).trim();
        const value = content.substring(colonIndex + 1).trim();
        
        nodeType = type.toLowerCase().replace(/\s+/g, '-');
        label = value ? `${type}: ${value}` : type;
      } else {
        nodeType = content.toLowerCase().replace(/\s+/g, '-');
        label = content;
      }

      // Criar nó com posição baseada no nível
      const node: Node = {
        id: `node-${nodeId++}`,
        type: 'astNode',
        position: { 
          x: level * 300, // Mais espaço para níveis mais profundos
          y: currentY 
        },
        data: { 
          label, 
          nodeType,
          originalContent: originalLine,
          level: level // Para debug
        },
      };

      newNodes.push(node);
      currentY += 100;

      // ✅ CORREÇÃO: Conectar com pai - lógica corrigida
      while (stack.length > 0 && stack[stack.length - 1].level >= level) {
        stack.pop();
      }

      if (stack.length > 0) {
        const parent = stack[stack.length - 1];
        newEdges.push({
          id: `edge-${parent.node.id}-${node.id}`,
          source: parent.node.id,
          target: node.id,
          sourceHandle: 'output',
          targetHandle: 'input',
          type: 'smoothstep',
          style: { 
            stroke: getEdgeColor(parent.node.data.nodeType),
            strokeWidth: 2 
          },
        });
        
        
  } else {
        
      }

      stack.push({ node, level });
    });

    

    setNodes(newNodes);
    setEdges(newEdges);
  }, [setNodes, setEdges]);

  // ✅ Função auxiliar para cores das arestas
  const getEdgeColor = (parentType: string): string => {
    const colors: { [key: string]: string } = {
      'program': '#6366f1',
      'seq': '#10b981',
      'while': '#f59e0b',
      'assignment': '#ef4444',
      'binaryop': '#8b5cf6',
      'default': '#94a3b8'
    };
    return colors[parentType] || colors.default;
  };

  // ✅ Estado de loading/error
  const [isLoading, setIsLoading] = React.useState(false);
  const [error, setError] = React.useState<string | null>(null);

  React.useEffect(() => {
    setIsLoading(true);
    setError(null);
    
    try {
      parseASTToFlow(astString);
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Erro desconhecido';
      setError(`Erro ao processar AST: ${errorMsg}`);
      console.error('❌ Erro no ASTVisualizer:', err);
    } finally {
      setIsLoading(false);
    }
  }, [astString, parseASTToFlow]);

  return (
    <div className="ast-visualizer">
      <div className="visualizer-header">
        <h3>Abstract Syntax Tree</h3>
        <div className="visualizer-info">
          <span className="node-count">{nodes.length} nodes</span>
          <span className="edge-count">{edges.length} connections</span>
        </div>
        <div className="visualizer-controls">
          <button 
            onClick={() => {
              parseASTToFlow(astString);
            }}
            disabled={isLoading}
          >
            {isLoading ? 'Processando...' : 'Refresh Layout'}
          </button>
        </div>
      </div>

      {error && (
        <div className="visualizer-error">
          <div className="error-icon">⚠️</div>
          <div className="error-message">{error}</div>
        </div>
      )}

      {isLoading ? (
        <div className="visualizer-loading">
          <div className="loading-spinner"></div>
          <p>Processando árvore sintática...</p>
        </div>
      ) : nodes.length === 0 ? (
        <div className="visualizer-empty">
          <div className="empty-icon">🌳</div>
          <h4>Nenhuma árvore para exibir</h4>
          <p>A árvore sintática aparecerá aqui após a compilação.</p>
        </div>
      ) : (
        // ✅ ENVOLVER COM ReactFlowProvider
        <ReactFlowProvider>
          <FlowWrapper
            nodes={nodes}
            edges={edges}
            onNodesChange={onNodesChange}
            onEdgesChange={onEdgesChange}
            onConnect={onConnect}
          />
        </ReactFlowProvider>
      )}
    </div>
  );
};

export default ASTVisualizer;