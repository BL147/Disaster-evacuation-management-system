const graphSvg = document.getElementById('graphSvg');
const resultArea = document.getElementById('resultArea');
const insightText = document.getElementById('insightText');

const inputs = {
  bfsOrigin: document.getElementById('bfsOrigin'),
  bfsRadius: document.getElementById('bfsRadius'),
  dijkstraSrc: document.getElementById('dijkstraSrc'),
  dijkstraDest: document.getElementById('dijkstraDest'),
  astarSrc: document.getElementById('astarSrc'),
  astarDest: document.getElementById('astarDest'),
  compareSrc: document.getElementById('compareSrc'),
  compareDest: document.getElementById('compareDest'),
  rerouteSrc: document.getElementById('rerouteSrc'),
  rerouteDest: document.getElementById('rerouteDest'),
  rerouteEvacuee: document.getElementById('rerouteEvacuee'),
  rerouteShelter: document.getElementById('rerouteShelter'),
};

const state = {
  nodes: [],
  edges: [],
  danger: [],
  path: [],
  mst: [],
  blocked: null,
  highlightType: null,
};

const algorithmInfo = {
  bfs: 'BFS explores the city one hop at a time, making it ideal for discovering the disaster impact radius and nearby danger zones.',
  dijkstra: 'Dijkstra finds the shortest weighted path across the road network, so it returns the optimal evacuation route when all roads are open.',
  astar: 'A* uses straight-line distance to guide search toward the shelter, exploring fewer nodes than blind search and speeding up route planning.',
  compare: 'Compare Dijkstra and A* to see how heuristic guidance affects route discovery while still preserving optimality.',
  kruskal: 'Kruskal builds the minimum backbone road network, helping planners identify the most critical roads to keep connected during emergencies.',
  reroute: 'Dynamic rerouting simulates road blockages and finds an alternate safe path using the current graph state in real time.',
};

function setResult(text) {
  resultArea.textContent = text;
}

function setInsight(text) {
  insightText.textContent = text;
}

function scalePoint(x, y) {
  const width = 900;
  const height = 540;
  const margin = 70;
  const px = margin + (x / 100) * (width - margin * 2);
  const py = height - (margin + (y / 100) * (height - margin * 2));
  return { x: px, y: py };
}

function edgeMatches(a, b) {
  return (a.src === b.src && a.dest === b.dest) || (a.src === b.dest && a.dest === b.src);
}

function renderGraph() {
  graphSvg.innerHTML = '';
  if (!state.nodes.length) return;

  state.edges.forEach((edge, index) => {
    const a = state.nodes[edge.src];
    const b = state.nodes[edge.dest];
    const p1 = scalePoint(a.x, a.y);
    const p2 = scalePoint(b.x, b.y);
    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', p1.x);
    line.setAttribute('y1', p1.y);
    line.setAttribute('x2', p2.x);
    line.setAttribute('y2', p2.y);
    line.setAttribute('class', 'edge-line');
    if (edge.blocked) line.classList.add('edge-blocked');
    if (state.mst.some(e => edgeMatches(e, edge))) line.classList.add('edge-active');
    if (state.pathEdges?.some(e => edgeMatches(e, edge))) line.classList.add('edge-active');
    if (state.dangerEdges?.some(e => edgeMatches(e, edge))) line.classList.add('edge-danger');
    graphSvg.appendChild(line);

    const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    label.setAttribute('x', (p1.x + p2.x) / 2);
    label.setAttribute('y', (p1.y + p2.y) / 2 - 6);
    label.setAttribute('class', 'edge-label');
    label.textContent = edge.weight;
    graphSvg.appendChild(label);
  });

  state.nodes.forEach(node => {
    const p = scalePoint(node.x, node.y);
    const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
    circle.setAttribute('cx', p.x);
    circle.setAttribute('cy', p.y);
    circle.setAttribute('r', '16');
    circle.setAttribute('class', 'node-circle');
    if (state.danger.includes(node.id)) circle.classList.add('node-danger');
    if (state.path.includes(node.id)) circle.classList.add('node-path');
    if (node.type === 'SHELTER') circle.classList.add('node-shelter');
    graphSvg.appendChild(circle);

    const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    label.setAttribute('x', p.x);
    label.setAttribute('y', p.y + 36);
    label.setAttribute('class', 'node-text');
    label.textContent = `${node.id}: ${node.name}`;
    graphSvg.appendChild(label);
  });
}

function updateNodeSelectors() {
  const options = state.nodes.map(node => `<option value="${node.id}">${node.id}: ${node.name}</option>`).join('');
  Object.values(inputs).forEach(select => {
    if (select.tagName === 'SELECT') select.innerHTML = options;
  });
}

async function fetchGraph() {
  try {
    const res = await fetch('/api/graph');
    state.nodes = await res.json().then(data => data.nodes);
    state.edges = await res.json().then(data => data.edges);
  } catch (err) {
    console.error(err);
  }
}

