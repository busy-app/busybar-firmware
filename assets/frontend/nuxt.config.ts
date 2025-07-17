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
  }
});
