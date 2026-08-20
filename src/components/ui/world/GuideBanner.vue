<template>
  <transition name="guide-banner">
    <div
      v-if="visible"
      class="guide-banner"
      role="status"
      aria-live="polite"
    >
      <span class="guide-banner__speaker">Aldwyn the Guide</span>
      <p class="guide-banner__text">{{ text }}</p>
    </div>
  </transition>
</template>

<script>
const DEFAULT_DURATION_MS = 9000;

export default {
  name: 'GuideBanner',
  props: {
    text: {
      type: String,
      default: '',
    },
    durationMs: {
      type: Number,
      default: DEFAULT_DURATION_MS,
    },
  },
  data() {
    return {
      visible: false,
      timer: null,
    };
  },
  watch: {
    text(value) {
      const next = value && String(value).trim();
      if (!next) {
        this.hide();
        return;
      }
      this.show(next);
    },
  },
  beforeUnmount() {
    this.clearTimer();
  },
  methods: {
    show(value) {
      this.clearTimer();
      this.visible = true;
      this.timer = setTimeout(() => {
        this.visible = false;
        this.timer = null;
      }, Math.max(1500, Number(this.durationMs) || DEFAULT_DURATION_MS));
    },
    hide() {
      this.clearTimer();
      this.visible = false;
    },
    clearTimer() {
      if (this.timer) {
        clearTimeout(this.timer);
        this.timer = null;
      }
    },
  },
};
</script>

<style lang="scss" scoped>
@use '@/assets/scss/abstracts/tokens' as *;

.guide-banner {
  display: flex;
  flex-direction: column;
  gap: 3px;
  max-width: min(560px, calc(100% - 24px));
  box-sizing: border-box;
  padding: 10px 14px;
  border: 1px solid rgba(224, 180, 92, 0.5);
  border-left: 3px solid var(--color-accent-strong, #e0b45c);
  background:
    linear-gradient(90deg, rgba(84, 63, 26, 0.5), transparent 72%),
    rgba(4, 5, 7, 0.9);
  color: #efe6cf;
  box-shadow: 0 10px 26px rgba(0, 0, 0, 0.55);
  text-align: left;
}

.guide-banner__speaker {
  font-family: 'GameFont', sans-serif;
  font-size: 0.62rem;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--color-accent-strong, #e7c570);
}

.guide-banner__text {
  margin: 0;
  font-family: 'ChatFont', 'GameFont', sans-serif;
  font-size: 0.86rem;
  line-height: 1.45;
}

.guide-banner-enter-active,
.guide-banner-leave-active {
  transition: opacity 220ms ease-out, transform 220ms ease-out;
}

.guide-banner-enter-from,
.guide-banner-leave-to {
  opacity: 0;
  transform: translateY(-8px);
}
</style>
