import { defineStore } from 'pinia';

export interface TabOption {
  label: string;
  icon: string;
  activeIcon?: string;
  value: string;
}

export const useTabStore = defineStore('tabs', () => {
  const tabOptions: TabOption[] = [
    {
      label: 'Network',
      icon: 'i-tabler-wifi',
      activeIcon: 'i-tabler-wifi-off',
      value: 'network'
    },
    {
      label: 'Firmware',
      icon: 'i-tabler-file-upload',
      value: 'firmware'
    },
    {
      label: 'Settings',
      icon: 'i-tabler-bluetooth',
      value: 'settings'
    }
  ];
  const currentTab = ref<TabOption['value']>(tabOptions[0].value);

  return {
    tabOptions,
    currentTab
  };
});
