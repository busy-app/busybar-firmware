import type { CSSProperties } from 'vue';

export const GLASS_CONTROL_STYLE: CSSProperties = {
  borderRadius: 'var(--rounded---ui-radius, 6px)',
  border: '1px solid var(--ui-border-opaque, var(--ui-border))',
  background: 'var(--alpha---ui-bg-elevated-75, rgba(38, 38, 38, 0.75))',
  boxShadow: 'inset 0 1px 1px 0 var(--alpha-white-10, rgba(250, 250, 250, 0.10))',
  backdropFilter: 'blur(32px)'
};
