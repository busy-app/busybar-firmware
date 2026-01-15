import { defineStore } from 'pinia';

export interface TabOption {
  label: string;
  icon: string;
  activeIcon?: string;
  value: string;
  hidden?: boolean;
}

export const useTabStore = defineStore('tabs', () => {
  const tabOptions: TabOption[] = [
    {
      label: 'Network',
      icon: 'i-bi-network',
      activeIcon: 'i-bi-network-fill',
      value: 'network'
    },
    {
      label: 'Firmware',
      icon: 'i-bi-firmware',
      activeIcon: 'i-bi-firmware-fill',
      value: 'firmware'
    },
    {
      label: 'Settings',
      icon: 'i-bi-settings',
      activeIcon: 'i-bi-settings-fill',
      value: 'settings'
    },
    {
      label: 'Animations',
      icon: 'i-bi-control-play',
      value: 'animations',
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
