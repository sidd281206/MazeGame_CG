// ==========================================================================
// Maze Escape 3D - Three.js WebGL Engine & Gameplay Logic
// Faithfully matching maze_escape_3d.cpp with Modern Visuals & Audio
// ==========================================================================

const CELL_SIZE = 2.0;
const WALL_HEIGHT = 2.4;
const PLAYER_RADIUS = 0.35;
const EYE_HEIGHT = 1.0;
const MOVE_SPEED = 4.2;
const TURN_SPEED = 2.4;

class MazeEscapeGame {
  constructor() {
    this.state = 'MENU'; // MENU, PLAYING, PAUSED, LEVEL_DONE, GAME_OVER
    this.level = 1;
    this.score = 0;
    this.levelStartScore = 0;
    this.timeLeft = 0;
    this.totalTime = 0;
    this.orbs = [];
    this.orbsTaken = 0;
    
    // Player transform
    this.px = CELL_SIZE * 1.5;
    this.pz = CELL_SIZE * 1.5;
    this.yaw = 0; // facing direction (radians)
    this.pitch = 0; // looking up/down
    this.bobPhase = 0;
    this.isMoving = false;
    this.stepTimer = 0;

    // Grid data
    this.W = 15;
    this.H = 15;
    this.maze = [];
    this.seen = [];
    this.exitX = 0;
    this.exitZ = 0;

    // Input
    this.keys = {};
    this.isPointerLocked = false;
    this.showMinimap = true;

    // Three.js instances
    this.scene = null;
    this.camera = null;
    this.renderer = null;
    this.torchLight = null;
    this.torchFlickerTime = 0;
    this.mazeMeshes = [];
    this.orbMeshes = [];
    this.portalGroup = null;
    this.portalLight = null;
    this.portalRing1 = null;
    this.portalRing2 = null;
    this.portalCore = null;
    this.portalParticles = null;

    // Clock
    this.clock = new THREE.Clock();

    // DOM Elements
    this.dom = {
      container: document.getElementById('canvas-container'),
      hud: document.getElementById('hud'),
      levelText: document.getElementById('hud-level'),
      orbsText: document.getElementById('hud-orbs'),
      scoreText: document.getElementById('hud-score'),
      timerText: document.getElementById('hud-timer'),
      timerBar: document.getElementById('timer-bar'),
      minimapCard: document.getElementById('minimap-card'),
      minimapCanvas: document.getElementById('minimap-canvas'),
      statusToast: document.getElementById('status-toast'),
      toastText: document.getElementById('toast-text'),
      crosshair: document.getElementById('crosshair'),
      clickPrompt: document.getElementById('click-prompt'),
      audioBtn: document.getElementById('audio-btn'),
      // Modals
      menuModal: document.getElementById('menu-modal'),
      pauseModal: document.getElementById('pause-modal'),
      levelDoneModal: document.getElementById('level-done-modal'),
      gameOverModal: document.getElementById('game-over-modal'),
      // Modal buttons
      btnStart: document.getElementById('btn-start'),
      btnResume: document.getElementById('btn-resume'),
      btnRestartPause: document.getElementById('btn-restart-pause'),
      btnNextLevel: document.getElementById('btn-next-level'),
      btnRetry: document.getElementById('btn-retry'),
      // Stat displays
      doneBonusText: document.getElementById('done-bonus'),
      doneScoreText: document.getElementById('done-score'),
      overLevelText: document.getElementById('over-level'),
      overScoreText: document.getElementById('over-score')
    };

    this.minimapCtx = this.dom.minimapCanvas.getContext('2d');
    this.init();
  }

  // ------------------------------------------------------------------------
  // Setup & Initialization
  // ------------------------------------------------------------------------
  init() {
    this.setupThree();
    this.bindEvents();
    this.generateProceduralTextures();
    this.startLevel(1);
    this.animate();
  }

  setupThree() {
    const w = window.innerWidth;
    const h = window.innerHeight;

    // Scene with dark gothic atmospheric fog
    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0x04060c);
    this.scene.fog = new THREE.FogExp2(0x04060c, 0.11);

    // Camera
    this.camera = new THREE.PerspectiveCamera(72, w / h, 0.1, 100);
    this.camera.rotation.order = 'YXZ';

    // Renderer with soft shadow maps and high performance
    this.renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
    this.renderer.setSize(w, h);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
    this.renderer.toneMappingExposure = 1.1;
    this.dom.container.appendChild(this.renderer.domElement);

    // Global ambient light (subtle dungeon chill)
    const ambient = new THREE.AmbientLight(0x182033, 0.5);
    this.scene.add(ambient);

