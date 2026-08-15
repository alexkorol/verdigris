<template>
  <button
    type="button"
    class="hud-orb"
    :class="variant"
    :aria-label="`${label}: ${displayValue}`"
    :title="displayValue"
    @click="$emit('activate')"
  >
    <canvas
      ref="canvas"
      class="hud-orb__canvas"
      :width="canvasSize"
      :height="canvasSize"
    />
    <div class="hud-orb__readout" aria-hidden="true">
      <strong class="hud-orb__value">{{ displayCurrent }}</strong>
      <span v-if="showMeter" class="hud-orb__maximum">/ {{ displayMaximum }}</span>
    </div>
  </button>
</template>

<script>
import WizardOrbRenderer from '@/core/hud/wizard-orb-renderer.js';

export default {
  name: 'HudOrb',
  emits: ['activate'],
  props: {
    variant: {
      type: String,
      default: 'neutral',
    },
    label: {
      type: String,
      required: true,
    },
    current: {
      type: Number,
      default: 0,
    },
    max: {
      type: Number,
      default: 0,
    },
  },
  data() {
    return {
      canvasSize: 256,
      renderer: null,
    };
  },
  computed: {
    showMeter() {
      return Number.isFinite(this.max) && this.max > 0;
    },
    displayValue() {
      if (!this.showMeter) {
        return String(this.displayCurrent);
      }
      return `${this.displayCurrent} / ${this.displayMaximum}`;
    },
    displayCurrent() {
      return Math.max(0, Math.round(this.current));
    },
    displayMaximum() {
      return Math.max(0, Math.round(this.max));
    },
    fillPercent() {
      if (!this.max || this.max <= 0) return 1;
      return Math.max(0, Math.min(1, this.current / this.max));
    },
  },
  watch: {
    fillPercent(value) {
      if (this.renderer) {
        this.renderer.setFill(value);
      }
    },
    variant(value) {
      if (this.renderer) {
        this.renderer.setVariant(value);
      }
    },
  },
  mounted() {
    this.renderer = new WizardOrbRenderer(this.$refs.canvas, {
      variant: this.variant,
      fill: this.fillPercent,
    });
  },
  beforeUnmount() {
    if (this.renderer) {
      this.renderer.destroy();
      this.renderer = null;
    }
  },
};
</script>

<style lang="scss" scoped>
.hud-orb {
  --orb-size: var(--hud-orb-size, clamp(176px, 16vw, 224px));
  --orb-accent: #d04545;
  --orb-accent-soft: rgba(208, 69, 69, 0.42);

  position: relative;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: var(--orb-size);
  height: var(--orb-size);

  /* Soft, centred contact shadow — the previous heavy offset shadow read as
   * an unsightly dark edge hugging the orb. */
  filter: drop-shadow(0 4px 10px rgba(0, 0, 0, 0.4));
  padding: 0;
  border: 0;
  background: transparent;
  color: inherit;
  font: inherit;
  cursor: pointer;
}

.hud-orb:focus-visible {
  outline: 2px solid rgba(238, 205, 129, 0.9);
  outline-offset: -12px;
}

.hud-orb.mp {
  --orb-accent: #5b92ef;
  --orb-accent-soft: rgba(91, 146, 239, 0.42);
}

.hud-orb__canvas {
  position: absolute;
  inset: 0;
  display: block;
  width: 100%;
  height: 100%;
  background: transparent;
  pointer-events: none;
  filter: drop-shadow(0 0 16px var(--orb-accent-soft));
}

.hud-orb__readout {
  position: absolute;
  bottom: 9%;
  left: 50%;
  z-index: 2;
  display: flex;
  align-items: baseline;
  justify-content: center;
  gap: 3px;
  min-width: 48px;
  padding: 2px 7px;
  border: 0;
  border-radius: 999px;
  background: rgba(4, 5, 7, 0.48);
  box-shadow: 0 1px 5px rgba(0, 0, 0, 0.5);
  color: rgba(255, 244, 220, 0.88);
  font-family: 'GameFont', sans-serif;
  text-shadow: 0 1px 3px #000;
  transform: translateX(-50%);
  white-space: nowrap;
}

.hud-orb__value {
  font-size: clamp(0.54rem, 0.76vw, 0.68rem);
  font-weight: 400;
}

.hud-orb__maximum {
  color: rgba(227, 216, 192, 0.48);
  font-size: clamp(0.42rem, 0.58vw, 0.52rem);
}
</style>
