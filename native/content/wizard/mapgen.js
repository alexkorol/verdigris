/*
 * MapGen — procedural 2D zone generator. Part of the WIZARD project.
 *
 * Self-contained and dependency-free. Loads as a browser global (window.MapGen)
 * or a CommonJS module (require('./mapgen.js')). Output is engine-agnostic:
 * a grid of logical tile ids plus an entity list, ready to be mapped onto
 * any tileset or renderer.
 *
 * Usage:
 *   const map = MapGen.generate({ zone:'dungeon', theme:'crypt', seed:1234, width:72, height:54 });
 *   map.tiles      Uint8Array (width*height), values from MapGen.TILE
 *   map.entities   [{type, x, y}, ...]
 *   map.entrance   {x, y}
 *   map.exit       {x, y}
 *   map.palette    suggested colors for the theme (optional to use)
 *
 * Same seed + options always produces the same map.
 */
(function (root, factory) {
  if (typeof module === 'object' && module.exports) module.exports = factory();
  else root.MapGen = factory();
})(typeof self !== 'undefined' ? self : this, function () {
  'use strict';

  var VERSION = '1.0.0';

  // ---------------------------------------------------------------- tiles

  var TILE = {
    VOID: 0, FLOOR: 1, WALL: 2, DOOR: 3,
    WATER: 4, DEEP: 5, LAVA: 6,
    GRASS: 7, TREE: 8, ROCK: 9,
    PATH: 10, BRIDGE: 11, SAND: 12, RUBBLE: 13, MURK: 14
  };

  var TILE_NAMES = {};
  Object.keys(TILE).forEach(function (k) { TILE_NAMES[TILE[k]] = k; });

  // Tiles a walker can stand on. Games are free to redefine this.
  var WALKABLE = new Set([
    TILE.FLOOR, TILE.DOOR, TILE.GRASS, TILE.PATH,
    TILE.BRIDGE, TILE.SAND, TILE.RUBBLE, TILE.MURK
  ]);

  function walkable(t) { return WALKABLE.has(t); }

  // ---------------------------------------------------------------- rng

  function hashString(str) {
    var h = 2166136261 >>> 0;
    for (var i = 0; i < str.length; i++) {
      h ^= str.charCodeAt(i);
      h = Math.imul(h, 16777619);
    }
    return h >>> 0;
  }

  function mulberry32(a) {
    a = a >>> 0;
    return function () {
      a = (a + 0x6D2B79F5) | 0;
      var t = Math.imul(a ^ (a >>> 15), 1 | a);
      t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
      return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
    };
  }

  function RNG(seed) {
    this.seed = seed >>> 0;
    this._f = mulberry32(this.seed);
  }
  RNG.prototype.next = function () { return this._f(); };
  RNG.prototype.int = function (n) { return Math.floor(this._f() * n); };
  RNG.prototype.irange = function (a, b) { return a + this.int(b - a + 1); };
  RNG.prototype.range = function (a, b) { return a + this._f() * (b - a); };
  RNG.prototype.pick = function (arr) { return arr[this.int(arr.length)]; };
  RNG.prototype.chance = function (p) { return this._f() < p; };
  RNG.prototype.shuffle = function (arr) {
    for (var i = arr.length - 1; i > 0; i--) {
      var j = this.int(i + 1);
      var t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
    return arr;
  };

  // Deterministic value noise + fBm, seeded from the map rng.
  function makeNoise(rng) {
    var perm = new Uint8Array(512);
    var p = [];
    for (var i = 0; i < 256; i++) p.push(i);
    rng.shuffle(p);
    for (i = 0; i < 512; i++) perm[i] = p[i & 255];
    function lattice(x, y) { return perm[(perm[x & 255] + y) & 255] / 255; }
    function fade(t) { return t * t * (3 - 2 * t); }
    function noise2(x, y) {
      var xi = Math.floor(x), yi = Math.floor(y);
      var xf = x - xi, yf = y - yi;
      var a = lattice(xi, yi), b = lattice(xi + 1, yi);
      var c = lattice(xi, yi + 1), d = lattice(xi + 1, yi + 1);
      var u = fade(xf), v = fade(yf);
      return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
    }
    function fbm(x, y, oct) {
      oct = oct || 4;
      var v = 0, amp = 0.5, f = 1, max = 0;
      for (var o = 0; o < oct; o++) {
        v += amp * noise2(x * f, y * f);
        max += amp; amp *= 0.5; f *= 2;
      }
      return v / max;
    }
    return { noise2: noise2, fbm: fbm };
  }

  function clamp(v, a, b) { return v < a ? a : (v > b ? b : v); }

  // ---------------------------------------------------------------- builder

  function MapBuilder(w, h, rng) {
    this.w = w; this.h = h; this.rng = rng;
    this.tiles = new Uint8Array(w * h);
    this.tiles.fill(TILE.WALL);
    this.entities = [];
    this.rooms = [];      // {x, y, w, h, cx, cy}
    this.gates = null;    // generator may pre-pick [entrance, exit]
  }
  MapBuilder.prototype.inBounds = function (x, y) {
    return x >= 0 && y >= 0 && x < this.w && y < this.h;
  };
  MapBuilder.prototype.get = function (x, y) {
    return this.inBounds(x, y) ? this.tiles[y * this.w + x] : TILE.VOID;
  };
  MapBuilder.prototype.set = function (x, y, t) {
    if (this.inBounds(x, y)) this.tiles[y * this.w + x] = t;
  };
  MapBuilder.prototype.fill = function (t) { this.tiles.fill(t); };
  MapBuilder.prototype.rect = function (x, y, w, h, t) {
    for (var j = y; j < y + h; j++)
      for (var i = x; i < x + w; i++) this.set(i, j, t);
  };
  MapBuilder.prototype.isWalkable = function (x, y) {
    return walkable(this.get(x, y));
  };
  MapBuilder.prototype.addEntity = function (type, x, y) {
    this.entities.push({ type: type, x: x, y: y });
  };
  MapBuilder.prototype.addRoom = function (x, y, w, h) {
    var r = { x: x, y: y, w: w, h: h, cx: x + (w >> 1), cy: y + (h >> 1) };
    this.rooms.push(r);
    return r;
  };

  var DIR4 = [[1, 0], [-1, 0], [0, 1], [0, -1]];

  // ---------------------------------------------------------------- graph helpers

  // Connected regions of walkable tiles. Returns arrays of [x, y] pairs.
  function walkableRegions(b) {
    var seen = new Uint8Array(b.w * b.h);
    var regions = [];
    for (var y = 0; y < b.h; y++) {
      for (var x = 0; x < b.w; x++) {
        var i = y * b.w + x;
        if (seen[i] || !walkable(b.tiles[i])) continue;
        var region = [];
        var stack = [[x, y]];
        seen[i] = 1;
        while (stack.length) {
          var c = stack.pop();
          region.push(c);
          for (var d = 0; d < 4; d++) {
            var nx = c[0] + DIR4[d][0], ny = c[1] + DIR4[d][1];
            if (!b.inBounds(nx, ny)) continue;
            var ni = ny * b.w + nx;
            if (!seen[ni] && walkable(b.tiles[ni])) {
              seen[ni] = 1;
              stack.push([nx, ny]);
            }
          }
        }
        regions.push(region);
      }
    }
    return regions;
  }

  // BFS distance field over walkable tiles from (sx, sy). -1 = unreachable.
  function bfs(b, sx, sy) {
    var dist = new Int32Array(b.w * b.h);
    dist.fill(-1);
    var queue = new Int32Array(b.w * b.h);
    var head = 0, tail = 0;
    dist[sy * b.w + sx] = 0;
    queue[tail++] = sy * b.w + sx;
    while (head < tail) {
      var i = queue[head++];
      var x = i % b.w, y = (i / b.w) | 0;
      for (var d = 0; d < 4; d++) {
        var nx = x + DIR4[d][0], ny = y + DIR4[d][1];
        if (nx < 0 || ny < 0 || nx >= b.w || ny >= b.h) continue;
        var ni = ny * b.w + nx;
        if (dist[ni] === -1 && walkable(b.tiles[ni])) {
          dist[ni] = dist[i] + 1;
          queue[tail++] = ni;
        }
      }
    }
    return dist;
  }

  function farthestIndex(dist) {
    var best = -1, bi = -1;
    for (var i = 0; i < dist.length; i++) {
      if (dist[i] > best) { best = dist[i]; bi = i; }
    }
    return bi;
  }

  // Carve a wobbly tunnel between two points. Liquids become bridges,
  // solids become `carve`. Guarantees arrival with a straight finish.
  function tunnel(b, x0, y0, x1, y1, carve) {
    function put(x, y) {
      var t = b.get(x, y);
      if (t === TILE.WATER || t === TILE.DEEP || t === TILE.LAVA) b.set(x, y, TILE.BRIDGE);
      else if (!walkable(t)) b.set(x, y, carve);
    }
    var x = x0, y = y0;
    put(x, y);
    var guard = (b.w + b.h) * 5;
    while ((x !== x1 || y !== y1) && guard-- > 0) {
      var dx = Math.sign(x1 - x), dy = Math.sign(y1 - y);
      var sx = 0, sy = 0;
      if (dx && dy) { if (b.rng.chance(0.5)) sx = dx; else sy = dy; }
      else if (dx) sx = dx;
      else sy = dy;
      if (b.rng.chance(0.16)) {
        if (sx) { sx = 0; sy = b.rng.chance(0.5) ? 1 : -1; }
        else { sy = 0; sx = b.rng.chance(0.5) ? 1 : -1; }
      }
      x = clamp(x + sx, 1, b.w - 2);
      y = clamp(y + sy, 1, b.h - 2);
      put(x, y);
    }
    while (x !== x1) { x += Math.sign(x1 - x); put(x, y); }
    while (y !== y1) { y += Math.sign(y1 - y); put(x, y); }
  }

  // After generation, stitch every walkable region to the largest one.
  function ensureConnected(b, carve) {
    var regs = walkableRegions(b);
    if (regs.length <= 1) return;
    regs.sort(function (a, c) { return c.length - a.length; });
    var main = regs[0];
    for (var r = 1; r < regs.length; r++) {
      var reg = regs[r];
      if (reg.length < 4) { // tiny pockets: wall them off instead
        for (var k = 0; k < reg.length; k++) b.set(reg[k][0], reg[k][1], TILE.WALL);
        continue;
      }
      // nearest pair between sampled points of the two regions
      var best = Infinity, pa = null, pb = null;
      var samples = 24;
      for (var i = 0; i < samples; i++) {
        var a = reg[b.rng.int(reg.length)];
        var c = main[b.rng.int(main.length)];
        var d = Math.abs(a[0] - c[0]) + Math.abs(a[1] - c[1]);
        if (d < best) { best = d; pa = a; pb = c; }
      }
      tunnel(b, pa[0], pa[1], pb[0], pb[1], carve);
    }
  }

  // A* over a cost function. Returns array of [x, y] or null.
  function aStar(b, sx, sy, tx, ty, costFn) {
    var w = b.w, h = b.h, size = w * h;
    var g = new Float64Array(size); g.fill(Infinity);
    var came = new Int32Array(size); came.fill(-1);
    var closed = new Uint8Array(size);
    // binary heap of [f, index]
    var heap = [];
    function push(f, i) {
      heap.push([f, i]);
      var c = heap.length - 1;
      while (c > 0) {
        var p = (c - 1) >> 1;
        if (heap[p][0] <= heap[c][0]) break;
        var t = heap[p]; heap[p] = heap[c]; heap[c] = t;
        c = p;
      }
    }
    function pop() {
      var top = heap[0];
      var last = heap.pop();
      if (heap.length) {
        heap[0] = last;
        var c = 0;
        for (;;) {
          var l = 2 * c + 1, r = l + 1, m = c;
          if (l < heap.length && heap[l][0] < heap[m][0]) m = l;
          if (r < heap.length && heap[r][0] < heap[m][0]) m = r;
          if (m === c) break;
          var t = heap[m]; heap[m] = heap[c]; heap[c] = t;
          c = m;
        }
      }
      return top;
    }
    var start = sy * w + sx, goal = ty * w + tx;
    g[start] = 0;
    push(Math.abs(tx - sx) + Math.abs(ty - sy), start);
    while (heap.length) {
      var cur = pop()[1];
      if (cur === goal) {
        var path = [];
        var i = goal;
        while (i !== -1) {
          path.push([i % w, (i / w) | 0]);
          i = came[i];
        }
        path.reverse();
        return path;
      }
      if (closed[cur]) continue;
      closed[cur] = 1;
      var cx = cur % w, cy = (cur / w) | 0;
      for (var d = 0; d < 4; d++) {
        var nx = cx + DIR4[d][0], ny = cy + DIR4[d][1];
        if (nx < 1 || ny < 1 || nx >= w - 1 || ny >= h - 1) continue;
        var ni = ny * w + nx;
        if (closed[ni]) continue;
        var step = costFn(b.tiles[ni], nx, ny);
        if (step >= Infinity) continue;
        var ng = g[cur] + step;
        if (ng < g[ni]) {
          g[ni] = ng;
          came[ni] = cur;
          push(ng + Math.abs(tx - nx) + Math.abs(ty - ny), ni);
        }
      }
    }
    return null;
  }

  // ---------------------------------------------------------------- generators

  function genDungeon(b, theme) {
    var rng = b.rng;
    // Diablo 2 style: rooms packed tight (1-tile walls between them) so
    // connections are doorways and stubs, not long corridors.
    var attempts = Math.floor((b.w * b.h) / 30);
    for (var a = 0; a < attempts; a++) {
      var rw = rng.irange(4, 9), rh = rng.irange(4, 7);
      var rx = rng.irange(2, b.w - rw - 3), ry = rng.irange(2, b.h - rh - 3);
      var ok = true;
      for (var r = 0; r < b.rooms.length && ok; r++) {
        var o = b.rooms[r];
        if (rx < o.x + o.w + 1 && rx + rw + 1 > o.x &&
            ry < o.y + o.h + 1 && ry + rh + 1 > o.y) ok = false;
      }
      if (!ok) continue;
      b.rect(rx, ry, rw, rh, TILE.FLOOR);
      b.addRoom(rx, ry, rw, rh);
    }
    if (b.rooms.length < 2) { // degenerate fallback
      b.rect(2, 2, b.w - 4, b.h - 4, TILE.FLOOR);
      b.addRoom(2, 2, b.w - 4, b.h - 4);
      return;
    }

    // connect rooms: Prim MST over centers, then loop edges between
    // near neighbors so runs never backtrack across the map
    var n = b.rooms.length;
    var inTree = new Uint8Array(n);
    inTree[0] = 1;
    var edges = [];
    var edgeSet = new Set();
    function noteEdge(i, j) {
      edges.push([i, j]);
      edgeSet.add(Math.min(i, j) + ':' + Math.max(i, j));
    }
    for (var added = 1; added < n; added++) {
      var best = Infinity, bi = -1, bj = -1;
      for (var i = 0; i < n; i++) {
        if (!inTree[i]) continue;
        for (var j = 0; j < n; j++) {
          if (inTree[j]) continue;
          var d = Math.abs(b.rooms[i].cx - b.rooms[j].cx) +
                  Math.abs(b.rooms[i].cy - b.rooms[j].cy);
          if (d < best) { best = d; bi = i; bj = j; }
        }
      }
      inTree[bj] = 1;
      noteEdge(bi, bj);
    }
    // loops: each extra edge joins a room to its nearest not-yet-linked neighbor
    var extra = Math.max(2, Math.floor(n * 0.4));
    for (var e = 0; e < extra; e++) {
      var i2 = rng.int(n);
      var best2 = Infinity, j3 = -1;
      for (var j2 = 0; j2 < n; j2++) {
        if (j2 === i2) continue;
        if (edgeSet.has(Math.min(i2, j2) + ':' + Math.max(i2, j2))) continue;
        var d2 = Math.abs(b.rooms[i2].cx - b.rooms[j2].cx) +
                 Math.abs(b.rooms[i2].cy - b.rooms[j2].cy);
        if (d2 < best2) { best2 = d2; j3 = j2; }
      }
      if (j3 >= 0) noteEdge(i2, j3);
    }
    edges.forEach(function (ed) {
      var ra = b.rooms[ed[0]], rb = b.rooms[ed[1]];
      // carve between the nearest interior points of the two rooms:
      // adjacent rooms get a doorway, distant rooms a short stub
      var ax = clamp(rb.cx, ra.x + 1, ra.x + ra.w - 2);
      var ay = clamp(rb.cy, ra.y + 1, ra.y + ra.h - 2);
      var bx = clamp(ax, rb.x + 1, rb.x + rb.w - 2);
      var by = clamp(ay, rb.y + 1, rb.y + rb.h - 2);
      if (rng.chance(0.5)) {
        for (var x = Math.min(ax, bx); x <= Math.max(ax, bx); x++) carveCorr(x, ay);
        for (var y = Math.min(ay, by); y <= Math.max(ay, by); y++) carveCorr(bx, y);
      } else {
        for (var y2 = Math.min(ay, by); y2 <= Math.max(ay, by); y2++) carveCorr(ax, y2);
        for (var x2 = Math.min(ax, bx); x2 <= Math.max(ax, bx); x2++) carveCorr(x2, by);
      }
    });
    function carveCorr(x, y) {
      if (b.get(x, y) === TILE.WALL) b.set(x, y, TILE.FLOOR);
    }

    // doors where corridors pierce room walls
    b.rooms.forEach(function (room) {
      var ring = [];
      for (var x = room.x - 1; x <= room.x + room.w; x++) {
        ring.push([x, room.y - 1]); ring.push([x, room.y + room.h]);
      }
      for (var y = room.y; y < room.y + room.h; y++) {
        ring.push([room.x - 1, y]); ring.push([room.x + room.w, y]);
      }
      ring.forEach(function (c) {
        var x = c[0], y = c[1];
        if (b.get(x, y) !== TILE.FLOOR) return;
        var hFlank = b.get(x - 1, y) === TILE.WALL && b.get(x + 1, y) === TILE.WALL &&
                     b.isWalkable(x, y - 1) && b.isWalkable(x, y + 1);
        var vFlank = b.get(x, y - 1) === TILE.WALL && b.get(x, y + 1) === TILE.WALL &&
                     b.isWalkable(x - 1, y) && b.isWalkable(x + 1, y);
        if ((hFlank || vFlank) && rng.chance(0.72)) {
          // avoid doors adjacent to doors
          for (var d = 0; d < 4; d++) {
            if (b.get(x + DIR4[d][0], y + DIR4[d][1]) === TILE.DOOR) return;
          }
          b.set(x, y, TILE.DOOR);
        }
      });
    });

    // occasional water/sewage channel through the dungeon
    if (theme.channel) {
      var noise = makeNoise(rng);
      for (var y3 = 1; y3 < b.h - 1; y3++) {
        for (var x3 = 1; x3 < b.w - 1; x3++) {
          if (b.get(x3, y3) !== TILE.FLOOR) continue;
          var v = noise.fbm(x3 * 0.09, y3 * 0.09, 3);
          if (v < 0.3) b.set(x3, y3, v < 0.22 ? TILE.DEEP : TILE.WATER);
        }
      }
    }
  }

  function genCaves(b, theme) {
    var rng = b.rng;
    var w = b.w, h = b.h;
    var cur = new Uint8Array(w * h);
    for (var y = 0; y < h; y++) {
      for (var x = 0; x < w; x++) {
        cur[y * w + x] = (x === 0 || y === 0 || x === w - 1 || y === h - 1 || rng.chance(0.44)) ? 1 : 0;
      }
    }
    var next = new Uint8Array(w * h);
    for (var it = 0; it < 5; it++) {
      for (y = 1; y < h - 1; y++) {
        for (x = 1; x < w - 1; x++) {
          var walls = 0;
          for (var dy = -1; dy <= 1; dy++)
            for (var dx = -1; dx <= 1; dx++) {
              walls += cur[(y + dy) * w + (x + dx)]; // 3x3 including self
            }
          next[y * w + x] = walls >= 5 ? 1 : 0;
        }
      }
      for (y = 0; y < h; y++) { next[y * w] = 1; next[y * w + w - 1] = 1; }
      for (x = 0; x < w; x++) { next[x] = 1; next[(h - 1) * w + x] = 1; }
      var t = cur; cur = next; next = t;
    }
    for (var i = 0; i < w * h; i++) b.tiles[i] = cur[i] ? TILE.WALL : TILE.FLOOR;

    // liquid pools in the low parts of a noise field
    var noise = makeNoise(rng);
    var liquid = theme.liquid || 'water';
    var level = theme.liquidLevel != null ? theme.liquidLevel : 0.34;
    for (y = 1; y < h - 1; y++) {
      for (x = 1; x < w - 1; x++) {
        if (b.get(x, y) !== TILE.FLOOR) continue;
        var v = noise.fbm(x * 0.07, y * 0.07, 4);
        if (liquid === 'lava') {
          if (v < level) b.set(x, y, TILE.LAVA);
        } else if (liquid === 'water') {
          if (v < level - 0.06) b.set(x, y, TILE.DEEP);
          else if (v < level) b.set(x, y, TILE.WATER);
        }
        // scattered boulders on high ground
        if (b.get(x, y) === TILE.FLOOR && v > 0.72 && rng.chance(0.16)) b.set(x, y, TILE.ROCK);
      }
    }
  }

  function genCatacombs(b, theme) {
    var rng = b.rng;
    // Coarse maze on a 4-tile pitch: 2-wide corridors, 2-tile walls.
    // Wide passages plus heavy braiding keep it a tomb, not a labyrinth
    // (the Maggot Lair is the cautionary tale here).
    var cw = Math.floor((b.w - 3) / 4), ch = Math.floor((b.h - 3) / 4);
    var visited = new Uint8Array(cw * ch);
    var open = [];
    for (var i = 0; i < cw * ch; i++) open.push([false, false, false, false]);
    var stack = [[rng.int(cw), rng.int(ch)]];
    visited[stack[0][1] * cw + stack[0][0]] = 1;
    while (stack.length) {
      var c = stack[stack.length - 1];
      var opts = [];
      for (var d = 0; d < 4; d++) {
        var nx = c[0] + DIR4[d][0], ny = c[1] + DIR4[d][1];
        if (nx >= 0 && ny >= 0 && nx < cw && ny < ch && !visited[ny * cw + nx]) opts.push([nx, ny, d]);
      }
      if (!opts.length) { stack.pop(); continue; }
      var pickd = rng.pick(opts);
      visited[pickd[1] * cw + pickd[0]] = 1;
      open[c[1] * cw + c[0]][pickd[2]] = true;
      open[pickd[1] * cw + pickd[0]][pickd[2] ^ 1] = true; // DIR4 pairs are opposites
      stack.push([pickd[0], pickd[1]]);
    }

    // braid: give almost every dead-end cell a second exit
    for (var cy = 0; cy < ch; cy++) {
      for (var cx = 0; cx < cw; cx++) {
        var o = open[cy * cw + cx];
        var exits = (o[0] ? 1 : 0) + (o[1] ? 1 : 0) + (o[2] ? 1 : 0) + (o[3] ? 1 : 0);
        if (exits !== 1 || !rng.chance(0.8)) continue;
        var cands = [];
        for (d = 0; d < 4; d++) {
          if (o[d]) continue;
          var nx2 = cx + DIR4[d][0], ny2 = cy + DIR4[d][1];
          if (nx2 >= 0 && ny2 >= 0 && nx2 < cw && ny2 < ch) cands.push(d);
        }
        if (!cands.length) continue;
        d = rng.pick(cands);
        o[d] = true;
        open[(cy + DIR4[d][1]) * cw + (cx + DIR4[d][0])][d ^ 1] = true;
      }
    }

    // rasterize: cell (cx,cy) covers tiles [2+4cx..3+4cx] x [2+4cy..3+4cy]
    for (cy = 0; cy < ch; cy++) {
      for (cx = 0; cx < cw; cx++) {
        var bx = 2 + cx * 4, by = 2 + cy * 4;
        b.rect(bx, by, 2, 2, TILE.FLOOR);
        var o2 = open[cy * cw + cx];
        if (o2[0]) b.rect(bx + 2, by, 2, 2, TILE.FLOOR); // east passage
        if (o2[2]) b.rect(bx, by + 2, 2, 2, TILE.FLOOR); // south passage
      }
    }

    // two wide galleries crossing the maze: fast traversal spines,
    // like the pillared halls in Diablo 2's tombs
    var gy = rng.irange(Math.floor(b.h * 0.25), Math.floor(b.h * 0.75));
    for (var x = 1; x < b.w - 1; x++) { b.set(x, gy, TILE.FLOOR); b.set(x, gy + 1, TILE.FLOOR); }
    var gx = rng.irange(Math.floor(b.w * 0.25), Math.floor(b.w * 0.75));
    for (var y = 1; y < b.h - 1; y++) { b.set(gx, y, TILE.FLOOR); b.set(gx + 1, y, TILE.FLOOR); }

    // burial chambers carved over the maze
    var chambers = rng.irange(8, 12);
    for (var r = 0; r < chambers; r++) {
      var rw = rng.irange(4, 8), rh = rng.irange(3, 6);
      var rx = rng.irange(1, b.w - rw - 2);
      var ry = rng.irange(1, b.h - rh - 2);
      b.rect(rx, ry, rw, rh, TILE.FLOOR);
      b.addRoom(rx, ry, rw, rh);
    }

    // flooded variant: sunken corridors
    if (theme.flooded) {
      var noise = makeNoise(rng);
      for (y = 1; y < b.h - 1; y++) {
        for (x = 1; x < b.w - 1; x++) {
          if (b.get(x, y) !== TILE.FLOOR) continue;
          var v = noise.fbm(x * 0.11, y * 0.11, 3);
          if (v < 0.34) b.set(x, y, TILE.WATER);
        }
      }
    }
  }

  // Platforms and walkways suspended over the void — the Arcane
  // Sanctuary / Horazon's Memory archetype.
  function genSanctum(b, theme) {
    var rng = b.rng;
    b.fill(TILE.VOID);
    var attempts = Math.floor((b.w * b.h) / 40);
    for (var a = 0; a < attempts; a++) {
      var rw = rng.irange(5, 10), rh = rng.irange(4, 8);
      var rx = rng.irange(2, b.w - rw - 3), ry = rng.irange(2, b.h - rh - 3);
      var ok = true;
      for (var r = 0; r < b.rooms.length && ok; r++) {
        var o = b.rooms[r];
        if (rx < o.x + o.w + 3 && rx + rw + 3 > o.x &&
            ry < o.y + o.h + 3 && ry + rh + 3 > o.y) ok = false;
      }
      if (!ok) continue;
      b.rect(rx, ry, rw, rh, TILE.FLOOR);
      b.addRoom(rx, ry, rw, rh);
    }
    if (b.rooms.length < 2) {
      b.rect(2, 2, b.w - 4, b.h - 4, TILE.FLOOR);
      b.addRoom(2, 2, b.w - 4, b.h - 4);
      return;
    }

    // MST + loop edges, carved as 2-wide walkways between nearest points
    var n = b.rooms.length;
    var inTree = new Uint8Array(n);
    inTree[0] = 1;
    var edges = [];
    for (var added = 1; added < n; added++) {
      var best = Infinity, bi = -1, bj = -1;
      for (var i = 0; i < n; i++) {
        if (!inTree[i]) continue;
        for (var j = 0; j < n; j++) {
          if (inTree[j]) continue;
          var d = Math.abs(b.rooms[i].cx - b.rooms[j].cx) +
                  Math.abs(b.rooms[i].cy - b.rooms[j].cy);
          if (d < best) { best = d; bi = i; bj = j; }
        }
      }
      inTree[bj] = 1;
      edges.push([bi, bj]);
    }
    for (var e = 0; e < Math.max(1, Math.floor(n * 0.3)); e++) {
      var i2 = rng.int(n), best2 = Infinity, j2 = -1;
      for (var k = 0; k < n; k++) {
        if (k === i2) continue;
        var d2 = Math.abs(b.rooms[i2].cx - b.rooms[k].cx) +
                 Math.abs(b.rooms[i2].cy - b.rooms[k].cy);
        if (d2 < best2 && d2 > 0) { best2 = d2; j2 = k; }
      }
      if (j2 >= 0) edges.push([i2, j2]);
    }
    edges.forEach(function (ed) {
      var ra = b.rooms[ed[0]], rb = b.rooms[ed[1]];
      var ax = clamp(rb.cx, ra.x + 1, ra.x + ra.w - 2);
      var ay = clamp(rb.cy, ra.y + 1, ra.y + ra.h - 2);
      var bx = clamp(ax, rb.x + 1, rb.x + rb.w - 2);
      var by = clamp(ay, rb.y + 1, rb.y + rb.h - 2);
      var x, y;
      if (rng.chance(0.5)) {
        for (x = Math.min(ax, bx); x <= Math.max(ax, bx); x++) { walkway(x, ay); walkway(x, ay + 1); }
        for (y = Math.min(ay, by); y <= Math.max(ay, by); y++) { walkway(bx, y); walkway(bx + 1, y); }
      } else {
        for (y = Math.min(ay, by); y <= Math.max(ay, by); y++) { walkway(ax, y); walkway(ax + 1, y); }
        for (x = Math.min(ax, bx); x <= Math.max(ax, bx); x++) { walkway(x, by); walkway(x, by + 1); }
      }
    });
    function walkway(x, y) {
      if (b.inBounds(x, y) && b.get(x, y) === TILE.VOID) b.set(x, y, TILE.FLOOR);
    }

    // infernal variant: molten pools eating into the platforms
    if (theme.liquid === 'lava') {
      var noise = makeNoise(rng);
      for (var y2 = 1; y2 < b.h - 1; y2++) {
        for (var x2 = 1; x2 < b.w - 1; x2++) {
          if (b.get(x2, y2) !== TILE.FLOOR) continue;
          if (noise.fbm(x2 * 0.1, y2 * 0.1, 3) < 0.26) b.set(x2, y2, TILE.LAVA);
        }
      }
    }
  }

  // A linear coastline: cliffs, a grass shelf, the beach highway,
  // then open sea — PoE's Coast/Strand pacing.
  function genShore(b, theme) {
    var rng = b.rng;
    var noise = makeNoise(rng);
    var sandYs = [], seaYs = [];
    for (var x = 0; x < b.w; x++) {
      var n = noise.fbm(x * 0.05, 3.7, 3);
      var seaY = Math.floor(b.h * 0.55 + (n - 0.5) * b.h * 0.4);
      var sandY = seaY - 3 - Math.floor(noise.fbm(x * 0.08, 9.1, 3) * 4);
      var cliffY = 2 + Math.floor(noise.fbm(x * 0.07, 15.3, 3) * 3);
      sandYs.push(sandY); seaYs.push(seaY);
      for (var y = 0; y < b.h; y++) {
        var t;
        if (y < cliffY) t = TILE.ROCK;
        else if (y < sandY) {
          var m = noise.fbm(x * 0.09, y * 0.09 + 20, 3);
          t = (m > 0.6 && rng.chance(0.55)) ? TILE.TREE : TILE.GRASS;
        }
        else if (y < seaY) t = TILE.SAND;
        else if (y < seaY + 3) t = TILE.WATER;
        else t = TILE.DEEP;
        b.set(x, y, t);
      }
    }

    // harbor variant: plank piers striding into the water, warehouses on the shelf
    if (theme.piers) {
      var piers = rng.irange(2, 4);
      for (var p = 0; p < piers; p++) {
        var px = rng.irange(6, b.w - 8);
        var start = sandYs[px] + 1;
        var len = rng.irange(6, 10);
        for (var py = start; py < Math.min(start + len, b.h - 2); py++) {
          b.set(px, py, TILE.BRIDGE); b.set(px + 1, py, TILE.BRIDGE);
        }
      }
      var sheds = rng.irange(2, 4);
      for (var s = 0; s < sheds; s++) {
        var sw = rng.irange(5, 8), sh = rng.irange(3, 5);
        var sx = rng.irange(3, b.w - sw - 4);
        var sy = rng.irange(4, Math.max(5, sandYs[sx] - sh - 3));
        b.rect(sx, sy, sw, sh, TILE.WALL);
        b.rect(sx + 1, sy + 1, sw - 2, sh - 2, TILE.FLOOR);
        b.set(rng.irange(sx + 1, sx + sw - 2), sy + sh - 1, TILE.FLOOR);
        b.addRoom(sx + 1, sy + 1, sw - 2, sh - 2);
      }
    }

    // the beach is the highway: gates at its far ends
    function beachPoint(x) {
      var y = sandYs[x] + 1;
      if (!b.isWalkable(x, y)) { b.set(x, y, TILE.SAND); }
      return { x: x, y: y };
    }
    b.gates = [beachPoint(3), beachPoint(b.w - 4)];
  }

  // Street grid, plaza, and building blocks — Kehjistan Marketplace,
  // Sarn, City of Ureh.
  function genCity(b, theme) {
    var rng = b.rng;
    var ground = theme.ground === 'sand' ? TILE.SAND : TILE.GRASS;
    b.fill(ground);
    var noise = makeNoise(rng);

    // streets on a loose grid, two tiles wide
    var streetsX = [], streetsY = [];
    var x = rng.irange(6, 12);
    while (x < b.w - 6) { streetsX.push(x); x += rng.irange(13, 20); }
    var y = rng.irange(5, 10);
    while (y < b.h - 5) { streetsY.push(y); y += rng.irange(10, 16); }
    streetsX.forEach(function (sx) {
      for (var yy = 1; yy < b.h - 1; yy++) { b.set(sx, yy, TILE.PATH); b.set(sx + 1, yy, TILE.PATH); }
    });
    streetsY.forEach(function (sy) {
      for (var xx = 1; xx < b.w - 1; xx++) { b.set(xx, sy, TILE.PATH); b.set(xx, sy + 1, TILE.PATH); }
    });

    // plaza at a central crossing
    if (streetsX.length && streetsY.length) {
      var cx = streetsX[rng.int(streetsX.length)], cy = streetsY[rng.int(streetsY.length)];
      var pw = rng.irange(8, 11), ph = rng.irange(6, 8);
      b.rect(clamp(cx - (pw >> 1), 1, b.w - pw - 1), clamp(cy - (ph >> 1), 1, b.h - ph - 1), pw, ph, TILE.PATH);
    }

    // buildings fill the blocks; doors spill onto the streets via alleys
    var attempts = Math.floor((b.w * b.h) / 16);
    for (var a = 0; a < attempts; a++) {
      var rw = rng.irange(4, 8), rh = rng.irange(3, 6);
      var rx = rng.irange(2, b.w - rw - 3), ry = rng.irange(2, b.h - rh - 3);
      var ok = true;
      for (var yy2 = ry - 1; yy2 < ry + rh + 1 && ok; yy2++) {
        for (var xx2 = rx - 1; xx2 < rx + rw + 1 && ok; xx2++) {
          var t = b.get(xx2, yy2);
          if (t !== ground && t !== TILE.RUBBLE) ok = false;
        }
      }
      if (!ok) continue;
      b.rect(rx, ry, rw, rh, TILE.WALL);
      b.rect(rx + 1, ry + 1, rw - 2, rh - 2, TILE.FLOOR);
      b.addRoom(rx + 1, ry + 1, rw - 2, rh - 2);
      // door + alley stub toward the nearest street
      var side = rng.int(4);
      var dx2, dy2, doorX, doorY;
      if (side === 0) { doorX = rng.irange(rx + 1, rx + rw - 2); doorY = ry; dx2 = 0; dy2 = -1; }
      else if (side === 1) { doorX = rng.irange(rx + 1, rx + rw - 2); doorY = ry + rh - 1; dx2 = 0; dy2 = 1; }
      else if (side === 2) { doorX = rx; doorY = rng.irange(ry + 1, ry + rh - 2); dx2 = -1; dy2 = 0; }
      else { doorX = rx + rw - 1; doorY = rng.irange(ry + 1, ry + rh - 2); dx2 = 1; dy2 = 0; }
      b.set(doorX, doorY, TILE.FLOOR);
      var ax = doorX + dx2, ay = doorY + dy2;
      for (var step = 0; step < 10; step++) {
        var t2 = b.get(ax, ay);
        if (t2 === TILE.PATH || t2 === TILE.VOID || !b.inBounds(ax, ay)) break;
        if (t2 === ground || t2 === TILE.RUBBLE) b.set(ax, ay, TILE.PATH);
        else break;
        ax += dx2; ay += dy2;
      }
      // decay for the derelict variant
      if (theme.decay) {
        for (var wy = ry; wy < ry + rh; wy++) {
          for (var wx = rx; wx < rx + rw; wx++) {
            if (b.get(wx, wy) === TILE.WALL && rng.chance(0.3)) b.set(wx, wy, TILE.RUBBLE);
          }
        }
      }
    }

    // ground clutter between blocks
    for (var gy = 1; gy < b.h - 1; gy++) {
      for (var gx = 1; gx < b.w - 1; gx++) {
        if (b.get(gx, gy) !== ground) continue;
        var v = noise.fbm(gx * 0.1, gy * 0.1, 3);
        if (theme.decay && v > 0.66 && rng.chance(0.5)) b.set(gx, gy, TILE.TREE);
        else if (v < 0.22 && rng.chance(0.35)) b.set(gx, gy, TILE.RUBBLE);
      }
    }
  }

  function genRuins(b, theme) {
    var rng = b.rng;
    var ground = theme.ground === 'sand' ? TILE.SAND : TILE.GRASS;
    b.fill(ground);
    var noise = makeNoise(rng);

    // ground clutter: trees for overgrown, rocks for desert
    for (var y = 1; y < b.h - 1; y++) {
      for (var x = 1; x < b.w - 1; x++) {
        var v = noise.fbm(x * 0.09, y * 0.09, 4);
        if (theme.ground !== 'sand') {
          if (v > 0.62 && rng.chance(0.5)) b.set(x, y, TILE.TREE);
        } else {
          if (v > 0.78 && rng.chance(0.25)) b.set(x, y, TILE.ROCK);
        }
        if (v < 0.24 && rng.chance(0.4)) b.set(x, y, TILE.RUBBLE);
      }
    }

    // building shells
    var placed = [];
    var attempts = Math.floor((b.w * b.h) / 160);
    for (var a = 0; a < attempts; a++) {
      var rw = rng.irange(6, 12), rh = rng.irange(5, 9);
      var rx = rng.irange(2, b.w - rw - 3), ry = rng.irange(2, b.h - rh - 3);
      var ok = true;
      for (var p = 0; p < placed.length && ok; p++) {
        var o = placed[p];
        if (rx < o.x + o.w + 3 && rx + rw + 3 > o.x &&
            ry < o.y + o.h + 3 && ry + rh + 3 > o.y) ok = false;
      }
      if (!ok) continue;
      placed.push({ x: rx, y: ry, w: rw, h: rh });
      // walls + interior
      b.rect(rx, ry, rw, rh, TILE.WALL);
      b.rect(rx + 1, ry + 1, rw - 2, rh - 2, TILE.FLOOR);
      b.addRoom(rx + 1, ry + 1, rw - 2, rh - 2);
      // door gap on a random side
      var side = rng.int(4);
      var gx, gy;
      if (side === 0) { gx = rng.irange(rx + 1, rx + rw - 2); gy = ry; }
      else if (side === 1) { gx = rng.irange(rx + 1, rx + rw - 2); gy = ry + rh - 1; }
      else if (side === 2) { gx = rx; gy = rng.irange(ry + 1, ry + rh - 2); }
      else { gx = rx + rw - 1; gy = rng.irange(ry + 1, ry + rh - 2); }
      b.set(gx, gy, TILE.FLOOR);
      // decay
      var decay = rng.chance(0.35) ? 0.4 : 0.18;
      for (var wy = ry; wy < ry + rh; wy++) {
        for (var wx = rx; wx < rx + rw; wx++) {
          if (b.get(wx, wy) === TILE.WALL && rng.chance(decay)) b.set(wx, wy, TILE.RUBBLE);
          else if (b.get(wx, wy) === TILE.FLOOR && rng.chance(0.07)) b.set(wx, wy, TILE.RUBBLE);
        }
      }
    }

    // worn paths between buildings
    function pathCost(t) {
      if (t === TILE.WALL) return 90;
      if (t === TILE.TREE) return 12;
      if (t === TILE.ROCK) return 14;
      if (t === TILE.WATER) return 25;
      if (t === TILE.DEEP) return 50;
      if (t === TILE.PATH) return 0.5;
      return 1.5;
    }
    for (p = 0; p + 1 < placed.length; p++) {
      var A = placed[p], B = placed[p + 1];
      var path = aStar(b,
        clamp(A.x + (A.w >> 1), 1, b.w - 2), clamp(A.y + A.h, 1, b.h - 2),
        clamp(B.x + (B.w >> 1), 1, b.w - 2), clamp(B.y + B.h, 1, b.h - 2),
        pathCost);
      if (!path) continue;
      path.forEach(function (c) {
        var t = b.get(c[0], c[1]);
        if (t === ground || t === TILE.RUBBLE || t === TILE.TREE || t === TILE.ROCK) b.set(c[0], c[1], TILE.PATH);
        else if (t === TILE.WATER || t === TILE.DEEP) b.set(c[0], c[1], TILE.BRIDGE);
        else if (t === TILE.WALL) b.set(c[0], c[1], TILE.RUBBLE);
        var t2 = b.get(c[0] + 1, c[1]);
        if ((t2 === ground || t2 === TILE.TREE) && rng.chance(0.55)) b.set(c[0] + 1, c[1], TILE.PATH);
      });
    }
  }

  function genWilds(b, theme) {
    var rng = b.rng;
    var noise = makeNoise(rng);
    var moist = makeNoise(rng);
    var waterLevel = theme.waterLevel != null ? theme.waterLevel : 0.33;
    var treeDensity = theme.treeDensity != null ? theme.treeDensity : 0.55;
    var liquidTile = theme.liquid === 'lava' ? TILE.LAVA : TILE.WATER;
    var deepTile = theme.liquid === 'lava' ? TILE.LAVA : TILE.DEEP;

    for (var y = 0; y < b.h; y++) {
      for (var x = 0; x < b.w; x++) {
        var e = noise.fbm(x * 0.06, y * 0.06, 4);
        var m = (moist.fbm(x * 0.08, y * 0.08, 3) - 0.5) * 3 + 0.5; // widen the fbm's narrow band
        var edge = Math.min(x, y, b.w - 1 - x, b.h - 1 - y);
        var t;
        if (edge < 2 || (edge < 4 && rng.chance(0.7 - edge * 0.15))) t = TILE.TREE;
        else if (e < waterLevel - 0.06) t = deepTile;
        else if (e < waterLevel) t = liquidTile;
        else if (e > 0.8) t = TILE.ROCK;
        else if (theme.murky && m > 0.75 && e < 0.55) t = TILE.MURK;
        else if (m > 0.52 && rng.chance((m - 0.52) * treeDensity * 2.6)) t = TILE.TREE;
        else t = TILE.GRASS;
        b.set(x, y, t);
      }
    }

    // a river crossing the whole map
    if (theme.river !== false) {
      var rx = rng.irange(Math.floor(b.w * 0.25), Math.floor(b.w * 0.75));
      var drift = 0;
      for (y = 0; y < b.h; y++) {
        drift += rng.range(-0.9, 0.9);
        drift = clamp(drift, -3, 3);
        var cx = clamp(Math.round(rx + drift + Math.sin(y * 0.25) * 2.5), 2, b.w - 3);
        for (var wdx = -1; wdx <= 1; wdx++) {
          b.set(cx + wdx, y, wdx === 0 ? deepTile : liquidTile);
        }
        rx += rng.range(-0.4, 0.4);
      }
    }

    // clearings with a point of interest
    var clearings = rng.irange(2, 4);
    for (var c = 0; c < clearings; c++) {
      var ccx = rng.irange(8, b.w - 9), ccy = rng.irange(8, b.h - 9);
      var rad = rng.irange(2, 4);
      for (var dy = -rad; dy <= rad; dy++) {
        for (var dx = -rad; dx <= rad; dx++) {
          if (dx * dx + dy * dy <= rad * rad) {
            var tt = b.get(ccx + dx, ccy + dy);
            if (tt === TILE.TREE || tt === TILE.ROCK || tt === TILE.MURK) b.set(ccx + dx, ccy + dy, TILE.GRASS);
          }
        }
      }
      if (theme.stoneCircles && c > 0) {
        // a ring of standing stones around the clearing
        for (var k = 0; k < 6; k++) {
          var mx = ccx + Math.round(Math.cos(k * Math.PI / 3) * (rad - 0.5));
          var my = ccy + Math.round(Math.sin(k * Math.PI / 3) * (rad - 0.5));
          if (b.isWalkable(mx, my)) b.addEntity('monolith', mx, my);
        }
        if (b.isWalkable(ccx, ccy)) b.addEntity(rng.pick(['shrine', 'waystone']), ccx, ccy);
      } else if (b.isWalkable(ccx, ccy)) {
        b.addEntity(c === 0 ? 'shrine' : rng.pick(['campfire', 'waystone', 'statue']), ccx, ccy);
      }
    }

    // the road: west edge to east edge
    function roadCost(t) {
      if (t === TILE.GRASS || t === TILE.SAND) return 1;
      if (t === TILE.PATH) return 0.5;
      if (t === TILE.MURK) return 2;
      if (t === TILE.RUBBLE) return 1.5;
      if (t === TILE.TREE) return 7;
      if (t === TILE.ROCK) return 11;
      if (t === TILE.WATER) return 18;
      if (t === TILE.DEEP || t === TILE.LAVA) return 30;
      return 40;
    }
    function edgePoint(x) {
      for (var tries = 0; tries < 60; tries++) {
        var yy = b.rng.irange(3, b.h - 4);
        if (b.isWalkable(x, yy)) return [x, yy];
      }
      // force one
      var yy2 = b.rng.irange(3, b.h - 4);
      b.set(x, yy2, TILE.GRASS);
      return [x, yy2];
    }
    var west = edgePoint(2), east = edgePoint(b.w - 3);
    var road = aStar(b, west[0], west[1], east[0], east[1], roadCost);
    if (road) {
      road.forEach(function (cc) {
        var t = b.get(cc[0], cc[1]);
        if (t === TILE.WATER || t === TILE.DEEP || t === TILE.LAVA) b.set(cc[0], cc[1], TILE.BRIDGE);
        else if (t !== TILE.MURK) b.set(cc[0], cc[1], TILE.PATH);
        // widen the road so it reads at map scale
        [[0, 1], [1, 0]].forEach(function (d) {
          var t2 = b.get(cc[0] + d[0], cc[1] + d[1]);
          if ((t2 === TILE.GRASS || t2 === TILE.TREE || t2 === TILE.SAND) && rng.chance(0.7)) {
            b.set(cc[0] + d[0], cc[1] + d[1], TILE.PATH);
          }
        });
      });
      b.gates = [{ x: west[0], y: west[1] }, { x: east[0], y: east[1] }];
    }
  }

  // ---------------------------------------------------------------- gates + decoration

  function placeGates(b, carve) {
    if (b.gates && b.gates.length === 2) {
      b.entrance = b.gates[0];
      b.exit = b.gates[1];
    } else {
      // each room's anchor: the walkable tile nearest its center
      // (a room center can be flooded, e.g. by the sewer channel pass)
      var anchors = [];
      b.rooms.forEach(function (r) {
        var best = Infinity, ax = -1, ay = -1;
        for (var yy = r.y; yy < r.y + r.h; yy++) {
          for (var xx = r.x; xx < r.x + r.w; xx++) {
            if (!b.isWalkable(xx, yy)) continue;
            var d = Math.abs(xx - r.cx) + Math.abs(yy - r.cy);
            if (d < best) { best = d; ax = xx; ay = yy; }
          }
        }
        if (ax >= 0) anchors.push({ x: ax, y: ay });
      });
      // directional flow: the map has an axis; the portal sits in the
      // starting band, the boss in the far band. D2/PoE map pacing:
      // push forward, never backtrack.
      var axis = b.axis;
      var minP = Infinity, maxP = -Infinity;
      var walk = [];
      for (var wy = 0; wy < b.h; wy++) {
        for (var wx = 0; wx < b.w; wx++) {
          if (!walkable(b.tiles[wy * b.w + wx])) continue;
          var proj = wx * axis[0] + wy * axis[1];
          walk.push([wx, wy, proj]);
          if (proj < minP) minP = proj;
          if (proj > maxP) maxP = proj;
        }
      }
      var span = (maxP - minP) || 1;
      function band(lo, hi) {
        return walk.filter(function (c) {
          var t = (c[2] - minP) / span;
          return t >= lo && t <= hi;
        });
      }
      function pickIn(cands) {
        if (!cands.length) return null;
        // prefer a room anchor inside the band
        var inBand = anchors.filter(function (a) {
          return cands.some(function (c) { return c[0] === a.x && c[1] === a.y; });
        });
        if (inBand.length) return b.rng.pick(inBand);
        var c = b.rng.pick(cands);
        return { x: c[0], y: c[1] };
      }
      b.entrance = pickIn(band(0, 0.14)) || pickIn(band(0, 0.3));
      b.exit = pickIn(band(0.86, 1)) || pickIn(band(0.7, 1));
    }
    if (!b.entrance || !b.exit) {
      // fallback: farthest pair of walkable tiles
      var sx = -1, sy = -1;
      for (var i = 0; i < b.tiles.length; i++) {
        if (walkable(b.tiles[i])) { sx = i % b.w; sy = (i / b.w) | 0; break; }
      }
      var dA = bfs(b, sx, sy);
      var a = farthestIndex(dA);
      var dB = bfs(b, a % b.w, (a / b.w) | 0);
      var z = farthestIndex(dB);
      b.entrance = b.entrance || { x: a % b.w, y: (a / b.w) | 0 };
      b.exit = b.exit || { x: z % b.w, y: (z / b.w) | 0 };
    }

    // the boss arena: a fightable clearing carved around the far gate
    carveArena(b, b.exit, carve);
    b.boss = { x: b.exit.x, y: b.exit.y };
    // the exit portal sits just past the boss, deeper along the axis
    var ex = clamp(b.exit.x + b.axis[0] * 3, 1, b.w - 2);
    var ey = clamp(b.exit.y + b.axis[1] * 3, 1, b.h - 2);
    if (b.isWalkable(ex, ey) && !(ex === b.boss.x && ey === b.boss.y)) {
      b.exit = { x: ex, y: ey };
    }
    b.addEntity('entrance', b.entrance.x, b.entrance.y);
    b.addEntity('exit', b.exit.x, b.exit.y);
  }

  // Blast a roughly circular arena around a point. Only opens terrain,
  // so it can never disconnect the map; a safety tunnel links it to the
  // rest of the zone in case the center sat inside solid rock.
  function carveArena(b, at, carve) {
    var rng = b.rng;
    var r = rng.irange(4, 5);
    at.x = clamp(at.x, r + 1, b.w - r - 2);
    at.y = clamp(at.y, r + 1, b.h - r - 2);
    for (var dy = -r; dy <= r; dy++) {
      for (var dx = -r; dx <= r; dx++) {
        var d2 = dx * dx + dy * dy;
        if (d2 > r * r) continue;
        var x = at.x + dx, y = at.y + dy;
        var t = b.get(x, y);
        if (!walkable(t)) {
          // ragged rim: leave some solid nubs at the very edge
          if (d2 > (r - 1) * (r - 1) && rng.chance(0.35)) continue;
          b.set(x, y, carve);
        }
      }
    }
    var regs = walkableRegions(b);
    if (regs.length > 1) ensureConnected(b, carve);
  }

  // Shortest walk from the portal to the boss: the spine of the map.
  function computeMainPath(b) {
    return aStar(b, b.entrance.x, b.entrance.y, b.boss.x, b.boss.y, function (t) {
      return walkable(t) ? 1 : Infinity;
    }) || [];
  }

  // Monster pack placement: packs pace the main path, extra packs and
  // elites live in side pockets off it, the boss holds the arena.
  function placeSpawns(b, path) {
    var rng = b.rng;
    var spawns = [];
    var used = new Set();
    function jitterSpot(x, y) {
      for (var tries = 0; tries < 12; tries++) {
        var nx = clamp(x + rng.irange(-2, 2), 1, b.w - 2);
        var ny = clamp(y + rng.irange(-2, 2), 1, b.h - 2);
        if (b.isWalkable(nx, ny) && !used.has(nx + ',' + ny)) return [nx, ny];
      }
      return null;
    }
    function add(type, x, y) {
      used.add(x + ',' + y);
      spawns.push({ type: type, x: x, y: y });
    }

    // along the spine
    var next = rng.irange(6, 10);
    for (var i = 0; i < path.length; i++) {
      if (i < next) continue;
      if (Math.abs(path[i][0] - b.boss.x) + Math.abs(path[i][1] - b.boss.y) < 7) break;
      var s = jitterSpot(path[i][0], path[i][1]);
      if (s) add('pack', s[0], s[1]);
      next = i + rng.irange(7, 12);
    }

    // distance-from-spine field over walkable tiles
    var dist = new Int32Array(b.w * b.h);
    dist.fill(-1);
    var queue = new Int32Array(b.w * b.h);
    var head = 0, tail = 0;
    path.forEach(function (c) {
      var idx = c[1] * b.w + c[0];
      if (dist[idx] === -1) { dist[idx] = 0; queue[tail++] = idx; }
    });
    while (head < tail) {
      var qi = queue[head++];
      var qx = qi % b.w, qy = (qi / b.w) | 0;
      for (var d = 0; d < 4; d++) {
        var nx = qx + DIR4[d][0], ny = qy + DIR4[d][1];
        if (nx < 0 || ny < 0 || nx >= b.w || ny >= b.h) continue;
        var ni = ny * b.w + nx;
        if (dist[ni] === -1 && walkable(b.tiles[ni])) {
          dist[ni] = dist[qi] + 1;
          queue[tail++] = ni;
        }
      }
    }

    // side pockets: reward leaving the spine, elites in the deep ones
    var pockets = [];
    for (var y = 1; y < b.h - 1; y++) {
      for (var x = 1; x < b.w - 1; x++) {
        var di = dist[y * b.w + x];
        if (di >= 6) pockets.push([x, y, di]);
      }
    }
    rng.shuffle(pockets);
    var want = Math.max(2, Math.floor(b.w * b.h / 260));
    var placed = [];
    for (var p = 0; p < pockets.length && placed.length < want; p++) {
      var c = pockets[p];
      var ok = true;
      for (var q = 0; q < placed.length; q++) {
        if (Math.abs(placed[q][0] - c[0]) + Math.abs(placed[q][1] - c[1]) < 8) { ok = false; break; }
      }
      if (!ok) continue;
      placed.push(c);
      add(c[2] >= 10 && rng.chance(0.45) ? 'elite' : 'pack', c[0], c[1]);
    }

    // the arena
    add('boss', b.boss.x, b.boss.y);
    for (var g = 0; g < 2; g++) {
      var gs = jitterSpot(b.boss.x, b.boss.y);
      if (gs) add('pack', gs[0], gs[1]);
    }
    b.spawns = spawns;
  }

  function decorate(b, theme) {
    var rng = b.rng;
    var area = b.w * b.h;
    var occupied = new Set();
    b.entities.forEach(function (e) { occupied.add(e.x + ',' + e.y); });
    (b.spawns || []).forEach(function (s) { occupied.add(s.x + ',' + s.y); });
    function free(x, y) { return !occupied.has(x + ',' + y); }
    function take(type, x, y) {
      occupied.add(x + ',' + y);
      b.addEntity(type, x, y);
    }

    // wall torches for dark interior themes
    if (theme.torches) {
      var candidates = [];
      for (var y = 1; y < b.h - 1; y++) {
        for (var x = 1; x < b.w - 1; x++) {
          if (b.get(x, y) === TILE.WALL && b.isWalkable(x, y + 1)) candidates.push([x, y]);
        }
      }
      rng.shuffle(candidates);
      var maxTorches = Math.floor(area / 42);
      var placedT = [];
      for (var c = 0; c < candidates.length && placedT.length < maxTorches; c++) {
        var t = candidates[c];
        var okT = true;
        for (var p = 0; p < placedT.length; p++) {
          if (Math.abs(placedT[p][0] - t[0]) < 5 && Math.abs(placedT[p][1] - t[1]) < 4) { okT = false; break; }
        }
        if (okT) { placedT.push(t); take('torch', t[0], t[1]); }
      }
    }

    // chests in dead ends (or room corners as fallback)
    var deadEnds = [];
    for (y = 1; y < b.h - 1; y++) {
      for (x = 1; x < b.w - 1; x++) {
        if (!b.isWalkable(x, y) || b.get(x, y) === TILE.DOOR) continue;
        var open = 0;
        for (var d = 0; d < 4; d++) {
          if (b.isWalkable(x + DIR4[d][0], y + DIR4[d][1])) open++;
        }
        if (open === 1 && free(x, y)) deadEnds.push([x, y]);
      }
    }
    rng.shuffle(deadEnds);
    var chestCount = 2 + Math.floor(area / 1400);
    for (c = 0; c < Math.min(chestCount, deadEnds.length); c++) {
      take('chest', deadEnds[c][0], deadEnds[c][1]);
    }
    if (chestCount > deadEnds.length && b.rooms.length) {
      var need = chestCount - deadEnds.length;
      for (c = 0; c < need; c++) {
        var room = rng.pick(b.rooms);
        var cx = rng.chance(0.5) ? room.x : room.x + room.w - 1;
        var cy = rng.chance(0.5) ? room.y : room.y + room.h - 1;
        if (b.isWalkable(cx, cy) && free(cx, cy)) take('chest', cx, cy);
      }
    }

    // a shrine in an open spot far from the entrance
    if (!b.entities.some(function (e) { return e.type === 'shrine'; })) {
      var dist = bfs(b, b.entrance.x, b.entrance.y);
      var maxD = 0;
      for (var i = 0; i < dist.length; i++) if (dist[i] > maxD) maxD = dist[i];
      var spots = [];
      for (y = 2; y < b.h - 2; y++) {
        for (x = 2; x < b.w - 2; x++) {
          if (!free(x, y)) continue;
          var di = dist[y * b.w + x];
          if (di < maxD * 0.35) continue;
          var openAll = true;
          for (var dy = -1; dy <= 1 && openAll; dy++)
            for (var dx = -1; dx <= 1 && openAll; dx++)
              if (!b.isWalkable(x + dx, y + dy)) openAll = false;
          if (openAll) spots.push([x, y]);
        }
      }
      if (spots.length) {
        var s = rng.pick(spots);
        take('shrine', s[0], s[1]);
      }
    }

    // pillars hold up the big chambers
    b.rooms.forEach(function (room) {
      if (room.w < 7 || room.h < 6) return;
      var x1 = room.x + 2, x2 = room.x + room.w - 3;
      var y1 = room.y + 2, y2 = room.y + room.h - 3;
      [[x1, y1], [x2, y1], [x1, y2], [x2, y2]].forEach(function (p) {
        if (b.isWalkable(p[0], p[1]) && free(p[0], p[1]) && rng.chance(0.85)) {
          take('pillar', p[0], p[1]);
        }
      });
    });

    // theme scatter decor
    var decor = theme.decor || {};
    var wallAdj = null; // lazily built list of walkable tiles hugging a wall
    function wallAdjacent() {
      if (wallAdj) return wallAdj;
      wallAdj = [];
      for (var yy = 1; yy < b.h - 1; yy++) {
        for (var xx = 1; xx < b.w - 1; xx++) {
          if (!b.isWalkable(xx, yy)) continue;
          for (var dd = 0; dd < 4; dd++) {
            if (b.get(xx + DIR4[dd][0], yy + DIR4[dd][1]) === TILE.WALL) { wallAdj.push([xx, yy]); break; }
          }
        }
      }
      return rng.shuffle(wallAdj);
    }
    var nearWallTypes = new Set(['statue', 'sarcophagus', 'barrel', 'crystal', 'web']);
    Object.keys(decor).forEach(function (type) {
      var count = Math.round(decor[type] * area / 1000 * rng.range(0.7, 1.3));
      var pool = nearWallTypes.has(type) ? wallAdjacent() : null;
      var placedN = 0, guard = count * 30;
      while (placedN < count && guard-- > 0) {
        var px, py;
        if (pool && pool.length) {
          var pc = pool[rng.int(pool.length)];
          px = pc[0]; py = pc[1];
        } else {
          px = rng.irange(1, b.w - 2); py = rng.irange(1, b.h - 2);
        }
        if (!b.isWalkable(px, py) || !free(px, py)) continue;
        // keep gates approachable
        if (Math.abs(px - b.entrance.x) + Math.abs(py - b.entrance.y) < 3) continue;
        if (Math.abs(px - b.exit.x) + Math.abs(py - b.exit.y) < 3) continue;
        take(type, px, py);
        placedN++;
      }
    });
  }

  // ---------------------------------------------------------------- themes

  var DEFAULT_PALETTE = {
    bg: '#0b0b10',
    floor: ['#4a4754', '#413e4e'],
    wallTop: '#2b2a3c', wallFace: '#17161f',
    door: '#7a5230',
    water: '#1e4258', deep: '#132b3c',
    lava: ['#ff8a2a', '#b53410'],
    grass: ['#3a5230', '#324828'],
    tree: ['#24361f', '#2e4527'],
    rock: '#5c5a66',
    path: ['#585049', '#4c4540'],
    sand: ['#c4ab7d', '#b89f72'],
    rubble: '#4f4c56',
    murk: '#2e3d2c',
    bridge: '#6f4e2c',
    accent: '#c9a227',
    light: '255,190,110',
    darkness: 0.55
  };

  function theme(id, name, zone, over) {
    var pal = {};
    Object.keys(DEFAULT_PALETTE).forEach(function (k) { pal[k] = DEFAULT_PALETTE[k]; });
    Object.keys(over.palette || {}).forEach(function (k) { pal[k] = over.palette[k]; });
    return {
      id: id, name: name, zone: zone,
      palette: pal,
      torches: !!over.torches,
      channel: !!over.channel,
      flooded: !!over.flooded,
      murky: !!over.murky,
      river: over.river,
      liquid: over.liquid,
      liquidLevel: over.liquidLevel,
      waterLevel: over.waterLevel,
      treeDensity: over.treeDensity,
      ground: over.ground,
      glowShrooms: !!over.glowShrooms,
      outdoor: !!over.outdoor,
      piers: !!over.piers,
      decay: !!over.decay,
      stoneCircles: !!over.stoneCircles,
      decor: over.decor || {}
    };
  }

  var THEMES = {
    crypt: theme('crypt', 'Crypt', 'dungeon', {
      torches: true,
      palette: {
        floor: ['#4b4759', '#413d4e'], wallTop: '#2d2c42', wallFace: '#15151f',
        accent: '#8fd0c8', light: '255,186,102', darkness: 0.4
      },
      decor: { bones: 4, statue: 1.1, sarcophagus: 1.4, barrel: 0.5 }
    }),
    fortress: theme('fortress', 'Fortress', 'dungeon', {
      torches: true,
      palette: {
        floor: ['#5c5045', '#50453c'], wallTop: '#38302a', wallFace: '#221c17',
        accent: '#c04b3a', light: '255,196,120', darkness: 0.38
      },
      decor: { barrel: 2.4, statue: 0.9, bones: 0.8 }
    }),
    sewer: theme('sewer', 'Sewer', 'dungeon', {
      torches: true, channel: true,
      palette: {
        floor: ['#48524a', '#3e473f'], wallTop: '#2a332b', wallFace: '#181f19',
        water: '#3a5232', deep: '#273a22',
        accent: '#a4c94a', light: '190,220,130', darkness: 0.55
      },
      decor: { barrel: 1.4, bones: 1.6, mushroom: 1, plant: 0.8 }
    }),
    cavern: theme('cavern', 'Cavern', 'caves', {
      torches: true, liquid: 'water', liquidLevel: 0.32,
      palette: {
        floor: ['#5c5143', '#50463b'], wallTop: '#332c24', wallFace: '#1e1a14',
        accent: '#d8b26a', light: '255,190,110', darkness: 0.52
      },
      decor: { bones: 1.4, mushroom: 1, crystal: 0.6, rock: 0 }
    }),
    ice: theme('ice', 'Ice', 'caves', {
      liquid: 'water', liquidLevel: 0.3,
      palette: {
        floor: ['#8ea6b8', '#7e96aa'], wallTop: '#3c5468', wallFace: '#243848',
        water: '#3e6a8a', deep: '#28495f', rock: '#a8bcc8',
        accent: '#9fe8ff', light: '150,220,255', darkness: 0.4
      },
      decor: { crystal: 2.6, bones: 0.9 }
    }),
    lava: theme('lava', 'Lava', 'caves', {
      liquid: 'lava', liquidLevel: 0.34,
      palette: {
        floor: ['#4a3c38', '#413430'], wallTop: '#241c1a', wallFace: '#140e0d',
        rock: '#4c3c38',
        accent: '#ff9d3f', light: '255,140,60', darkness: 0.38
      },
      decor: { ember: 3.4, bones: 1.2 }
    }),
    fungal: theme('fungal', 'Fungal', 'caves', {
      liquid: 'water', liquidLevel: 0.28, glowShrooms: true,
      palette: {
        floor: ['#544a66', '#483f58'], wallTop: '#2e2740', wallFace: '#181322',
        water: '#274054', deep: '#1a2c3c',
        accent: '#5ee0c0', light: '110,230,190', darkness: 0.44
      },
      decor: { mushroom: 5.5, plant: 1.6, crystal: 0.5 }
    }),
    bone: theme('bone', 'Bone', 'catacombs', {
      torches: true,
      palette: {
        floor: ['#68604c', '#5a5341'], wallTop: '#3e3729', wallFace: '#26211a',
        accent: '#e8ddb8', light: '255,196,120', darkness: 0.56
      },
      decor: { bones: 7, sarcophagus: 2.2, statue: 0.7 }
    }),
    flooded: theme('flooded', 'Flooded', 'catacombs', {
      torches: true, flooded: true,
      palette: {
        floor: ['#3f505c', '#374650'], wallTop: '#243039', wallFace: '#141d24',
        water: '#2a5060', deep: '#1a3441',
        accent: '#6ac2c9', light: '160,220,220', darkness: 0.56
      },
      decor: { bones: 1.8, barrel: 0.9, mushroom: 0.9, plant: 0.8 }
    }),
    desert: theme('desert', 'Desert', 'ruins', {
      outdoor: true, ground: 'sand',
      palette: {
        bg: '#141009',
        floor: ['#b09468', '#a3885f'], wallTop: '#8f7550', wallFace: '#655138',
        rock: '#8a7458', rubble: '#93805e', path: ['#a08b62', '#948057'],
        accent: '#e8c56a', light: '255,220,150', darkness: 0.12
      },
      decor: { statue: 1, bones: 1.8, plant: 0.4, barrel: 0.4 }
    }),
    overgrown: theme('overgrown', 'Overgrown', 'ruins', {
      outdoor: true,
      palette: {
        bg: '#0a0f0a',
        floor: ['#4c5544', '#434b3c'], wallTop: '#4f5a4a', wallFace: '#333c30',
        grass: ['#4a6a3c', '#426034'], tree: ['#2a4423', '#35522c'],
        path: ['#5f584a', '#534d41'], rubble: '#4d5347',
        accent: '#8fce6a', light: '190,235,150', darkness: 0.18
      },
      decor: { plant: 4.5, statue: 1.1, bones: 0.8, mushroom: 0.8 }
    }),
    forest: theme('forest', 'Forest', 'wilds', {
      outdoor: true, waterLevel: 0.31, treeDensity: 0.65,
      palette: {
        bg: '#090d08',
        grass: ['#4e7038', '#446430'], tree: ['#26471f', '#33582a'],
        water: '#2a5a6a', deep: '#1a3d4c',
        path: ['#6a5c46', '#5d513d'], rock: '#6a6a62',
        accent: '#a8d878', light: '220,240,170', darkness: 0.16
      },
      decor: { plant: 3.5, mushroom: 0.8, bones: 0.25 }
    }),
    swamp: theme('swamp', 'Swamp', 'wilds', {
      outdoor: true, waterLevel: 0.4, treeDensity: 0.42, murky: true,
      palette: {
        bg: '#0a0c08',
        grass: ['#4a5232', '#41482b'], tree: ['#2c351f', '#363f27'],
        water: '#3a4a34', deep: '#273424', murk: '#3c452c',
        path: ['#5c5340', '#504838'], rock: '#5c5c50',
        accent: '#b0c860', light: '190,210,120', darkness: 0.38
      },
      decor: { plant: 2.8, mushroom: 1.8, bones: 0.9 }
    }),
    prison: theme('prison', 'Prison', 'dungeon', {
      torches: true,
      palette: {
        floor: ['#474b54', '#3d414a'], wallTop: '#2e313a', wallFace: '#1a1c22',
        door: '#5a5f6a',
        accent: '#8a9bb0', light: '210,220,255', darkness: 0.46
      },
      decor: { bones: 3.2, barrel: 1.2, statue: 0.5 }
    }),
    spider: theme('spider', 'Spider', 'caves', {
      liquid: 'none',
      palette: {
        floor: ['#453c4a', '#3b3340'], wallTop: '#2b2530', wallFace: '#17141c',
        accent: '#cfd8e8', light: '190,200,230', darkness: 0.52
      },
      decor: { web: 4.5, bones: 2.8, crystal: 0.4, mushroom: 0.6 }
    }),
    mines: theme('mines', 'Mines', 'caves', {
      torches: true, liquid: 'water', liquidLevel: 0.26,
      palette: {
        floor: ['#5e4c3a', '#523f30'], wallTop: '#3a2e22', wallFace: '#221a12',
        accent: '#e8b45a', light: '255,196,110', darkness: 0.5
      },
      decor: { crystal: 1.6, barrel: 1.8, bones: 1, plant: 0.3 }
    }),
    tomb: theme('tomb', 'Tomb', 'catacombs', {
      torches: true,
      palette: {
        floor: ['#8a7452', '#7a6646'], wallTop: '#5a4a32', wallFace: '#382e1e',
        accent: '#e8c56a', light: '255,214,130', darkness: 0.5
      },
      decor: { sarcophagus: 3, bones: 3.5, statue: 1.2, barrel: 0.4 }
    }),
    arcane: theme('arcane', 'Arcane', 'sanctum', {
      palette: {
        bg: '#0a0916',
        floor: ['#6a6390', '#5c567e'], wallTop: '#3a3558', wallFace: '#242040',
        accent: '#9f8fff', light: '160,140,255', darkness: 0.42,
        voidGlow: true
      },
      decor: { crystal: 2.2, statue: 1, bones: 0.4 }
    }),
    infernal: theme('infernal', 'Infernal', 'sanctum', {
      liquid: 'lava',
      palette: {
        bg: '#120806',
        floor: ['#4c3b38', '#423230'], wallTop: '#2c201c', wallFace: '#180f0c',
        accent: '#ff9d3f', light: '255,130,55', darkness: 0.5,
        voidGlow: true
      },
      decor: { ember: 3, statue: 1.4, bones: 1.8 }
    }),
    coast: theme('coast', 'Coast', 'shore', {
      outdoor: true,
      palette: {
        bg: '#0a0d10',
        sand: ['#d8c08e', '#ccb482'], grass: ['#5c7c44', '#527239'],
        tree: ['#2c4626', '#375630'], water: '#3a7a97', deep: '#26586f',
        rock: '#8a8578', path: ['#c2ab7c', '#b6a072'],
        accent: '#ffe08a', light: '255,230,170', darkness: 0.1
      },
      decor: { plant: 1.6, bones: 0.5, barrel: 0.4, rock: 0 }
    }),
    harbor: theme('harbor', 'Harbor', 'shore', {
      outdoor: true, piers: true,
      palette: {
        bg: '#090b0e',
        sand: ['#b09a70', '#a89066'], grass: ['#4c6039', '#445630'],
        tree: ['#26381f', '#304829'], water: '#2c5a74', deep: '#1c3e52',
        rock: '#6e6a5e', bridge: '#7a583a', path: ['#a08e66', '#94825c'],
        accent: '#ffcf7a', light: '255,200,130', darkness: 0.35
      },
      decor: { barrel: 3, plant: 0.8, bones: 0.7 }
    }),
    market: theme('market', 'Market', 'city', {
      outdoor: true, ground: 'sand',
      palette: {
        bg: '#12100a',
        sand: ['#c2a878', '#b69c6e'], wallTop: '#977c54', wallFace: '#6a5538',
        path: ['#ab9268', '#9d8560'], rubble: '#9a8662',
        accent: '#ffd677', light: '255,220,150', darkness: 0.14
      },
      decor: { barrel: 3, statue: 1.2, plant: 0.5, bones: 0.3 }
    }),
    derelict: theme('derelict', 'Derelict', 'city', {
      outdoor: true, decay: true,
      palette: {
        bg: '#0a0c0a',
        grass: ['#4e5844', '#454e3b'], wallTop: '#5c625a', wallFace: '#3a3f38',
        tree: ['#2a3d24', '#34492c'], path: ['#6a6456', '#5e594c'],
        rubble: '#565c50',
        accent: '#9fce7a', light: '200,235,160', darkness: 0.34
      },
      decor: { rubble: 0, plant: 3, bones: 1.4, statue: 1, barrel: 0.8 }
    }),
    tundra: theme('tundra', 'Tundra', 'wilds', {
      outdoor: true, waterLevel: 0.3, treeDensity: 0.5,
      palette: {
        bg: '#0b0e10',
        grass: ['#c4ced6', '#b4c2cc'], tree: ['#2e4a3c', '#3a5a4a'],
        water: '#5a92b0', deep: '#3a6684', rock: '#8a949a',
        path: ['#a6a89e', '#989a90'], murk: '#8aa2ac',
        accent: '#bfe8ff', light: '200,235,255', darkness: 0.2
      },
      decor: { rock: 0, bones: 0.7, plant: 0.5, crystal: 0.3 }
    }),
    moor: theme('moor', 'Moor', 'wilds', {
      outdoor: true, waterLevel: 0.27, treeDensity: 0.22, stoneCircles: true,
      palette: {
        bg: '#090a08',
        grass: ['#485438', '#404a31'], tree: ['#2c3823', '#36452c'],
        water: '#35505a', deep: '#22363e', rock: '#68685c',
        path: ['#5e5646', '#524c3e'],
        accent: '#a8c890', light: '200,220,160', darkness: 0.4
      },
      decor: { bones: 1.6, statue: 0.8, plant: 1.4, mushroom: 0.5 }
    }),
    ash: theme('ash', 'Ash', 'wilds', {
      outdoor: true, waterLevel: 0.3, treeDensity: 0.3, liquid: 'lava', river: false,
      palette: {
        bg: '#0d0a09',
        grass: ['#4c4642', '#433e3a'], tree: ['#332c28', '#3c332e'],
        path: ['#5c524c', '#514842'], rock: '#5a504a',
        accent: '#ff9d3f', light: '255,140,60', darkness: 0.38
      },
      decor: { ember: 3.2, bones: 1.6, plant: 0.3 }
    })
  };

  // ---------------------------------------------------------------- zones

  var ZONES = {
    dungeon: { id: 'dungeon', name: 'Dungeon', gen: genDungeon, carve: TILE.FLOOR, themes: ['crypt', 'fortress', 'sewer', 'prison'] },
    caves: { id: 'caves', name: 'Caves', gen: genCaves, carve: TILE.FLOOR, themes: ['cavern', 'ice', 'lava', 'fungal', 'spider', 'mines'] },
    catacombs: { id: 'catacombs', name: 'Catacombs', gen: genCatacombs, carve: TILE.FLOOR, themes: ['bone', 'flooded', 'tomb'] },
    sanctum: { id: 'sanctum', name: 'Sanctum', gen: genSanctum, carve: TILE.FLOOR, themes: ['arcane', 'infernal'] },
    ruins: { id: 'ruins', name: 'Ruins', gen: genRuins, carve: TILE.PATH, themes: ['desert', 'overgrown'] },
    city: { id: 'city', name: 'City', gen: genCity, carve: TILE.PATH, themes: ['market', 'derelict'] },
    shore: { id: 'shore', name: 'Shore', gen: genShore, carve: TILE.PATH, themes: ['coast', 'harbor'] },
    wilds: { id: 'wilds', name: 'Wilds', gen: genWilds, carve: TILE.PATH, themes: ['forest', 'swamp', 'ash', 'tundra', 'moor'] }
  };

  // ---------------------------------------------------------------- api

  function generate(opts) {
    opts = opts || {};
    var zone = ZONES[opts.zone] || ZONES.dungeon;
    var seed = opts.seed;
    if (seed === undefined || seed === null || seed === '') seed = (Math.random() * 0xFFFFFFFF) >>> 0;
    if (typeof seed === 'string') seed = /^\d+$/.test(seed) ? (parseInt(seed, 10) >>> 0) : hashString(seed);
    seed = seed >>> 0;
    var rng = new RNG(seed);
    var themeId = (opts.theme && zone.themes.indexOf(opts.theme) >= 0) ? opts.theme : rng.pick(zone.themes);
    var th = THEMES[themeId];
    var width = clamp(Math.floor(opts.width || 72), 24, 256);
    var height = clamp(Math.floor(opts.height || 54), 24, 256);

    var b = new MapBuilder(width, height, rng);
    zone.gen(b, th);
    ensureConnected(b, zone.carve);
    b.axis = b.gates ? [1, 0] : rng.pick(zone.axes || [[1, 0], [-1, 0], [0, 1], [0, -1]]);
    placeGates(b, zone.carve);
    var mainPath = computeMainPath(b);
    placeSpawns(b, mainPath);
    decorate(b, th);

    return {
      version: VERSION,
      seed: seed,
      zone: zone.id,
      theme: themeId,
      width: width,
      height: height,
      tiles: b.tiles,
      entities: b.entities,
      rooms: b.rooms,
      entrance: b.entrance,
      exit: b.exit,
      boss: b.boss,
      axis: b.axis,
      mainPath: mainPath,
      spawns: b.spawns,
      palette: th.palette,
      outdoor: th.outdoor,
      glowShrooms: th.glowShrooms
    };
  }

  // Plain-JSON form: tiles become hex strings (one char per tile, one string per row).
  function toJSON(map) {
    var rows = [];
    for (var y = 0; y < map.height; y++) {
      var row = '';
      for (var x = 0; x < map.width; x++) row += map.tiles[y * map.width + x].toString(16);
      rows.push(row);
    }
    return {
      version: map.version,
      generator: 'WIZARD Cartographer',
      seed: map.seed,
      zone: map.zone,
      theme: map.theme,
      width: map.width,
      height: map.height,
      legend: TILE_NAMES,
      tiles: rows,
      entities: map.entities,
      entrance: map.entrance,
      exit: map.exit,
      boss: map.boss,
      axis: map.axis,
      mainPath: map.mainPath,
      spawns: map.spawns,
      palette: map.palette
    };
  }

  function fromJSON(data) {
    var tiles = new Uint8Array(data.width * data.height);
    for (var y = 0; y < data.height; y++) {
      for (var x = 0; x < data.width; x++) {
        tiles[y * data.width + x] = parseInt(data.tiles[y][x], 16);
      }
    }
    var map = {};
    Object.keys(data).forEach(function (k) { map[k] = data[k]; });
    map.tiles = tiles;
    return map;
  }

  return {
    VERSION: VERSION,
    TILE: TILE,
    TILE_NAMES: TILE_NAMES,
    WALKABLE: WALKABLE,
    ZONES: ZONES,
    THEMES: THEMES,
    generate: generate,
    toJSON: toJSON,
    fromJSON: fromJSON,
    hashString: hashString
  };
});
