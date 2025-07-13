// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  buildId: 'bsb-frontend',
  modules: [
    '@nuxt/ui',
    '@nuxt/eslint',
    '@pinia/nuxt',
    'pinia-plugin-persistedstate/nuxt'
  ],
  ssr: false,
  imports: {
    dirs: ['./util']
  },
  devtools: {
    enabled: true,

    timeline: {
      enabled: true
    }
  },
  app: {
    head: {
      link: [
        { rel: 'icon', type: 'image/svg+xml', href: '/favicon.svg' },
        { rel: 'icon', type: 'image/x-icon', href: '/favicon.ico' }
      ]
    }
  },
  css: [
    '@/assets/css/fonts.css',
    '@/assets/css/typography.css',
    '@/assets/css/global.css'
  ],
  ui: {
    colorMode: true
  },
  runtimeConfig: {
    public: {
      barUrl: ''
    }
  },
  experimental: {
    appManifest: false,
    buildCache: true
  },
  compatibilityDate: '2025-05-15',
  eslint: {
    checker: {
      configType: 'flat'
    },
    config: {
      stylistic: {
        semi: true,
        quotes: 'single',
        indent: 2,
        jsx: true,
        arrowParens: false,
        braceStyle: '1tbs',
        commaDangle: 'never'
      }
    }
  }
});
