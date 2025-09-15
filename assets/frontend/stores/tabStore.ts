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
      icon: 'i-ri-signal-tower-fill',
      activeIcon: 'i-ri-signal-tower-fill',
      value: 'network'
    },
    {
      label: 'Firmware',
      icon: 'i-ri-cpu-line',
      value: 'firmware'
    },
    {
      label: 'Settings',
      icon: 'i-ri-settings-line',
      activeIcon: 'i-ri-settings-fill',
      value: 'settings'
    }
  ];
  const currentTab = ref<TabOption['value']>(tabOptions[0].value);

  return {
    tabOptions,
    currentTab
  };
});
