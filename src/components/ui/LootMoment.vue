<template>
  <div class="loot-moment">
    <section
      class="loot-moment__card"
      role="status"
      aria-live="polite"
      aria-label="First find"
    >
      <p class="loot-moment__eyebrow">First find</p>
      <h2 class="loot-moment__name">{{ name }}</h2>
      <p
        v-if="examine"
        class="loot-moment__examine"
      >
        {{ examine }}
      </p>
      <p class="loot-moment__comparison">{{ comparison }}</p>
      <p class="loot-moment__hint">Open your inventory (I) to equip it.</p>
    </section>
  </div>
</template>

<script>
import { onBeforeUnmount, onMounted, ref } from 'vue';
import bus from '../../core/utilities/bus.js';

const AUTO_DISMISS_MS = 7000;

/**
 * TASK-0042 first-loot moment: a compact inspect toast for the session's one
 * curated first-delve drop. Mounted through the existing open:screen seam
 * (GameCanvas renders the passed component with :game and :data) and dismisses
 * itself — the pane close button remains as the manual affordance.
 */
export default {
  name: 'LootMoment',
  props: {
    game: {
      type: Object,
      required: true,
    },
    data: {
      type: Object,
      default: () => ({}),
    },
  },
  setup(props) {
    const name = ref(props.data && props.data.name ? props.data.name : 'Unknown find');
    const examine = ref(props.data && props.data.examine ? props.data.examine : '');
    const comparison = ref(props.data && props.data.comparison ? props.data.comparison : '');

    let dismissTimer = null;
    const dismiss = () => bus.$emit('screen:close');

    onMounted(() => {
      dismissTimer = setTimeout(dismiss, AUTO_DISMISS_MS);
    });

    onBeforeUnmount(() => {
      if (dismissTimer) {
        clearTimeout(dismissTimer);
        dismissTimer = null;
      }
    });

    return {
      name,
      examine,
      comparison,
    };
  },
};
</script>

<style scoped lang="scss">
/* The host pane stretches its direct child (.pane div); the toast instead
   floats compact near the top so the drop stays visible behind it. */
.loot-moment {
  display: flex;
  width: 100%;
  height: 100%;
  align-items: flex-start;
  justify-content: center;
  padding-top: clamp(24px, 12vh, 120px);
  pointer-events: none;
}

.loot-moment__card {
  width: min(440px, 90%);
  padding: 18px 22px 16px;
  border: 1px solid rgba(209, 168, 92, 0.65);
  outline: 1px solid rgba(38, 28, 18, 0.95);
  outline-offset: -4px;
  background:
    linear-gradient(135deg, rgba(60, 68, 44, 0.35), transparent 45%),
    linear-gradient(180deg, rgba(20, 18, 16, 0.97), rgba(9, 10, 11, 0.97));
  box-shadow: 0 18px 60px rgba(0, 0, 0, 0.7), inset 0 0 32px rgba(218, 167, 77, 0.06);
  color: #e9dfc6;
  font-family: Georgia, serif;
  text-align: center;
  pointer-events: auto;
}

.loot-moment__eyebrow {
  margin: 0 0 8px;
  color: #d4a959;
  font: 0.66rem 'GameFont', sans-serif;
  letter-spacing: 0.18em;
  text-transform: uppercase;
}

.loot-moment__name {
  margin: 0;
  color: #f1d58d;
  font: 1.3rem 'GameFont', Georgia, serif;
  letter-spacing: 0.05em;
  text-shadow: 0 2px 10px rgba(0, 0, 0, 0.8);
}

.loot-moment__examine {
  margin: 10px 0 0;
  color: rgba(232, 222, 198, 0.78);
  font-size: 0.82rem;
  font-style: italic;
  line-height: 1.5;
}

.loot-moment__comparison {
  margin: 12px 0 0;
  color: #a8d5a2;
  font: 0.74rem 'GameFont', sans-serif;
  letter-spacing: 0.04em;
  line-height: 1.5;
}

.loot-moment__hint {
  margin: 12px 0 0;
  color: rgba(186, 177, 155, 0.66);
  font-size: 0.72rem;
}
</style>
