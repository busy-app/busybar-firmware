export default defineAppConfig({
  ui: {
    icons: {
      arrowLeft: 'i-ri-arrow-left-line',
      arrowRight: 'i-ri-arrow-right-line',
      check: 'i-ri-check-line',
      chevronDoubleLeft: 'i-ri-arrow-left-double-fill',
      chevronDoubleRight: 'i-ri-arrow-right-double-fill',
      chevronDown: 'i-ri-arrow-down-line',
      chevronLeft: 'i-ri-arrow-left-s-line',
      chevronRight: 'i-ri-arrow-right-s-line',
      chevronUp: 'i-ri-arrow-up-line',
      close: 'i-ri-close-line',
      ellipsis: 'i-ri-more-fill',
      external: 'i-ri-external-link-line',
      folder: 'i-ri-folder-2-line',
      folderOpen: 'i-ri-folder-open-line',
      loading: 'i-busy-loader',
      minus: 'i-ri-subtract-fill',
      plus: 'i-ri-add-fill',
      search: 'i-ri-search-line'
    },
    colors: {
      primary: 'brand',
      neutral: 'neutral'
    },
    container: {
      base: 'p-0 sm:p-0 lg:p-0'
    },
    badge: {
      compoundVariants: [
        {
          color: 'neutral',
          variant: 'soft',
          class: 'text-neutral-400 dark:text-neutral-500 bg-elevated'
        }
      ]
    },
    button: {
      slots: {
        base: 'rounded-lg cursor-pointer'
      },
      variants: {
        size: {
          xl: {
            base: 'py-3'
          }
        }
      },
      compoundVariants: [
        {
          color: 'primary',
          variant: 'solid',
          class: '!text-white dark:text-white dark:bg-primary-500'
        }
      ]
    },
    formField: {
      slots: {
        help: 'mt-1 text-label text-xs text-neutral-500 dark:text-neutral-400',
        error: 'mt-1 text-label text-xs text-red-500 dark:text-red-400'
      }
    },
    input: {
      slots: {
        root: 'w-full',
        base: 'rounded-lg',
        leading: ''
      },
      variants: {
        size: {
          xl: {
            base: 'py-2.5'
          }
        },
        type: {
          file: 'cursor-pointer'
        }
      }
    },
    modal: {
      variants: {
        fullscreen: {
          false: {
            content: 'rounded-3xl'
          }
        }
      }
    }
  }
});
