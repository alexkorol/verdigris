<template>
  <section
    v-if="visible"
    ref="overlayRef"
    class="death-overlay"
    role="dialog"
    aria-modal="true"
    aria-labelledby="death-overlay-title"
    @keydown.stop.prevent="handleKeydown"
  >
    <div class="death-overlay__veil" aria-hidden="true" />
    <div class="death-overlay__panel">
      <p class="death-overlay__eyebrow">The expedition ends here</p>
      <h1 id="death-overlay-title">{{ summary.permanent ? 'The Scion has fallen' : 'You have fallen' }}</h1>
      <p class="death-overlay__lede">
        {{ summary.permanent
          ? 'The mortal oath is fulfilled. This Scion will not return to the road.'
          : 'The road will receive you again. Nothing carried is taken from you.' }}
      </p>

      <div class="death-overlay__columns">
        <section class="death-overlay__card" aria-labelledby="death-loss-title">
          <h2 id="death-loss-title">What leaves this run</h2>
          <ul v-if="summary.losses?.length" class="death-overlay__list">
            <li v-for="entry in summary.losses" :key="`loss-${entry.id}-${entry.kind}`">
              <span>{{ entry.name }}</span>
              <small v-if="entry.quantity > 1">×{{ entry.quantity }}</small>
            </li>
          </ul>
          <p v-else class="death-overlay__empty">No carried value is lost.</p>
        </section>

        <section class="death-overlay__card" aria-labelledby="death-protected-title">
          <h2 id="death-protected-title">
            {{ summary.permanent ? 'Recovered to the House pool' : 'Protected on return' }}
          </h2>
          <ul
            v-if="(summary.permanent ? summary.recoveredToPool : summary.protected)?.length"
            class="death-overlay__list"
          >
            <li
              v-for="entry in (summary.permanent ? summary.recoveredToPool : summary.protected)"
              :key="`protected-${entry.id}-${entry.kind}`"
            >
              <span>{{ entry.name }}</span>
              <small v-if="entry.quantity > 1">×{{ entry.quantity }}</small>
            </li>
          </ul>
          <p v-else class="death-overlay__empty">The ledger records no carried value.</p>
        </section>
      </div>

      <p class="death-overlay__destination">
        <strong>Next:</strong> {{ summary.respawnDestination || summary.respawn?.destination || 'Return to the road' }}
        <span v-if="summary.respawn?.at && !summary.permanent">
          · The return is being prepared.
        </span>
      </p>

      <p v-if="summary.succession" class="death-overlay__oath">
        The Chronicles will keep this name. Continue to choose a living successor.
      </p>

      <button
        ref="continueButton"
        class="death-overlay__continue"
        type="button"
        @click="continueFromDeath"
      >
        {{ summary.succession ? 'Return to the Chronicles' : 'Continue' }}
      </button>
      <p class="death-overlay__hint">Press Enter or Space to continue</p>
    </div>
  </section>
</template>

<script>
import { nextTick, onBeforeUnmount, onMounted, ref } from 'vue';
import bus from '../../../core/utilities/bus.js';
import Socket from '../../../core/utilities/socket.js';

export default {
  name: 'DeathOverlay',
  props: {
    game: {
      type: Object,
      required: true,
    },
  },
  setup(props) {
    const visible = ref(false);
    const summary = ref({});
    const overlayRef = ref(null);
    const continueButton = ref(null);

    const showDeath = (incoming = {}) => {
      summary.value = incoming && typeof incoming === 'object' ? incoming : {};
      visible.value = true;
      nextTick(() => continueButton.value?.focus());
    };

    const hideDeath = () => {
      visible.value = false;
      summary.value = {};
    };

    const continueFromDeath = () => {
      const current = summary.value || {};
      if (current.succession || current.permanent) {
        const scion = current.scion || {};
        Socket.emit('player:chronicles:return', {
          houseId: scion.houseId || props.game?.player?.chronicles?.houseId || null,
          scionId: scion.id || props.game?.player?.scionId || null,
        });
      }
      hideDeath();
      if (typeof window !== 'undefined' && typeof window.focusOnGame === 'function') {
        window.focusOnGame();
      }
    };

    const handleKeydown = (event) => {
      if (event.key === 'Enter' || event.key === ' ' || event.key === 'Spacebar') {
        continueFromDeath();
      }
    };

    const captureDeathInput = (event) => {
      if (!visible.value) {
        return;
      }
      event.preventDefault();
      event.stopPropagation();
      handleKeydown(event);
    };

    onMounted(() => {
      bus.$on('player:death-summary', showDeath);
      window.addEventListener('keydown', captureDeathInput, true);
    });

    onBeforeUnmount(() => {
      bus.$off('player:death-summary', showDeath);
      window.removeEventListener('keydown', captureDeathInput, true);
    });

    return {
      visible,
      summary,
      overlayRef,
      continueButton,
      continueFromDeath,
      handleKeydown,
    };
  },
};
</script>