    // Mobile Torch Light (attached to camera / player)
    this.torchLight = new THREE.PointLight(0xffa834, 2.0, 14, 1.4);
    this.torchLight.castShadow = false;
    this.scene.add(this.torchLight);
  }

  // ------------------------------------------------------------------------
  // Procedural Textures (Authentic C++ Replica with Enhanced Fidelity)
  // ------------------------------------------------------------------------
  generateProceduralTextures() {
    // 1. Brick Wall Texture (Procedural Canvas)
    const S = 256;
    const wallCanvas = document.createElement('canvas');
    wallCanvas.width = S;
    wallCanvas.height = S;
    const wctx = wallCanvas.getContext('2d');

    // Base background mortar
    wctx.fillStyle = '#1c1b18';
    wctx.fillRect(0, 0, S, S);

    const rows = 8;
    const rowH = S / rows;
    const brickW = 64;

    for (let y = 0; y < rows; y++) {
      const offsetX = (y % 2 === 0) ? 0 : brickW / 2;
      for (let x = -brickW; x < S + brickW; x += brickW) {
        const shade = Math.floor(Math.random() * 25);
        const r = 115 + shade + (y * 4) % 15;
        const g = 52 + Math.floor(shade / 2);
        const b = 38 + Math.floor(shade / 3);

        wctx.fillStyle = `rgb(${r},${g},${b})`;
        wctx.fillRect(x + offsetX + 2, y * rowH + 2, brickW - 4, rowH - 4);

        // Brick surface noise grain
        for (let i = 0; i < 40; i++) {
          const nx = x + offsetX + 2 + Math.random() * (brickW - 4);
          const ny = y * rowH + 2 + Math.random() * (rowH - 4);
          wctx.fillStyle = Math.random() > 0.5 ? 'rgba(0,0,0,0.18)' : 'rgba(255,255,255,0.08)';
          wctx.fillRect(nx, ny, 2, 2);
        }

        // Inner bevel highlight / shadow
        wctx.fillStyle = 'rgba(255,255,255,0.12)';
        wctx.fillRect(x + offsetX + 2, y * rowH + 2, brickW - 4, 1);
        wctx.fillStyle = 'rgba(0,0,0,0.3)';
        wctx.fillRect(x + offsetX + 2, y * rowH + rowH - 3, brickW - 4, 1);
      }
    }

    this.wallTexture = new THREE.CanvasTexture(wallCanvas);
    this.wallTexture.wrapS = THREE.RepeatWrapping;
    this.wallTexture.wrapT = THREE.RepeatWrapping;

    // 2. Stone Floor Texture (Procedural Canvas)
    const floorCanvas = document.createElement('canvas');
    floorCanvas.width = S;
    floorCanvas.height = S;
    const fctx = floorCanvas.getContext('2d');

    fctx.fillStyle = '#0a0d14';
    fctx.fillRect(0, 0, S, S);

    const tiles = 4;
    const tileSize = S / tiles;
    for (let ty = 0; ty < tiles; ty++) {
      for (let tx = 0; tx < tiles; tx++) {
        const checker = ((tx + ty) % 2 === 0) ? 12 : 0;
        const baseVal = 70 + checker;

        fctx.fillStyle = `rgb(${baseVal},${baseVal + 2},${baseVal + 6})`;
        fctx.fillRect(tx * tileSize + 2, ty * tileSize + 2, tileSize - 4, tileSize - 4);

        // Stone texture noise
        for (let i = 0; i < 80; i++) {
          const nx = tx * tileSize + 2 + Math.random() * (tileSize - 4);
          const ny = ty * tileSize + 2 + Math.random() * (tileSize - 4);
          fctx.fillStyle = Math.random() > 0.5 ? 'rgba(255,255,255,0.06)' : 'rgba(0,0,0,0.15)';
          fctx.fillRect(nx, ny, 3, 3);
        }
      }
    }

    this.floorTexture = new THREE.CanvasTexture(floorCanvas);
    this.floorTexture.wrapS = THREE.RepeatWrapping;
    this.floorTexture.wrapT = THREE.RepeatWrapping;
  }

  // ------------------------------------------------------------------------
  // Procedural Maze Generator (Recursive Backtracker + Loop Carving)
  // ------------------------------------------------------------------------
  buildMaze(w, h) {
    this.W = w;
    this.H = h;
    this.maze = [];
    this.seen = [];

    // Initialize all as solid walls
    for (let z = 0; z < h; z++) {
      this.maze[z] = [];
      this.seen[z] = [];
      for (let x = 0; x < w; x++) {
        this.maze[z][x] = 1; // 1 = Wall, 0 = Passage
        this.seen[z][x] = false;
      }
    }

    // Iterative Recursive Backtracker
    const stack = [];
    const visited = [];
    for (let z = 0; z < h; z++) {
      visited[z] = [];
      for (let x = 0; x < w; x++) visited[z][x] = false;
    }

    const startX = 1, startZ = 1;
    this.maze[startZ][startX] = 0;
    visited[startZ][startX] = true;
    stack.push({ x: startX, z: startZ });

    const dirs = [
      { dx: 0, dz: -2 },
      { dx: 0, dz: 2 },
      { dx: -2, dz: 0 },
      { dx: 2, dz: 0 }
    ];

    while (stack.length > 0) {
      const cur = stack[stack.length - 1];
      const neighbors = [];

      for (const d of dirs) {
        const nx = cur.x + d.dx;
        const nz = cur.z + d.dz;
        if (nx > 0 && nx < w - 1 && nz > 0 && nz < h - 1 && !visited[nz][nx]) {
          neighbors.push({ x: nx, z: nz, wallX: cur.x + d.dx / 2, wallZ: cur.z + d.dz / 2 });
        }
      }

      if (neighbors.length > 0) {
        // Pick random neighbor
        const chosen = neighbors[Math.floor(Math.random() * neighbors.length)];
        this.maze[chosen.wallZ][chosen.wallX] = 0;
        this.maze[chosen.z][chosen.x] = 0;
        visited[chosen.z][chosen.x] = true;
        stack.push({ x: chosen.x, z: chosen.z });
      } else {
        stack.pop();
      }
    }

    // Loop Carving: punch a few extra holes to allow alternate pathways
    const loops = Math.floor(Math.min(w, h) / 3);
    for (let i = 0; i < loops; i++) {
      const rx = 1 + 2 * Math.floor(Math.random() * ((w - 3) / 2));
      const rz = 1 + 2 * Math.floor(Math.random() * ((h - 3) / 2));
      const d = dirs[Math.floor(Math.random() * dirs.length)];
      const wx = rx + d.dx / 2;
      const wz = rz + d.dz / 2;
      if (wx > 0 && wx < w - 1 && wz > 0 && wz < h - 1) {
        this.maze[wz][wx] = 0;
      }
    }

    // Exit portal at opposite quadrant
    this.exitX = w - 2;
    this.exitZ = h - 2;
    this.maze[this.exitZ][this.exitX] = 0;

    // Distribute glowing orbs in corridors / dead ends
    const numOrbs = 3 + (this.level - 1) * 2;
    this.orbs = [];
    const candidates = [];

    for (let z = 1; z < h - 1; z++) {
      for (let x = 1; x < w - 1; x++) {
        if (this.maze[z][x] === 0 && !(x === 1 && z === 1) && !(x === this.exitX && z === this.exitZ)) {
          // Calculate distance from start
          const d = Math.abs(x - 1) + Math.abs(z - 1);
          candidates.push({ x, z, dist: d });
        }
      }
    }

    // Shuffle and pick spaced candidates
    candidates.sort(() => Math.random() - 0.5);
    for (let i = 0; i < Math.min(numOrbs, candidates.length); i++) {
      this.orbs.push({
        gridX: candidates[i].x,
        gridZ: candidates[i].z,
        x: (candidates[i].x + 0.5) * CELL_SIZE,
        z: (candidates[i].z + 0.5) * CELL_SIZE,
        taken: false
      });
    }
  }

  // ------------------------------------------------------------------------
  // Construct 3D Scene Geometry for Current Level
  // ------------------------------------------------------------------------
  rebuild3DWorld() {
    // Clear old meshes
    for (const m of this.mazeMeshes) this.scene.remove(m);
    this.mazeMeshes = [];
    for (const o of this.orbMeshes) this.scene.remove(o.group);
    this.orbMeshes = [];
    if (this.portalGroup) this.scene.remove(this.portalGroup);

    // Floor and Ceiling materials
    const floorMat = new THREE.MeshStandardMaterial({
      map: this.floorTexture,
      roughness: 0.8,
      metalness: 0.1
    });
    this.floorTexture.repeat.set(this.W, this.H);

    // Large floor quad
    const floorGeo = new THREE.PlaneGeometry(this.W * CELL_SIZE, this.H * CELL_SIZE);
    const floorMesh = new THREE.Mesh(floorGeo, floorMat);
    floorMesh.rotation.x = -Math.PI / 2;
    floorMesh.position.set((this.W * CELL_SIZE) / 2, 0, (this.H * CELL_SIZE) / 2);
    this.scene.add(floorMesh);
    this.mazeMeshes.push(floorMesh);

    // Ceiling quad
    const ceilMat = new THREE.MeshStandardMaterial({
      map: this.floorTexture,
      roughness: 0.9,
      color: 0x222630
    });
    const ceilMesh = new THREE.Mesh(floorGeo, ceilMat);
    ceilMesh.rotation.x = Math.PI / 2;
    ceilMesh.position.set((this.W * CELL_SIZE) / 2, WALL_HEIGHT, (this.H * CELL_SIZE) / 2);
    this.scene.add(ceilMesh);
    this.mazeMeshes.push(ceilMesh);

    // Merged Wall Instancing / Box Geometries
    const wallGeo = new THREE.BoxGeometry(CELL_SIZE, WALL_HEIGHT, CELL_SIZE);
    const wallMat = new THREE.MeshStandardMaterial({
      map: this.wallTexture,
      roughness: 0.85,
      metalness: 0.15
    });

    let wallCount = 0;
    for (let z = 0; z < this.H; z++) {
      for (let x = 0; x < this.W; x++) {
        if (this.maze[z][x] === 1) wallCount++;
      }
    }

    const instancedWalls = new THREE.InstancedMesh(wallGeo, wallMat, wallCount);
    const dummy = new THREE.Object3D();
    let idx = 0;

    for (let z = 0; z < this.H; z++) {
      for (let x = 0; x < this.W; x++) {
        if (this.maze[z][x] === 1) {
          dummy.position.set((x + 0.5) * CELL_SIZE, WALL_HEIGHT / 2, (z + 0.5) * CELL_SIZE);
          dummy.updateMatrix();
          instancedWalls.setMatrixAt(idx++, dummy.matrix);
        }
      }
    }
    instancedWalls.instanceMatrix.needsUpdate = true;
    this.scene.add(instancedWalls);
    this.mazeMeshes.push(instancedWalls);

    // Build Floating Glowing Orbs
    for (let i = 0; i < this.orbs.length; i++) {
      const orb = this.orbs[i];
      const group = new THREE.Group();
      group.position.set(orb.x, 1.0, orb.z);

      // Core Golden Sphere
      const sphereGeo = new THREE.SphereGeometry(0.22, 24, 24);
      const sphereMat = new THREE.MeshStandardMaterial({
        color: 0xffd154,
        emissive: 0xf59e0b,
        emissiveIntensity: 0.8,
        roughness: 0.2,
        metalness: 0.8
      });
      const sphere = new THREE.Mesh(sphereGeo, sphereMat);
      group.add(sphere);

      // Concentric Torus Orbital Ring
      const torusGeo = new THREE.TorusGeometry(0.38, 0.035, 12, 32);
      const torusMat = new THREE.MeshStandardMaterial({
        color: 0xfbbf24,
        emissive: 0xd97706,
        emissiveIntensity: 0.6
      });
      const torus = new THREE.Mesh(torusGeo, torusMat);
      torus.rotation.x = Math.PI / 3;
      group.add(torus);

      // Light glow
      const orbLight = new THREE.PointLight(0xf59e0b, 1.4, 5.0, 1.5);
      group.add(orbLight);

      this.scene.add(group);
      this.orbMeshes.push({ group, sphere, torus, light: orbLight, orbIndex: i });
    }

    // Build Exit Portal
    this.buildExitPortal();
  }

  buildExitPortal() {
    this.portalGroup = new THREE.Group();
    const px = (this.exitX + 0.5) * CELL_SIZE;
    const pz = (this.exitZ + 0.5) * CELL_SIZE;
    this.portalGroup.position.set(px, 1.1, pz);

    // Inner glowing ring
    const ringGeo1 = new THREE.TorusGeometry(0.72, 0.08, 16, 40);
    const ringMat = new THREE.MeshStandardMaterial({
      color: 0xef4444,
      emissive: 0xef4444,
      emissiveIntensity: 0.7
    });
    this.portalRing1 = new THREE.Mesh(ringGeo1, ringMat);
    this.portalGroup.add(this.portalRing1);

    // Outer counter-rotating ring
    const ringGeo2 = new THREE.TorusGeometry(0.9, 0.05, 12, 36);
    this.portalRing2 = new THREE.Mesh(ringGeo2, ringMat.clone());
    this.portalRing2.rotation.y = Math.PI / 2;
    this.portalGroup.add(this.portalRing2);

    // Pulsing Core
    const coreGeo = new THREE.SphereGeometry(0.35, 24, 24);
    const coreMat = new THREE.MeshStandardMaterial({
      color: 0xffffff,
      emissive: 0xef4444,
      emissiveIntensity: 1.0,
      transparent: true,
      opacity: 0.85
    });
    this.portalCore = new THREE.Mesh(coreGeo, coreMat);
    this.portalGroup.add(this.portalCore);

    // Exit PointLight
    this.portalLight = new THREE.PointLight(0xef4444, 2.5, 8.0, 1.2);
    this.portalGroup.add(this.portalLight);

    // Swirling portal vortex particles
    const particleCount = 80;
    const pGeo = new THREE.BufferGeometry();
    const pPos = new Float32Array(particleCount * 3);
    for (let i = 0; i < particleCount; i++) {
      const angle = Math.random() * Math.PI * 2;
      const radius = 0.2 + Math.random() * 0.65;
      pPos[i * 3] = Math.cos(angle) * radius;
      pPos[i * 3 + 1] = (Math.random() - 0.5) * 0.4;
      pPos[i * 3 + 2] = Math.sin(angle) * radius;
    }
    pGeo.setAttribute('position', new THREE.BufferAttribute(pPos, 3));
    const pMat = new THREE.PointsMaterial({
      color: 0xffffff,
      size: 0.04,
      transparent: true,
      opacity: 0.7
    });
    this.portalParticles = new THREE.Points(pGeo, pMat);
    this.portalGroup.add(this.portalParticles);

    this.scene.add(this.portalGroup);
  }

  // ------------------------------------------------------------------------
  // Game Loop & Level Progression
  // ------------------------------------------------------------------------
  startLevel(lvl) {
    this.level = lvl;
    this.orbsTaken = 0;
    const gridDim = 13 + (lvl - 1) * 4; // 13, 17, 21, 25...
    this.buildMaze(gridDim, gridDim);
    this.rebuild3DWorld();

    // Reset player position at start cell
    this.px = (1 + 0.5) * CELL_SIZE;
    this.pz = (1 + 0.5) * CELL_SIZE;
    this.yaw = 0;
    this.pitch = 0;
    this.bobPhase = 0;

    // Time allowance based on maze size
    this.totalTime = 40 + (lvl - 1) * 18;
    this.timeLeft = this.totalTime;

    this.markSeen();
    this.updateHUD();
    this.renderMinimap();

    this.showToast(`LEVEL ${lvl}: Collect all orbs to unlock portal!`);
  }

  isWall(x, z) {
    const gx = Math.floor(x / CELL_SIZE);
    const gz = Math.floor(z / CELL_SIZE);
    if (gx < 0 || gz < 0 || gx >= this.W || gz >= this.H) return true;
    return this.maze[gz][gx] === 1;
  }

  blocked(x, z) {
    const r = PLAYER_RADIUS;
    return (
      this.isWall(x - r, z - r) ||
      this.isWall(x + r, z - r) ||
      this.isWall(x - r, z + r) ||
      this.isWall(x + r, z + r)
    );
  }

  markSeen() {
    const gx = Math.floor(this.px / CELL_SIZE);
    const gz = Math.floor(this.pz / CELL_SIZE);
    const r = 2; // sight radius
    for (let dz = -r; dz <= r; dz++) {
      for (let dx = -r; dx <= r; dx++) {
        const nx = gx + dx;
        const nz = gz + dz;
        if (nx >= 0 && nx < this.W && nz >= 0 && nz < this.H) {
          this.seen[nz][nx] = true;
        }
      }
    }
  }

  allOrbsCollected() {
    return this.orbsTaken === this.orbs.length;
  }

  // ------------------------------------------------------------------------
  // Physics & Input Update
  // ------------------------------------------------------------------------
  update(dt) {
    if (this.state !== 'PLAYING') return;

    // Timer countdown
    this.timeLeft -= dt;
    if (this.timeLeft <= 0) {
      this.timeLeft = 0;
      this.onGameOver();
      return;
    }

    // Audio tick countdown warning
    if (this.timeLeft <= 10 && Math.floor(this.timeLeft) !== Math.floor(this.timeLeft + dt)) {
      window.sounds.playTick();
    }

    // Keyboard Movement Vectors
    let vx = 0, vz = 0;
    if (this.keys['KeyW'] || this.keys['ArrowUp']) {
      vx += Math.cos(this.yaw);
      vz += Math.sin(this.yaw);
    }
    if (this.keys['KeyS'] || this.keys['ArrowDown']) {
      vx -= Math.cos(this.yaw);
      vz -= Math.sin(this.yaw);
    }
    if (this.keys['KeyA'] || this.keys['KeyQ']) {
      vx += Math.cos(this.yaw - Math.PI / 2);
      vz += Math.sin(this.yaw - Math.PI / 2);
    }
    if (this.keys['KeyD'] || this.keys['KeyE']) {
      vx += Math.cos(this.yaw + Math.PI / 2);
      vz += Math.sin(this.yaw + Math.PI / 2);
    }

    // Keyboard Arrow Turn (Left/Right)
    if (this.keys['ArrowLeft']) this.yaw -= TURN_SPEED * dt;
    if (this.keys['ArrowRight']) this.yaw += TURN_SPEED * dt;

    const len = Math.sqrt(vx * vx + vz * vz);
    this.isMoving = len > 0.01;

    if (this.isMoving) {
      vx = (vx / len) * MOVE_SPEED * dt;
      vz = (vz / len) * MOVE_SPEED * dt;

      // Axis-separated collision test allows smooth sliding along walls
      const nx = this.px + vx;
      const nz = this.pz + vz;
      if (!this.blocked(nx, this.pz)) this.px = nx;
      if (!this.blocked(this.px, nz)) this.pz = nz;

      this.bobPhase += dt * 10;
      this.stepTimer += dt;
      if (this.stepTimer > 0.35) {
        window.sounds.playFootstep();
        this.stepTimer = 0;
      }
    }

    this.markSeen();

    // Check Orb Pickups
    for (let i = 0; i < this.orbs.length; i++) {
      const orb = this.orbs[i];
      if (orb.taken) continue;

      const dx = orb.x - this.px;
      const dz = orb.z - this.pz;
      if (dx * dx + dz * dz < 0.7 * 0.7) {
        orb.taken = true;
        this.orbsTaken++;
        this.score += 100;

        // Hide mesh
        this.orbMeshes[i].group.visible = false;
        window.sounds.playOrbPickup();
        this.showScorePopup('+100');

        if (this.allOrbsCollected()) {
          this.showToast('All orbs collected! The exit portal is OPEN!', 'success');
          window.sounds.playPortalUnlock();
          this.activatePortalGlow();
        } else {
          this.showToast(`Orb collected! ${this.orbs.length - this.orbsTaken} remaining`);
        }
      }
    }

    // Check Exit Portal
    const portalCenterX = (this.exitX + 0.5) * CELL_SIZE;
    const portalCenterZ = (this.exitZ + 0.5) * CELL_SIZE;
    const ex = portalCenterX - this.px;
    const ez = portalCenterZ - this.pz;

    if (ex * ex + ez * ez < 0.85 * 0.85) {
      if (this.allOrbsCollected()) {
        this.onLevelComplete();
      } else {
        this.showToast('Portal is locked - collect all glowing orbs first!', 'warning');
      }
    }

    // Update Camera position & head-bob oscillation
    const bob = this.isMoving ? Math.sin(this.bobPhase) * 0.05 : 0;
    this.camera.position.set(this.px, EYE_HEIGHT + bob, this.pz);
    this.camera.rotation.y = -this.yaw - Math.PI / 2;
    this.camera.rotation.x = this.pitch;

    // Update Torch Light position with subtle flame flicker
    this.torchFlickerTime += dt * 8;
    const flicker = 0.08 * (Math.sin(this.torchFlickerTime) + Math.cos(this.torchFlickerTime * 2.3));
    this.torchLight.position.set(this.px, EYE_HEIGHT + bob, this.pz);
    this.torchLight.intensity = 2.0 + flicker;

    // Animate Orbs (Spin & Levitate)
    const t = this.clock.getElapsedTime();
    for (let i = 0; i < this.orbMeshes.length; i++) {
      const om = this.orbMeshes[i];
      if (!this.orbs[om.orbIndex].taken) {
        om.group.position.y = 1.0 + Math.sin(t * 3.0 + i) * 0.12;
        om.torus.rotation.z = t * 2.0;
        om.sphere.rotation.y = t * 1.5;
      }
    }

    // Animate Exit Portal Rings & Core
    if (this.portalGroup) {
      this.portalRing1.rotation.z += dt * 1.4;
      this.portalRing2.rotation.x += dt * 1.0;
      const coreScale = 1.0 + 0.12 * Math.sin(t * 4.5);
      this.portalCore.scale.set(coreScale, coreScale, coreScale);
      if (this.portalParticles) this.portalParticles.rotation.y += dt * 0.8;
    }

    this.updateHUD();
    this.renderMinimap();
  }

  activatePortalGlow() {
    if (!this.portalLight) return;
    const emerald = new THREE.Color(0x10b981);
    this.portalLight.color = emerald;
    this.portalRing1.material.color = emerald;
    this.portalRing1.material.emissive = emerald;
    this.portalRing2.material.color = emerald;
    this.portalRing2.material.emissive = emerald;
    this.portalCore.material.emissive = emerald;
    this.portalLight.intensity = 4.0;
  }

  // ------------------------------------------------------------------------
  // State Handlers & Modals
  // ------------------------------------------------------------------------
  onLevelComplete() {
    this.state = 'LEVEL_DONE';
    window.sounds.playLevelComplete();
    const timeBonus = Math.floor(this.timeLeft) * 10;
    this.score += timeBonus;

    this.dom.doneBonusText.textContent = `+${timeBonus}`;
    this.dom.doneScoreText.textContent = this.score;

    this.dom.levelDoneModal.classList.add('active');
    document.exitPointerLock?.();
  }

  onGameOver() {
    this.state = 'GAME_OVER';
    window.sounds.playGameOver();

    this.dom.overLevelText.textContent = this.level;
    this.dom.overScoreText.textContent = this.score;

    this.dom.gameOverModal.classList.add('active');
    document.exitPointerLock?.();
  }

  // ------------------------------------------------------------------------
  // HUD, UI & Minimap
  // ------------------------------------------------------------------------
  updateHUD() {
    this.dom.levelText.textContent = this.level;
    this.dom.orbsText.textContent = `${this.orbsTaken}/${this.orbs.length}`;
    this.dom.scoreText.textContent = this.score;

    const sec = Math.max(0, Math.ceil(this.timeLeft));
    this.dom.timerText.textContent = `${sec}s`;

    const pct = Math.max(0, Math.min(100, (this.timeLeft / this.totalTime) * 100));
    this.dom.timerBar.style.width = `${pct}%`;

    if (this.timeLeft <= 15) {
      this.dom.timerBar.classList.add('critical');
    } else {
      this.dom.timerBar.classList.remove('critical');
    }
  }

  renderMinimap() {
    if (!this.showMinimap) return;
    const ctx = this.minimapCtx;
    const S = this.dom.minimapCanvas.width;
    ctx.clearRect(0, 0, S, S);

    const cellW = S / this.W;
    const cellH = S / this.H;

    // Draw Explored Maze Grid
    for (let z = 0; z < this.H; z++) {
      for (let x = 0; x < this.W; x++) {
        if (!this.seen[z][x]) {
          // Fog of war
          ctx.fillStyle = '#060910';
        } else if (this.maze[z][x] === 1) {
          // Explored Wall
          ctx.fillStyle = '#334155';
        } else {
          // Explored Passage
          ctx.fillStyle = '#0f172a';
        }
        ctx.fillRect(x * cellW, z * cellH, cellW + 0.5, cellH + 0.5);
      }
    }

    // Draw Orbs on Minimap if discovered
    for (const orb of this.orbs) {
      if (orb.taken) continue;
      if (this.seen[orb.gridZ][orb.gridX]) {
        ctx.fillStyle = '#f59e0b';
        ctx.shadowColor = '#f59e0b';
        ctx.shadowBlur = 6;
        ctx.beginPath();
        ctx.arc((orb.gridX + 0.5) * cellW, (orb.gridZ + 0.5) * cellH, cellW * 0.35, 0, Math.PI * 2);
        ctx.fill();
        ctx.shadowBlur = 0;
      }
    }

    // Draw Exit Portal if discovered
    if (this.seen[this.exitZ][this.exitX]) {
      const open = this.allOrbsCollected();
      ctx.fillStyle = open ? '#10b981' : '#ef4444';
      ctx.shadowColor = ctx.fillStyle;
      ctx.shadowBlur = 8;
      ctx.beginPath();
      ctx.arc((this.exitX + 0.5) * cellW, (this.exitZ + 0.5) * cellH, cellW * 0.45, 0, Math.PI * 2);
      ctx.fill();
      ctx.shadowBlur = 0;
    }

    // Draw Player Indicator with direction needle
    const playerGx = this.px / CELL_SIZE;
    const playerGz = this.pz / CELL_SIZE;
    const pxCanvas = playerGx * cellW;
    const pzCanvas = playerGz * cellH;

    ctx.fillStyle = '#38bdf8';
    ctx.shadowColor = '#38bdf8';
    ctx.shadowBlur = 8;
    ctx.beginPath();
    ctx.arc(pxCanvas, pzCanvas, cellW * 0.4, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;

    // Heading needle
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(pxCanvas, pzCanvas);
    ctx.lineTo(
      pxCanvas + Math.cos(this.yaw) * cellW * 0.8,
      pzCanvas + Math.sin(this.yaw) * cellH * 0.8
    );
    ctx.stroke();
  }

  showToast(msg, type = '') {
    this.dom.toastText.textContent = msg;
    this.dom.statusToast.className = 'show ' + type;
    if (this.toastTimeout) clearTimeout(this.toastTimeout);
    this.toastTimeout = setTimeout(() => {
      this.dom.statusToast.className = '';
    }, 2800);
  }

  showScorePopup(text) {
    const popup = document.createElement('div');
    popup.className = 'score-popup';
    popup.textContent = text;
    popup.style.left = '50%';
    popup.style.top = '45%';
    document.body.appendChild(popup);
    setTimeout(() => popup.remove(), 1000);
  }

  // ------------------------------------------------------------------------
  // Event Bindings
  // ------------------------------------------------------------------------
  bindEvents() {
    window.addEventListener('resize', () => {
      const w = window.innerWidth;
      const h = window.innerHeight;
      this.camera.aspect = w / h;
      this.camera.updateProjectionMatrix();
      this.renderer.setSize(w, h);
    });

    // Keyboard handlers
    window.addEventListener('keydown', (e) => {
      this.keys[e.code] = true;

      if (e.code === 'KeyM') {
        this.showMinimap = !this.showMinimap;
        this.dom.minimapCard.classList.toggle('minimized', !this.showMinimap);
      }

      if (e.code === 'KeyP') {
        if (this.state === 'PLAYING') {
          this.state = 'PAUSED';
          this.dom.pauseModal.classList.add('active');
          document.exitPointerLock?.();
        } else if (this.state === 'PAUSED') {
          this.state = 'PLAYING';
          this.dom.pauseModal.classList.remove('active');
          this.dom.container.requestPointerLock?.();
        }
      }

      if (e.code === 'Enter') {
        if (this.state === 'MENU') this.startGame();
        else if (this.state === 'LEVEL_DONE') this.nextLevel();
        else if (this.state === 'GAME_OVER') this.restartGame();
      }

      if (e.code === 'KeyR' && (this.state === 'PLAYING' || this.state === 'PAUSED')) {
        this.score = this.levelStartScore;
        this.startLevel(this.level);
        this.state = 'PLAYING';
        this.dom.pauseModal.classList.remove('active');
      }
    });

    window.addEventListener('keyup', (e) => {
      this.keys[e.code] = false;
    });

    // Pointer Lock & Mouse Look
    this.dom.container.addEventListener('click', () => {
      window.sounds.ensureContext();
      if (this.state === 'PLAYING' && !this.isPointerLocked) {
        this.dom.container.requestPointerLock?.();
      }
    });

    document.addEventListener('pointerlockchange', () => {
      this.isPointerLocked = document.pointerLockElement === this.dom.container;
      this.dom.crosshair.classList.toggle('active', this.isPointerLocked);
      this.dom.clickPrompt.style.display = this.isPointerLocked ? 'none' : 'block';
    });

    document.addEventListener('mousemove', (e) => {
      if (!this.isPointerLocked || this.state !== 'PLAYING') return;
      const sensitivity = 0.0022;
      this.yaw += e.movementX * sensitivity;
      this.pitch -= e.movementY * sensitivity;
      this.pitch = Math.max(-Math.PI / 3, Math.min(Math.PI / 3, this.pitch));
    });

    // UI Buttons
    this.dom.btnStart.addEventListener('click', () => this.startGame());
    this.dom.btnResume.addEventListener('click', () => {
      this.state = 'PLAYING';
      this.dom.pauseModal.classList.remove('active');
      this.dom.container.requestPointerLock?.();
    });
    this.dom.btnRestartPause.addEventListener('click', () => {
      this.score = this.levelStartScore;
      this.startLevel(this.level);
      this.state = 'PLAYING';
      this.dom.pauseModal.classList.remove('active');
      this.dom.container.requestPointerLock?.();
    });
    this.dom.btnNextLevel.addEventListener('click', () => this.nextLevel());
    this.dom.btnRetry.addEventListener('click', () => this.restartGame());

    // Audio Mute Toggle
    this.dom.audioBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      window.sounds.ensureContext();
      const enabled = window.sounds.toggle();
      this.dom.audioBtn.style.opacity = enabled ? '1' : '0.4';
      this.showToast(enabled ? 'Sound FX Enabled' : 'Sound FX Muted');
    });
  }

  startGame() {
    window.sounds.ensureContext();
    this.dom.menuModal.classList.remove('active');
    this.state = 'PLAYING';
    this.levelStartScore = this.score;
    this.dom.container.requestPointerLock?.();
  }

  nextLevel() {
    this.dom.levelDoneModal.classList.remove('active');
    this.levelStartScore = this.score;
    this.startLevel(this.level + 1);
    this.state = 'PLAYING';
    this.dom.container.requestPointerLock?.();
  }

  restartGame() {
    this.dom.gameOverModal.classList.remove('active');
    this.score = 0;
    this.levelStartScore = 0;
    this.startLevel(1);
    this.state = 'PLAYING';
    this.dom.container.requestPointerLock?.();
  }

  // ------------------------------------------------------------------------
  // Animation Frame Loop
  // ------------------------------------------------------------------------
  animate() {
    requestAnimationFrame(() => this.animate());
    const dt = Math.min(this.clock.getDelta(), 0.1);
    this.update(dt);
    this.renderer.render(this.scene, this.camera);
  }
}

// Instantiate game once window loads
window.addEventListener('DOMContentLoaded', () => {
  window.game = new MazeEscapeGame();
});
