export default defineAppConfig({
  ui: {
    icons: {
      arrowLeft: 'i-tabler-arrow-left',
      arrowRight: 'i-tabler-arrow-right',
      check: 'i-tabler-check',
      chevronDoubleLeft: 'i-tabler-chevrons-left',
      chevronDoubleRight: 'i-tabler-chevrons-right',
      chevronDown: 'i-tabler-chevron-down',
      chevronLeft: 'i-tabler-chevron-left',
      chevronRight: 'i-tabler-chevron-right',
      chevronUp: 'i-tabler-chevron-up',
      close: 'i-tabler-x',
      ellipsis: 'i-tabler-dots',
      external: 'i-tabler-external-link',
      folder: 'i-tabler-folder',
      folderOpen: 'i-tabler-folder-open',
      loading: 'i-tabler-refresh',
      minus: 'i-tabler-minus',
      plus: 'i-tabler-plus',
      search: 'i-tabler-search'
    },
    colors: {
      primary: 'sky',
      neutral: 'zinc'
    },
    container: {
      base: 'max-w-[1920px]'
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
          class: '!text-white dark:text-white'
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
