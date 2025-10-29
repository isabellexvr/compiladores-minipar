import React, { useCallback, useMemo } from 'react';
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
  Position
} from 'reactflow';
import 'reactflow/dist/style.css';
import './ASTVisualizer.css';

interface ASTVisualizerProps {
  astString: string;
}

// Node customizado para AST - DEFINIDO FORA DO COMPONENTE
const AstNode = ({ data }: { data: any }) => (
  <div className={`ast-node ast-node-${data.nodeType}`}>
    {/* Handle de entrada (para arestas que chegam a este nó) */}
    <Handle type="target" position={Position.Top} id="input" />

    <div className="ast-node-label">{data.label}</div>

    {/* Handle de saída (para arestas que saem deste nó) */}
    <Handle type="source" position={Position.Bottom} id="output" />
  </div>
);

// nodeTypes DEFINIDO FORA DO COMPONENTE (useMemo para cache)
const useNodeTypes = (): NodeTypes => {
  return useMemo(() => ({
    astNode: AstNode,
  }), []);
};

const ASTVisualizer: React.FC<ASTVisualizerProps> = ({ astString }) => {
  const [nodes, setNodes, onNodesChange] = useNodesState([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState([]);
  const nodeTypes = useNodeTypes();

  const onConnect = useCallback(
    (params: Connection) => setEdges((eds) => addEdge(params, eds)),
    [setEdges]
  );

  // Função para converter a string AST em nodes/edges do React Flow
  const parseASTToFlow = useCallback((astStr: string) => {
    if (!astStr?.trim()) {
      setNodes([]);
      setEdges([]);
      return;
    }

    const newNodes: Node[] = [];
    const newEdges: Edge[] = [];
    let nodeId = 0;

    const lines = astStr
      .split('\n')
      .map(l => l.replace(/\t/g, '    ')) // tab -> 4 spaces
      .filter(l => l.trim().length > 0);

    const stack: { node: Node; level: number }[] = [];

    const indentSize = 2; // muda pra 4 se teu AST usa 4 espaços por nível

    lines.forEach((line, index) => {
      const leadingSpaces = line.match(/^ */)?.[0].length ?? 0;
      const level = Math.floor(leadingSpaces / indentSize);
      const content = line.trim();

      let nodeType = 'default';
      let label = content;

      if (content.endsWith(':')) {
        nodeType = content.replace(':', '').toLowerCase().replace(/\s+/g, '-');
        label = content.slice(0, -1);
      } else if (content.includes(':')) {
        const [type, value] = content.split(':').map(s => s.trim());
        nodeType = type.toLowerCase().replace(/\s+/g, '-');
        label = value ? `${type}: ${value}` : type;
      }

      const node: Node = {
        id: `node-${nodeId++}`,
        type: 'astNode',
        position: { x: level * 250, y: index * 100 },
        data: { label, nodeType },
      };

      newNodes.push(node);

      // Conecta com o pai de nível imediatamente menor
      while (stack.length > 0 && stack[stack.length - 1].level >= level) {
        stack.pop();
      }

      if (stack.length > 0) {
        const parent = stack[stack.length - 1];
        newEdges.push({
          id: `edge-${parent.node.id}-${node.id}`,
          source: parent.node.id,
          target: node.id,
          sourceHandle: 'output', // 👈 handle de saída do pai
          targetHandle: 'input',  // 👈 handle de entrada do filho
          type: 'smoothstep',
          style: { stroke: '#94a3b8', strokeWidth: 2 },
        });
      }

      stack.push({ node, level });
    });

    setNodes(newNodes);
    setEdges(newEdges);
  }, [setNodes, setEdges]);


  React.useEffect(() => {
    parseASTToFlow(astString);
  }, [astString, parseASTToFlow]);

  //console.log(nodes, edges);

  edges.forEach(e => {
  if (!e.sourceHandle || !e.targetHandle)
    console.warn('⚠️ Edge com handle indefinido:', e);
});


  return (
    <div className="ast-visualizer">
      <div className="visualizer-header">
        <h3>Abstract Syntax Tree</h3>
        <div className="visualizer-controls">
          <button onClick={() => parseASTToFlow(astString)}>Refresh Layout</button>
        </div>
      </div>
      
      {/* CONTAINER COM DIMENSÕES EXPLÍCITAS */}
      <div className="flow-container" style={{ width: '100%', height: '500px' }}>
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onConnect={onConnect}
          nodeTypes={nodeTypes}
          fitView
          attributionPosition="bottom-left"
        >
          <Controls />
          <MiniMap 
            nodeColor="#e2e8f0"
            maskColor="rgba(0, 0, 0, 0.1)"
          />
          <Background variant="dots" gap={20} size={1} />
        </ReactFlow>
      </div>
    </div>
  );
};

export default ASTVisualizer;