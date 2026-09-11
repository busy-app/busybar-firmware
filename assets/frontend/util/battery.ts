import { BSB_State, type StatusPower } from '@busy-app/busy-lib';

export const BATTERY_STATUS_NAMES: Record<number, StatusPower['state']> = {
  [BSB_State.BatteryStatus.DISCHARGING]: 'discharging',
  [BSB_State.BatteryStatus.CHARGING]: 'charging',
  [BSB_State.BatteryStatus.CHARGED]: 'charged'
};

export function normalizeBatteryStatus (value: BSB_State.BatteryStatus | StatusPower['state'] | null | undefined) {
  if (value === null || value === undefined) {
    return undefined;
  }

  if (typeof value === 'number') {
    return BATTERY_STATUS_NAMES[value];
  }

  return value;
}