function clearHighlights() {
  state.danger = [];
  state.path = [];
  state.pathEdges = [];
  state.mst = [];
  state.dangerEdges = [];
  state.blocked = null;
}

async function loadGraph() {
  try {
    const response = await fetch('/api/graph');
    const data = await response.json();
    state.nodes = data.nodes;
    state.edges = data.edges;
    clearHighlights();
    renderGraph();
    updateNodeSelectors();
    setResult('Graph loaded: ' + state.nodes.length + ' nodes, ' + state.edges.length + ' edges.');
    setInsight('Select any operation to see the graph change in realtime.');
  } catch (error) {
    setResult('Unable to load graph: ' + error.message);
  }
}

function edgeListFromPath(path) {
  const edges = [];
  for (let i = 0; i < path.length - 1; i++) {
    const src = path[i];
    const dest = path[i + 1];
    edges.push({ src, dest });
  }
  return edges;
}

async function runOperation(endpoint, payload, label, insight) {
  try {
    const url = payload ? '/api/' + endpoint : '/api/' + endpoint + payload;
    const init = payload ? { method: 'POST', body: JSON.stringify(payload), headers: { 'Content-Type': 'application/json' } } : undefined;
    const res = await fetch(url, init);
    const data = await res.json();
    if (res.ok) {
      setResult(JSON.stringify(data, null, 2));
      setInsight(insight);
      return data;
    } else {
      setResult(data.error || 'Operation failed');
      setInsight('Review the input and try again.');
      return null;
    }
  } catch (err) {
    setResult('Error: ' + err.message);
    setInsight('Check that the web server is running and the evacuation binary is built.');
    return null;
  }
}

function mapPathData(data) {
  return {
    path: data.path || [],
    pathEdges: edgeListFromPath(data.path || []),
  };
}

function mapKruskalData(data) {
  return {
    mst: data.edges.map(edge => ({ src: edge.src, dest: edge.dest })),
  };
}

function mapBfsData(data) {
  return {
    danger: data.affected,
  };
}

function mapRerouteData(data) {
  return {
    path: data.path || [],
    pathEdges: edgeListFromPath(data.path || []),
    blocked: data.roadBlocked,
  };
}

async function init() {
  document.getElementById('refreshGraph').addEventListener('click', loadGraph);
  document.getElementById('runBfs').addEventListener('click', async () => {
    const data = await runOperation(`bfs?origin=${inputs.bfsOrigin.value}&radius=${inputs.bfsRadius.value}`, null, 'BFS danger zone calculation', algorithmInfo.bfs);
    if (data) {
      state.danger = data.affected;
      state.path = [];
      state.pathEdges = [];
      state.mst = [];
      renderGraph();
    }
  });

  document.getElementById('runDijkstra').addEventListener('click', async () => {
    const data = await runOperation(`dijkstra?src=${inputs.dijkstraSrc.value}&dest=${inputs.dijkstraDest.value}`, null, 'Dijkstra shortest path', algorithmInfo.dijkstra);
    if (data) {
      state.path = data.path;
      state.pathEdges = edgeListFromPath(data.path);
      state.danger = [];
      state.mst = [];
      renderGraph();
    }
  });

  document.getElementById('runAstar').addEventListener('click', async () => {
    const data = await runOperation(`astar?src=${inputs.astarSrc.value}&dest=${inputs.astarDest.value}`, null, 'A* heuristic-guided path', algorithmInfo.astar);
    if (data) {
      state.path = data.path;
      state.pathEdges = edgeListFromPath(data.path);
      state.danger = [];
      state.mst = [];
      renderGraph();
    }
  });

  document.getElementById('runCompare').addEventListener('click', async () => {
    await runOperation(`compare?src=${inputs.compareSrc.value}&dest=${inputs.compareDest.value}`, null, 'Compare Dijkstra and A*', algorithmInfo.compare);
    state.path = [];
    state.pathEdges = [];
    state.danger = [];
    state.mst = [];
    renderGraph();
  });

  document.getElementById('runKruskal').addEventListener('click', async () => {
    const data = await runOperation('kruskal', null, 'Minimum spanning tree', algorithmInfo.kruskal);
    if (data) {
      state.mst = data.edges.map(edge => ({ src: edge.src, dest: edge.dest }));
      state.path = [];
      state.pathEdges = [];
      state.danger = [];
      renderGraph();
    }
  });

  document.getElementById('runReroute').addEventListener('click', async () => {
    const body = {
      roadSrc: Number(inputs.rerouteSrc.value),
      roadDest: Number(inputs.rerouteDest.value),
      evacuee: Number(inputs.rerouteEvacuee.value),
      shelter: Number(inputs.rerouteShelter.value),
    };
    const data = await runOperation('reroute', body, 'Dynamic rerouting after road blockage', algorithmInfo.reroute);
    if (data) {
      state.path = data.path || [];
      state.pathEdges = edgeListFromPath(data.path || []);
      state.danger = [];
      state.mst = [];
      state.blocked = data.roadBlocked;
      renderGraph();
    }
  });

  await loadGraph();
}

init();
