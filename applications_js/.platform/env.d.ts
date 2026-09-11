/// <reference types="vite/client" />

interface ImportMetaEnv {
  /** BUSY Bar address for dev mode (see .env). */
  readonly VITE_BUSY_ADDR?: string
}

interface ImportMeta {
  readonly env: ImportMetaEnv
}

/** Importing a .anim file (binary device asset) — Vite returns a URL. */
declare module '*.anim' {
  const src: string
  export default src
}


/** Glyph maps of every font in use, collected by the font-maps plugin. */
declare module 'virtual:font-maps' {
  export const MAPS: { fonts: Record<string, unknown> }[]
}
