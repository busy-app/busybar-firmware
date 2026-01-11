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
    '@/assets/css/typography.css',
    '@/assets/css/global.css',
    '@/assets/css/fonts.css'
  ],
  ui: {
    colorMode: true,
    fonts: false
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
    customCollections: [
      {
        prefix: 'busy',
        dir: './assets/icons/busy'
      },
      {
        prefix: 'bi',
        dir: './assets/icons/bi'
      }
    ],
    clientBundle: {
      icons: [
        'ri:account-circle-fill',
        'ri:add-fill',
        'ri:add-line',
        'ri:alert-line',
        'ri:alert-fill',
        'ri:arrow-down-s-fill',
        'ri:arrow-left-double-fill',
        'ri:arrow-left-line',
        'ri:arrow-left-s-fill',
        'ri:arrow-right-double-fill',
        'ri:arrow-right-line',
        'ri:arrow-right-s-fill',
        'ri:arrow-right-s-line',
        'ri:arrow-up-s-fill',
        'ri:battery-fill',
        'ri:battery-line',
        'ri:battery-low-line',
        'ri:check-line',
        'ri:close-line',
        'ri:cpu-fill',
        'ri:cpu-line',
        'ri:delete-bin-7-line',
        'ri:download-cloud-line',
        'ri:error-warning-line',
        'ri:exchange-2-line',
        'ri:external-link-line',
        'ri:eye-close-line',
        'ri:eye-line',
        'ri:file-2-line',
        'ri:file-copy-line',
        'ri:file-zip-line',
        'ri:folder-2-line',
        'ri:folder-open-line',
        'ri:information-fill',
        'ri:input-method-line',
        'ri:lock-fill',
        'ri:lock-line',
        'ri:lock-password-line',
        'ri:lock-unlock-line',
        'ri:moon-line',
        'ri:more-fill',
        'ri:pencil-line',
        'ri:restart-line',
        'ri:search-line',
        'ri:settings-fill',
        'ri:settings-line',
        'ri:signal-tower-fill',
        'ri:signal-wifi-1-fill',
        'ri:signal-wifi-2-fill',
        'ri:signal-wifi-3-fill',
        'ri:signal-wifi-fill',
        'ri:signal-wifi-line',
        'ri:subtract-fill',
        'ri:sun-line',
        'ri:upload-2-line',
        'ri:usb-line',
        'ri:volume-mute-line',
        'ri:volume-up-line'
      ],
      // include all custom collections in the client bundle
      includeCustomCollections: true,
      // guard for uncompressed bundle size, will fail the build if exceeds
      sizeLimitKb: 1024
    }
  }
});
