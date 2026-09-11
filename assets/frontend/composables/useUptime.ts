import { pad2 } from '@/util/pad';

const UPTIME_REGEX = /^(\d+)d\s+(\d+)h\s+(\d+)m\s+(\d+)s$/;
const SECONDS_PER_MINUTE = 60;
const SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
const SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;

let tickInterval: ReturnType<typeof setInterval> | undefined;
let tickSubscribers = 0;

const now = ref(Date.now());

function parseUptime (uptime: string) {
  const match = uptime.trim().match(UPTIME_REGEX);
  if (!match) {
    return undefined;
  }

  const [days, hours, minutes, seconds] = match.slice(1).map(Number);

  const normalizedDays = hours >= 24 ? 0 : days;

  return normalizedDays * SECONDS_PER_DAY
    + hours * SECONDS_PER_HOUR
    + minutes * SECONDS_PER_MINUTE
    + seconds;
}

function formatUptime (totalSeconds: number) {
  return `${pad2(Math.floor(totalSeconds / SECONDS_PER_DAY))}d `
    + `${pad2(Math.floor(totalSeconds / SECONDS_PER_HOUR) % 24)}h `
    + `${pad2(Math.floor(totalSeconds / SECONDS_PER_MINUTE) % 60)}m`;
}

function subscribeToTick () {
  tickSubscribers++;

  if (tickInterval !== undefined) {
    return;
  }

  const deviceStore = useDeviceStore();
  now.value = Date.now();

  tickInterval = setInterval(() => {
    if (deviceStore.isConnected) {
      now.value = Date.now();
    }
  }, 1000);
}

function unsubscribeFromTick () {
  tickSubscribers = Math.max(0, tickSubscribers - 1);

  if (!tickSubscribers && tickInterval !== undefined) {
    clearInterval(tickInterval);
    tickInterval = undefined;
  }
}

function disposeTick () {
  if (tickInterval !== undefined) {
    clearInterval(tickInterval);
    tickInterval = undefined;
  }

  tickSubscribers = 0;
}

if (import.meta.hot) {
  import.meta.hot.dispose(disposeTick);
}

export function useUptime () {
  const deviceStore = useDeviceStore();

  subscribeToTick();
  onScopeDispose(unsubscribeFromTick);

  return computed(() => {
    const uptime = deviceStore.deviceStatus?.system?.uptime;
    const fetchedAt = deviceStore.deviceStatusFetchedAt;
    if (!uptime || fetchedAt === undefined) {
      return undefined;
    }

    const uptimeSeconds = parseUptime(uptime);
    if (uptimeSeconds === undefined) {
      return uptime;
    }

    const elapsedSeconds = Math.max(0, Math.floor((now.value - fetchedAt) / 1000));
    return formatUptime(uptimeSeconds + elapsedSeconds);
  });
}
