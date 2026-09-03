import { defineStore } from 'pinia';

export interface TabOption {
  label: string;
  icon: string;
  activeIcon?: string;
  value: string;
  dataId: string;
  hidden?: boolean;
}

export const useTabStore = defineStore('tabs', () => {
  const tabOptions: TabOption[] = [
    {
      label: 'Network',
      icon: 'i-bi-network',
      activeIcon: 'i-bi-network-fill',
      value: 'network',
      dataId: 'tab-network'
    },
    {
      label: 'Firmware',
      icon: 'i-bi-firmware',
      activeIcon: 'i-bi-firmware-fill',
      value: 'firmware',
      dataId: 'tab-firmware'
    },
    {
      label: 'Settings',
      icon: 'i-bi-settings',
      activeIcon: 'i-bi-settings-fill',
      value: 'settings',
      dataId: 'tab-settings'
    },
    {
      label: 'Draw tool',
      icon: 'i-bi-palette',
      value: 'draw-tool',
      dataId: 'tab-draw-tool'
    },
    {
      label: 'Files',
      icon: 'i-bi-folder',
      value: 'files',
      dataId: 'tab-files',
      hidden: true
    },
    {
      label: 'Animations',
      icon: 'i-bi-control-play',
      value: 'animations',
      dataId: 'tab-animations',
      hidden: true
    }
  ];

  const currentTab = ref<TabOption['value']>(tabOptions[0].value);

  const showHiddenTabs = ref(false);

  return {
    tabOptions,
    currentTab,
    showHiddenTabs
  };
}, {
  persist: {
    key: 'tabStore',
    storage: piniaPluginPersistedstate.localStorage()
  }
});