<style scoped lang="scss">
.death-overlay {
  position: fixed;
  inset: 0;
  z-index: 240;
  display: grid;
  place-items: center;
  padding: clamp(14px, 4vw, 48px);
  color: #e9dfc6;
  font-family: Georgia, serif;
  isolation: isolate;
  outline: none;
  pointer-events: auto;
}

.death-overlay__veil {
  position: absolute;
  inset: 0;
  z-index: -1;
  background:
    radial-gradient(circle at 50% 42%, rgba(105, 36, 27, 0.34), transparent 40%),
    linear-gradient(180deg, rgba(5, 6, 7, 0.88), rgba(3, 4, 5, 0.98));
  backdrop-filter: blur(4px) saturate(0.7);
}

.death-overlay__panel {
  width: min(720px, 100%);
  max-height: min(760px, 100%);
  overflow: auto;
  padding: clamp(22px, 4vw, 46px);
  border: 1px solid rgba(209, 168, 92, 0.65);
  outline: 1px solid rgba(38, 28, 18, 0.95);
  outline-offset: -5px;
  background:
    linear-gradient(135deg, rgba(74, 29, 25, 0.3), transparent 42%),
    linear-gradient(180deg, rgba(20, 18, 16, 0.98), rgba(9, 10, 11, 0.98));
  box-shadow: 0 24px 80px rgba(0, 0, 0, 0.75), inset 0 0 40px rgba(218, 167, 77, 0.05);
  text-align: center;
}

.death-overlay__eyebrow {
  margin: 0 0 9px;
  color: #d4a959;
  font: 0.68rem 'GameFont', sans-serif;
  letter-spacing: 0.16em;
  text-transform: uppercase;
}

h1 {
  margin: 0;
  color: #f1d58d;
  font: clamp(1.5rem, 4vw, 2.45rem) 'GameFont', Georgia, serif;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  text-shadow: 0 2px 12px rgba(0, 0, 0, 0.8);
}

.death-overlay__lede {
  max-width: 560px;
  margin: 14px auto 24px;
  color: rgba(232, 222, 198, 0.84);
  font-size: 0.95rem;
  line-height: 1.55;
}

.death-overlay__columns {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 10px;
  text-align: left;
}

.death-overlay__card {
  min-height: 124px;
  padding: 14px;
  border: 1px solid rgba(169, 135, 76, 0.34);
  background: rgba(7, 8, 9, 0.56);
}

.death-overlay__card h2 {
  margin: 0 0 10px;
  color: #d6b568;
  font: 0.68rem 'GameFont', sans-serif;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.death-overlay__list {
  display: grid;
  gap: 6px;
  margin: 0;
  padding: 0;
  list-style: none;
  color: rgba(238, 228, 204, 0.9);
  font-size: 0.82rem;
}

.death-overlay__list li {
  display: flex;
  justify-content: space-between;
  gap: 10px;
  border-bottom: 1px solid rgba(172, 135, 70, 0.14);
  padding-bottom: 4px;
}

.death-overlay__list small,
.death-overlay__empty,
.death-overlay__hint {
  color: rgba(186, 177, 155, 0.66);
  font-size: 0.73rem;
}

.death-overlay__empty {
  margin: 0;
  line-height: 1.45;
}

.death-overlay__destination,
.death-overlay__oath {
  margin: 18px 0 0;
  color: rgba(232, 222, 198, 0.82);
  font-size: 0.82rem;
  line-height: 1.45;
}

.death-overlay__destination strong {
  color: #e0b65d;
  font-family: 'GameFont', sans-serif;
  font-size: 0.68rem;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.death-overlay__oath {
  color: #d89473;
}

.death-overlay__continue {
  min-width: 230px;
  margin-top: 24px;
  padding: 12px 24px;
  border: 1px solid #d3a952;
  background: linear-gradient(180deg, #76572d, #4a321c);
  color: #fff0c2;
  font: 0.76rem 'GameFont', sans-serif;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  cursor: pointer;
  box-shadow: inset 0 0 0 1px rgba(255, 234, 175, 0.12), 0 5px 14px rgba(0, 0, 0, 0.44);
}

.death-overlay__continue:hover,
.death-overlay__continue:focus-visible {
  border-color: #f3d27c;
  background: linear-gradient(180deg, #987238, #624321);
  outline: 2px solid rgba(229, 183, 83, 0.36);
  outline-offset: 3px;
}

.death-overlay__hint {
  margin: 9px 0 0;
}

@media (height <= 800px) {
  .death-overlay {
    padding: 10px;
  }

  .death-overlay__panel {
    max-height: calc(100dvh - 20px);
    padding: 16px 18px 18px;
  }

  .death-overlay__lede {
    margin: 10px auto 14px;
  }

  .death-overlay__continue {
    margin-top: 14px;
  }
}

@media (width <= 620px) {
  .death-overlay__columns {
    grid-template-columns: 1fr;
  }

  .death-overlay__panel {
    padding: 22px 16px;
  }
}
</style>
