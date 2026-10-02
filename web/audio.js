// ==========================================================================
// Maze Escape 3D - Procedural Web Audio Sound Synthesizer
// ==========================================================================

class SoundManager {
  constructor() {
    this.ctx = null;
    this.enabled = true;
    this.droneGain = null;
    this.portalGain = null;
    this.portalOsc = null;
    this.droneOsc = null;
    this.initialized = false;
  }

  init() {
    if (this.initialized) return;
    try {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      this.ctx = new AudioCtx();
      this.initialized = true;
      this.startAmbientDrone();
    } catch (e) {
      console.warn("Web Audio API not supported", e);
    }
  }

  ensureContext() {
    if (!this.initialized) {
      this.init();
    } else if (this.ctx && this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  toggle() {
    this.enabled = !this.enabled;
    if (this.droneGain && this.ctx) {
      this.droneGain.gain.setTargetAtTime(this.enabled ? 0.08 : 0, this.ctx.currentTime, 0.1);
    }
    return this.enabled;
  }

  startAmbientDrone() {
    if (!this.ctx || !this.enabled) return;

    // Sub-bass atmospheric labyrinth hum
    const osc1 = this.ctx.createOscillator();
    const osc2 = this.ctx.createOscillator();
    const filter = this.ctx.createBiquadFilter();
    this.droneGain = this.ctx.createGain();

    osc1.type = 'triangle';
    osc1.frequency.setValueAtTime(55, this.ctx.currentTime); // A1 note
    osc2.type = 'sine';
    osc2.frequency.setValueAtTime(55.6, this.ctx.currentTime); // subtle detune beat

    filter.type = 'lowpass';
    filter.frequency.setValueAtTime(160, this.ctx.currentTime);

    this.droneGain.gain.setValueAtTime(this.enabled ? 0.08 : 0, this.ctx.currentTime);

    osc1.connect(filter);
    osc2.connect(filter);
    filter.connect(this.droneGain);
    this.droneGain.connect(this.ctx.destination);

    osc1.start();
    osc2.start();
  }

  // Soft rhythmic stone footstep
  playFootstep() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    const filter = this.ctx.createBiquadFilter();

    osc.type = 'sine';
    osc.frequency.setValueAtTime(80 + Math.random() * 20, t);
    osc.frequency.exponentialRampToValueAtTime(30, t + 0.08);

    filter.type = 'lowpass';
    filter.frequency.setValueAtTime(250, t);

    gain.gain.setValueAtTime(0.04, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + 0.09);

    osc.connect(filter);
    filter.connect(gain);
    gain.connect(this.ctx.destination);

    osc.start(t);
    osc.stop(t + 0.1);
  }

  // Sparkling golden orb pickup chime
  playOrbPickup() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const notes = [587.33, 880.0, 1174.66, 1760.0]; // D5, A5, D6, A6 sparkle

    notes.forEach((freq, idx) => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();

      osc.type = 'sine';
      osc.frequency.setValueAtTime(freq, t + idx * 0.04);

      gain.gain.setValueAtTime(0.12, t + idx * 0.04);
      gain.gain.exponentialRampToValueAtTime(0.001, t + idx * 0.04 + 0.35);

      osc.connect(gain);
      gain.connect(this.ctx.destination);

      osc.start(t + idx * 0.04);
      osc.stop(t + idx * 0.04 + 0.4);
    });
  }

  // Glorious portal unlock fanfare chord
  playPortalUnlock() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const freqs = [329.63, 415.30, 493.88, 659.25, 987.77]; // E major chord triumph

    freqs.forEach((freq, i) => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();

      osc.type = 'triangle';
      osc.frequency.setValueAtTime(freq, t + i * 0.06);

      gain.gain.setValueAtTime(0.15, t + i * 0.06);
      gain.gain.exponentialRampToValueAtTime(0.001, t + 1.2);

      osc.connect(gain);
      gain.connect(this.ctx.destination);

      osc.start(t + i * 0.06);
      osc.stop(t + 1.3);
    });
  }

  // Portal teleportation whoosh sound
  playLevelComplete() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();

    osc.type = 'sine';
    osc.frequency.setValueAtTime(220, t);
    osc.frequency.exponentialRampToValueAtTime(880, t + 0.5);
    osc.frequency.exponentialRampToValueAtTime(1760, t + 0.8);

    gain.gain.setValueAtTime(0.2, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + 0.9);

    osc.connect(gain);
    gain.connect(this.ctx.destination);

    osc.start(t);
    osc.stop(t + 1.0);
  }

  // Game over / timer expired sting
  playGameOver() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();

    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(150, t);
    osc.frequency.exponentialRampToValueAtTime(40, t + 0.8);

    gain.gain.setValueAtTime(0.2, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + 0.9);

    osc.connect(gain);
    gain.connect(this.ctx.destination);

    osc.start(t);
    osc.stop(t + 1.0);
  }

  // Ticking urgency warning sound
  playTick() {
    if (!this.ctx || !this.enabled) return;
    this.ensureContext();

    const t = this.ctx.currentTime;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();

    osc.type = 'sine';
    osc.frequency.setValueAtTime(800, t);

    gain.gain.setValueAtTime(0.06, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + 0.05);

    osc.connect(gain);
    gain.connect(this.ctx.destination);

    osc.start(t);
    osc.stop(t + 0.06);
  }
}

window.sounds = new SoundManager();
