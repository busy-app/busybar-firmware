export default defineAppConfig({
  ui: {
    icons: {
      arrowLeft: 'i-bi-arrow-back',
      arrowRight: 'i-ri-arrow-right-line',
      check: 'i-bi-checkmark-circle-fill',
      chevronDoubleLeft: 'i-ri-arrow-left-double-fill',
      chevronDoubleRight: 'i-ri-arrow-right-double-fill',
      chevronDown: 'i-bi-chevron-down',
      chevronLeft: 'i-ri-arrow-left-s-fill',
      chevronRight: 'i-ri-arrow-right-s-fill',
      chevronUp: 'i-ri-arrow-up-s-fill',
      close: 'i-bi-cross',
      ellipsis: 'i-bi-more',
      external: 'i-bi-open-in-new',
      file: 'i-bi-file',
      folder: 'i-bi-folder',
      folderOpen: 'i-ri-folder-open-line',
      loading: 'i-busy-loader',
      minus: 'i-ri-subtract-fill',
      plus: 'i-bi-plus',
      search: 'i-bi-search',
      upload: 'i-bi-upload'
    },
    colors: {
      primary: 'brand',
      neutral: 'neutral'
    },
    container: {
      base: 'p-0 sm:p-0 lg:p-0'
    },
    button: {
      slots: {
        base: 'rounded-full cursor-pointer'
      },
      variants: {
        size: {
          md: {
            base: 'px-3 py-2'
          },
          lg: {
            base: 'px-3 py-2.5'
          },
          xl: {
            base: 'py-3'
          }
        }
      },
      compoundVariants: [
        {
          color: 'primary',
          variant: 'solid',
          class: 'dark:text-white dark:bg-primary-500'
        },
        {
          color: 'neutral',
          variant: 'outline',
          class: 'bg-transparent'
        }
      ]
    },
    formField: {
      slots: {
        error: 'mt-1 text-xs'
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
            content: 'max-w-[480px] rounded-3xl divide-none bg-modal ring-1 ring-glass',
            overlay: 'bg-modal-overlay'
          }
        }
      }
    },
    switch: {
      slots: {
        base: 'dark:data-[state=checked]:bg-primary-500',
        thumb: 'bg-white dark:bg-white'
      },
      defaultVariants: {
        size: 'xl'
      }
    },
    separator: {
      variants: {
        color: {
          neutral: {
            border: 'border-divider'
          }
        }
      }
    },
    tooltip: {
      slots: {
        content: 'bg-inverted text-inverted'
      }
    }
  }
});
