// SymbolTableTab.tsx - versão adaptada para o JSON
import React from 'react';
import './SymbolTableTab.css';

interface SymbolEntryRaw {
  name: string;
  symbolType?: string; // frontend esperado
  type?: string;       // backend legado
  dataType?: string;
  scope?: string;
  value?: string;
}

interface SymbolTableTabProps {
  table: SymbolEntryRaw[];
}

const SymbolTableTab: React.FC<SymbolTableTabProps> = ({ table }) => {
  // Normaliza entradas vindas do backend ("type" -> "symbolType") e valores padrão
  const norm = (table || []).map(e => ({
    ...e,
    symbolType: (e.symbolType || e.type || 'UNKNOWN').toString(),
    dataType: e.dataType || '?',
    scope: e.scope || 'global'
  }));

  const getSymbolColor = (symbolType: string): string => {
    switch (symbolType.toUpperCase()) {
      case 'VAR':
      case 'VARIABLE': return '#4CAF50';      // Verde - variáveis
      case 'FUN':
      case 'FUNCTION': return '#2196F3';      // Azul - funções
      case 'CHAN':
      case 'CHANNEL': return '#FF9800';       // Laranja - canais
      case 'CONST': return '#9C27B0';         // Roxo - constantes
      default: return '#607D8B';              // Cinza - outros
    }
  };

  const getSymbolIcon = (symbolType: string): string => {
    switch (symbolType.toUpperCase()) {
      case 'VAR':
      case 'VARIABLE': return '📦';
      case 'FUN':
      case 'FUNCTION': return '⚡';
      case 'CHAN':
      case 'CHANNEL': return '🔗';
      case 'CONST': return '🔒';
      default: return '❓';
    }
  };

  const getScopeColor = (scope: string): string => {
    switch (scope.toLowerCase()) {
      case 'global': return '#F44336';        // Vermelho - global
      case 'local': return '#FF9800';         // Laranja - local
      case 'param': return '#2196F3';         // Azul - parâmetro
      default: return '#607D8B';              // Cinza - outros
    }
  };

  // ✅ ESTATÍSTICAS ATUALIZADAS
  const stats = {
    total: norm.length,
    variables: norm.filter(s => {
      const t = s.symbolType.toUpperCase();
      return t === 'VAR' || t === 'VARIABLE';
    }).length,
    functions: norm.filter(s => {
      const t = s.symbolType.toUpperCase();
      return t === 'FUN' || t === 'FUNCTION';
    }).length,
    channels: norm.filter(s => {
      const t = s.symbolType.toUpperCase();
      return t === 'CHAN' || t === 'CHANNEL';
    }).length,
    globals: norm.filter(s => s.scope.toLowerCase() === 'global').length
  };

  if (!norm || norm.length === 0) {
    return (
      <div className="symbol-table-container">
        <div className="symbol-empty">
          <div className="empty-icon">📊</div>
          <h3>No Symbol Table Available</h3>
          <p>The symbol table will appear here after compilation.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="symbol-table-container">
      {/* Header */}
      <div className="symbol-header">
        <div className="symbol-title-section">
          <h3>Symbol Table</h3>
          <p className="symbol-description">
            Tabela de símbolos contendo variáveis, funções e canais do programa
          </p>
        </div>
        <div className="symbol-stats">
          <span className="stat">{stats.total} símbolos</span>
          <span className="stat">{stats.variables} variáveis</span>
          <span className="stat">{stats.functions} funções</span>
        </div>
      </div>

      {/* Legenda */}
      <div className="symbol-legend">
        <div className="legend-title">Tipos de Símbolos:</div>
        <div className="legend-items">
          <span className="legend-item" style={{ '--color': '#4CAF50' } as any}>
            <span className="legend-icon">📦</span> Variável (VAR)
          </span>
          <span className="legend-item" style={{ '--color': '#2196F3' } as any}>
            <span className="legend-icon">⚡</span> Função (FUN)
          </span>
          <span className="legend-item" style={{ '--color': '#FF9800' } as any}>
            <span className="legend-icon">🔗</span> Canal (CHAN)
          </span>
          <span className="legend-item" style={{ '--color': '#9C27B0' } as any}>
            <span className="legend-icon">🔒</span> Constante (CONST)
          </span>
        </div>
      </div>

      {/* Informações */}
      <div className="symbol-info">
        <div className="info-section">
          <h4>📋 Sobre a Tabela de Símbolos:</h4>
          <p>
            A tabela de símbolos é gerada durante a análise semântica e contém informações
            sobre todos os identificadores do programa, incluindo tipo, escopo e tipo de dado.
          </p>
        </div>
      </div>

      {/* Tabela */}
      <div className="symbol-table-wrapper">
        <div className="table-header">
          <span className="table-title">Símbolos do Programa</span>
          <span className="table-subtitle">{stats.total} entradas</span>
        </div>
        <table className="symbol-table">
          <thead>
            <tr>
              <th className="col-icon"></th>
              <th className="col-name">Nome</th>
              <th className="col-type">Tipo Símbolo</th>
              <th className="col-datatype">Tipo Dado</th>
              <th className="col-scope">Escopo</th>
            </tr>
          </thead>
          <tbody>
            {norm.map((entry, idx) => (
              <tr key={idx} className="symbol-row">
                <td className="symbol-icon-cell">
                  <span
                    className="symbol-icon"
                    style={{ backgroundColor: getSymbolColor(entry.symbolType) }}
                  >
                    {getSymbolIcon(entry.symbolType)}
                  </span>
                </td>
                <td className="symbol-name">
                  <code>{entry.name}</code>
                </td>
                <td className="symbol-type">
                  <span
                    className="type-badge"
                    style={{ backgroundColor: getSymbolColor(entry.symbolType) }}
                  >
                    {entry.symbolType}
                  </span>
                </td>
                <td className="symbol-datatype">
                  <code className="datatype-code">{entry.dataType}</code>
                </td>
                <td className="symbol-scope">
                  <span
                    className="scope-badge"
                    style={{ backgroundColor: getScopeColor(entry.scope) }}
                  >
                    {entry.scope}
                  </span>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      {/* Resumo */}
      <div className="symbol-summary">
        <h4>📊 Resumo da Tabela de Símbolos:</h4>
        <div className="summary-grid">
          <div className="summary-item">
            <span className="summary-label">Total de Símbolos:</span>
            <span className="summary-value">{stats.total}</span>
          </div>
          <div className="summary-item">
            <span className="summary-label">Variáveis:</span>
            <span className="summary-value">{stats.variables}</span>
          </div>
          <div className="summary-item">
            <span className="summary-label">Funções:</span>
            <span className="summary-value">{stats.functions}</span>
          </div>
          <div className="summary-item">
            <span className="summary-label">Canais:</span>
            <span className="summary-value">{stats.channels}</span>
          </div>
          <div className="summary-item">
            <span className="summary-label">Escopo Global:</span>
            <span className="summary-value">{stats.globals}</span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SymbolTableTab;