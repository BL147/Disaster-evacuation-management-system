const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const WEB_ROOT = path.resolve(__dirname, 'public');
const BINARY_PATH = path.resolve(__dirname, '..', 'evacuation');
const PORT = process.env.PORT || 3000;

const mimeTypes = {
  '.html': 'text/html',
  '.css': 'text/css',
  '.js': 'application/javascript',
  '.json': 'application/json',
  '.svg': 'image/svg+xml',
  '.png': 'image/png',
  '.ico': 'image/x-icon',
};

function sendJson(res, status, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(status, {
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(body),
  });
  res.end(body);
}

function sendError(res, status, message) {
  sendJson(res, status, { error: message });
}

function serveStatic(req, res) {
  let pathname = decodeURIComponent(new URL(req.url, `http://${req.headers.host}`).pathname);
  if (pathname === '/') pathname = '/index.html';
  const filePath = path.join(WEB_ROOT, pathname);

  if (!filePath.startsWith(WEB_ROOT)) {
    sendError(res, 403, 'Forbidden');
    return;
  }

  fs.readFile(filePath, (err, data) => {
    if (err) {
      sendError(res, 404, 'Not found');
      return;
    }
    const ext = path.extname(filePath).toLowerCase();
    res.writeHead(200, { 'Content-Type': mimeTypes[ext] || 'application/octet-stream' });
    res.end(data);
  });
}

function runBinary(args) {
  const result = spawnSync(BINARY_PATH, args, { encoding: 'utf8' });
  if (result.error) {
    throw result.error;
  }
  if (result.status !== 0) {
    throw new Error(result.stderr || result.stdout || `Binary exited with ${result.status}`);
  }
  return result.stdout;
}

function parseJsonBody(req, callback) {
  let body = '';
  req.on('data', chunk => {
    body += chunk;
  });
  req.on('end', () => {
    try {
      callback(JSON.parse(body || '{}'));
    } catch (err) {
      callback(null, err);
    }
  });
}

function handleApi(req, res) {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const pathname = url.pathname;
  const query = url.searchParams;

  try {
    if (pathname === '/api/graph') {
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'graph'])));
    }
    if (pathname === '/api/bfs') {
      const origin = query.get('origin');
      const radius = query.get('radius');
      if (!origin || !radius) return sendError(res, 400, 'origin and radius are required');
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'bfs', origin, radius])));
    }
    if (pathname === '/api/dijkstra') {
      const src = query.get('src');
      const dest = query.get('dest');
      if (!src || !dest) return sendError(res, 400, 'src and dest are required');
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'dijkstra', src, dest])));
    }
    if (pathname === '/api/astar') {
      const src = query.get('src');
      const dest = query.get('dest');
      if (!src || !dest) return sendError(res, 400, 'src and dest are required');
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'astar', src, dest])));
    }
    if (pathname === '/api/compare') {
      const src = query.get('src');
      const dest = query.get('dest');
      if (!src || !dest) return sendError(res, 400, 'src and dest are required');
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'compare', src, dest])));
    }
    if (pathname === '/api/kruskal') {
      return sendJson(res, 200, JSON.parse(runBinary(['--json', 'kruskal'])));
    }
    if (pathname === '/api/reroute' && req.method === 'POST') {
      return parseJsonBody(req, (body, err) => {
        if (err) return sendError(res, 400, 'Invalid JSON body');
        const { roadSrc, roadDest, evacuee, shelter } = body;
        if ([roadSrc, roadDest, evacuee, shelter].some(v => v === undefined)) {
          return sendError(res, 400, 'roadSrc, roadDest, evacuee, and shelter are required');
        }
        try {
          const result = JSON.parse(runBinary(['--json', 'reroute', String(roadSrc), String(roadDest), String(evacuee), String(shelter)]));
          sendJson(res, 200, result);
        } catch (error) {
          sendError(res, 500, error.message);
        }
      });
    }

    sendError(res, 404, 'API endpoint not found');
  } catch (error) {
    sendError(res, 500, error.message);
  }
}

const server = http.createServer((req, res) => {
  if (req.url.startsWith('/api/')) {
    return handleApi(req, res);
  }
  serveStatic(req, res);
});

server.listen(PORT, () => {
  console.log(`Evacuation web server running on http://localhost:${PORT}`);
  console.log('Ensure the evacuation binary is built at ../evacuation');
});
