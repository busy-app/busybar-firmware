import { resolve } from 'path';

const BUILD_ID = process.env.BUILD_ID || 'bsb-frontend';

// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  // use provided build id (e.g. commit hash) or default to a static one
  buildId: BUILD_ID,
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
  nitro: {
    output: {
      dir: process.env.NODE_ENV === 'development' ? resolve(__dirname, 'dist') : resolve(__dirname, '../frontend-build')
    },
    hooks: {
      'prerender:generate' (route) {
        if (route.contents && typeof route.contents === 'string') {
          // find the timestamp and set it to 0
          route.contents = route.contents.replace(
            /\[\{"prerenderedAt":\d+,"serverRendered":\d+\},(\d+),false\]/g,
            (match, timestamp) => {
              return match.replace(timestamp, '0');
            }
          );
        }
      }
    }
  },
  vite: {
    build: {
      rollupOptions: {
        output: {
          entryFileNames: `_nuxt/[name]-${BUILD_ID}.js`,
          chunkFileNames: `_nuxt/[name]-${BUILD_ID}.js`,
          assetFileNames: ({ names, originalFileNames }) => {
            const name = names[0] || originalFileNames[0];
            const ext = name.split('.').pop();
            return `_nuxt/[name]-${BUILD_ID}.${ext}`;
          }
        }
      }
    }
  },
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
  },
  icon: {
    customCollections: [{
      prefix: 'busy',
      dir: './assets/icons'
    }],
    clientBundle: {
      icons: [
        'tabler:arrow-left',
        'tabler:arrow-right',
        'tabler:check',
        'tabler:chevrons-left',
        'tabler:chevrons-right',
        'tabler:chevron-down',
        'tabler:chevron-left',
        'tabler:chevron-right',
        'tabler:chevron-up',
        'tabler:x',
        'tabler:dots',
        'tabler:external-link',
        'tabler:folder',
        'tabler:folder-open',
        'tabler:refresh',
        'tabler:minus',
        'tabler:plus',
        'tabler:search',
        'tabler:moon-filled',
        'tabler:sun-filled',
        'tabler:usb',
        'tabler:upload',
        'tabler:file-upload',
        'tabler:eye-exclamation',
        'tabler:wifi',
        'tabler:wifi-off',
        'tabler:wifi-0',
        'tabler:wifi-1',
        'tabler:wifi-2',
        'tabler:bluetooth',
        'tabler:lock-open',
        'tabler:lock',
        'tabler:plane-departure',
        'tabler:login-2',
        'tabler:alert-triangle-filled'
      ],
      // include all custom collections in the client bundle
      includeCustomCollections: true,
      // guard for uncompressed bundle size, will fail the build if exceeds
      sizeLimitKb: 1024
    }
  }
});
