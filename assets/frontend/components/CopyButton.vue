<template>
  <UTooltip
    :delay-duration="0"
    :text="copyState === 'idle' ? 'Copy to clipboard' : copyState === 'copied' ? 'Copied!' : undefined"
  >
    <UButton
      v-bind="$attrs"
      class="justify-between"
      @click="copyToClipboard"
    >
        <template v-if="!$slots.default">{{ props.text }}</template>
        <slot />

        <UIcon
          :name="currentIcon"
          :class="iconClassName"
        />
    </UButton>
  </UTooltip>
</template>

<script setup lang="ts">
const props = defineProps<{
  text: string;
  iconClass?: string;
}>();

const copyState = ref<'idle' | 'copying' | 'copied' | 'error'>('idle');

const currentIcon = computed(() => {
  switch (copyState.value) {
    case 'idle':
      return 'i-bi-copy';
    case 'copied':
      return 'i-bi-checkmark';
    case 'error':
      return 'i-bi-alert';
    default:
      return 'i-bi-copy';
  }
});

const iconClassName = computed(() => {
  switch (copyState.value) {
    case 'copied':
      return `${props.iconClass ?? ''} text-green-500`;
    case 'error':
      return `${props.iconClass ?? ''} text-red-500`;
    default:
      return props.iconClass ?? '';
  }
});

async function copyToClipboard () {
  if (copyState.value === 'copying') {
    return;
  }
  copyState.value = 'copying';
  try {
    if (navigator.clipboard && window.isSecureContext) {
      await navigator.clipboard.writeText(props.text);
    } else {
      // Fallback for insecure context or unsupported clipboard API
      const textarea = document.createElement('textarea');
      textarea.value = props.text;
      textarea.setAttribute('readonly', '');
      textarea.style.position = 'absolute';
      textarea.style.left = '-9999px';
      document.body.appendChild(textarea);
      textarea.select();
      document.execCommand('copy');
      document.body.removeChild(textarea);
    }
    copyState.value = 'copied';
    setTimeout(() => {
      copyState.value = 'idle';
    }, 1500);
  } catch (error) {
    copyState.value = 'error';
    console.error('Failed to copy to clipboard:', error);
    toast.add({
      id: 'copy-error',
      title: 'Failed to copy to clipboard',
      description: 'Check console for more details',
      icon: 'i-bi-alert',
      color: 'error',
      duration: 10000
    });
    setTimeout(() => {
      copyState.value = 'idle';
    }, 2000);
  }
}
</script>
