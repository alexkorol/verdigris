<template>
  <div class="bindings">
    <div
      v-for="action in actions"
      :key="action.id"
      class="bindings__row"
      :class="{ 'bindings__row--capturing': captureActionId === action.id }"
    >
      <div class="bindings__info">
        <strong>{{ action.label }}</strong>
        <small>{{ action.id }}</small>
      </div>
      <div class="bindings__keys">
        <span
          v-for="key in bindingsFor(action.id)"
          :key="key"
          class="bindings__chip"
        >
          {{ labelFor(key) }}
          <button
            type="button"
            class="bindings__chip-remove"
            :aria-label="`Remove ${labelFor(key)} from ${action.label}`"
            @click="removeBinding(action.id, key)"
          >
            &times;
          </button>
        </span>
        <button
          type="button"
          class="bindings__add"
          @click="beginCapture(action.id)"
        >
          {{ captureActionId === action.id ? 'Press a key or mouse button…' : '+ Rebind' }}
        </button>
      </div>
    </div>

    <p
      v-if="captureActionId"
      class="bindings__capture-hint"
    >
      Press any key or mouse button to bind it. Esc cancels.
    </p>
    <p
      v-if="error"
      class="bindings__error"
    >
      {{ error }}
    </p>
    <p class="bindings__hint">
      LMB attacks and RMB casts in the world, aimed at the cursor. The context
      menu stays available via Shift&#8202;+&#8202;right-click.
    </p>

    <button
      type="button"
      class="bindings__reset"
      @click="resetAll"
    >
      Reset all to defaults
    </button>
  </div>
</template>

<script>
import {
  ACTION_DEFINITIONS,
  MOUSE_BUTTON_TO_BINDING,
  addActionBinding,
  displayBinding,
  getActionBindings,
  removeActionBinding,
  resetBindings,
  subscribeBindings,
} from '../../core/config/controls.js';

export default {
  name: 'SettingsBindings',
  data() {
    return {
      actions: ACTION_DEFINITIONS,
      captureActionId: null,
      error: '',
      // Bumped on every live binding change so the rows re-render.
      bindingsVersion: 0,
    };
  },
  created() {
    this.unsubscribeBindings = subscribeBindings(() => {
      this.bindingsVersion += 1;
    });
  },
  beforeUnmount() {
    this.endCapture();
    if (typeof this.unsubscribeBindings === 'function') {
      this.unsubscribeBindings();
      this.unsubscribeBindings = null;
    }
  },
  methods: {
    bindingsFor(actionId) {
      void this.bindingsVersion;
      return getActionBindings(actionId);
    },
    labelFor(binding) {
      return displayBinding(binding);
    },
    removeBinding(actionId, binding) {
      this.error = '';
      removeActionBinding(actionId, binding);
    },
    beginCapture(actionId) {
      if (this.captureActionId === actionId) {
        this.endCapture();
        return;
      }
      this.endCapture();
      this.error = '';
      this.captureActionId = actionId;
      window.addEventListener('keydown', this.handleCaptureKeydown, { capture: true });
      window.addEventListener('mousedown', this.handleCaptureMousedown, { capture: true });
      window.addEventListener('contextmenu', this.handleCaptureContextmenu, { capture: true });
    },
    endCapture() {
      if (!this.captureActionId) {
        return;
      }
      this.captureActionId = null;
      window.removeEventListener('keydown', this.handleCaptureKeydown, { capture: true });
      window.removeEventListener('mousedown', this.handleCaptureMousedown, { capture: true });
      window.removeEventListener('contextmenu', this.handleCaptureContextmenu, { capture: true });
    },
    handleCaptureKeydown(event) {
      event.preventDefault();
      event.stopPropagation();
      if (event.repeat) {
        return;
      }
      if (event.key === 'Escape' || event.key === 'Esc') {
        this.endCapture();
        return;
      }
      this.assignBinding(event.key);
    },
    handleCaptureMousedown(event) {
      event.preventDefault();
      event.stopPropagation();
      const pseudoKey = MOUSE_BUTTON_TO_BINDING[event.button];
      if (pseudoKey) {
        this.assignBinding(pseudoKey);
      }
    },
    handleCaptureContextmenu(event) {
      // Keep the browser menu out of the way while RMB is being captured.
      event.preventDefault();
      event.stopPropagation();
    },
    assignBinding(rawBinding) {
      const actionId = this.captureActionId;
      if (!actionId) {
        return;
      }
      const result = addActionBinding(actionId, rawBinding);
      if (!result.ok && result.conflict) {
        const owner = this.actions.find((action) => action.id === result.conflict);
        const ownerLabel = owner ? owner.label : result.conflict;
        this.error = `${displayBinding(rawBinding)} is already bound to ${ownerLabel}.`;
      } else {
        this.error = '';
      }
      this.endCapture();
    },
    resetAll() {
      this.error = '';
      this.endCapture();
      resetBindings();
    },
  },
};
</script>

<style lang="scss" scoped>
.bindings {
  width: 100%;

  &__row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
    padding: 7px 8px;
    border-bottom: 1px solid var(--color-border-subtle);

    &--capturing {
      outline: 1px solid var(--color-accent);
      background: rgba(224, 163, 78, 0.12);
    }
  }

  &__info {
    display: flex;
    flex-direction: column;
    gap: 3px;

    strong {
      color: var(--color-text-primary);
      font-weight: 500;
    }

    small {
      color: var(--color-text-dim);
      font: 0.68rem "ChatFont", sans-serif;
    }
  }

  &__keys {
    display: flex;
    align-items: center;
    flex-wrap: wrap;
    justify-content: flex-end;
    gap: 6px;
  }

  &__chip {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    padding: 2px 6px;
    color: var(--color-accent-strong);
    background: rgba(0, 0, 0, 0.45);
    border: 1px solid var(--color-border-subtle);
    font-size: 0.72rem;
  }

  &__chip-remove {
    padding: 0 2px;
    color: var(--color-text-dim);
    background: none;
    border: none;
    cursor: pointer;
    font-size: 0.8rem;
    line-height: 1;

    &:hover {
      color: var(--color-accent-strong);
    }
  }

  &__add {
    padding: 2px 8px;
    color: var(--color-text-primary);
    background: rgba(0, 0, 0, 0.35);
    border: 1px dashed var(--color-border-subtle);
    cursor: pointer;
    font-size: 0.72rem;

    &:hover {
      color: var(--color-accent-strong);
      border-color: var(--color-accent);
    }
  }

  &__capture-hint {
    margin: 10px 0 0;
    color: var(--color-accent-strong);
    font-size: 0.72rem;
  }

  &__error {
    margin: 10px 0 0;
    color: #e06c5b;
    font-size: 0.72rem;
  }

  &__hint {
    margin: 10px 0 0;
    color: var(--color-text-dim);
    font: 0.7rem "ChatFont", sans-serif;
  }

  &__reset {
    margin-top: 12px;
    padding: 5px 12px;
    color: var(--color-text-primary);
    background: rgba(0, 0, 0, 0.35);
    border: 1px solid var(--color-border-subtle);
    cursor: pointer;
    font-size: 0.75rem;

    &:hover {
      color: var(--color-accent-strong);
      border-color: var(--color-accent);
    }
  }
}
</style>
