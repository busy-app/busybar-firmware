import { defineStore } from 'pinia';
import { ScreenStream, DeviceScreen } from '@busy-app/busy-lib';

export const useDeviceScreenStreamStore = defineStore('deviceScreenStream', () => {
  const barUrl = useRuntimeConfig().public.barUrl ? `http://${useRuntimeConfig().public.barUrl}` : location.origin;
  const apiKey = useApiStore().apiKey;

  const currentScreen = ref<DeviceScreen>(DeviceScreen.FRONT);

  const screenStream = ref<ScreenStream | null>(null);
  const isConnected = ref(false);

  type DataCallback = (data: Uint8Array) => void;
  type StopCallback = () => void;

  async function openWebsocket (
    deviceScreen: DeviceScreen,
    dataCallback: DataCallback,
    stopCallback: StopCallback | undefined = () => {
    }
  ) {
    screenStream.value = new ScreenStream({
      mode: 'local',
      apiKey: apiKey || '',
      barUrl,
      deviceScreen
    });

    screenStream.value.onData((data: Uint8Array) => {
      isConnected.value = true;
      dataCallback(data);
    });

    screenStream.value.onStop(() => {
      isConnected.value = false;
      stopCallback();
    });

    screenStream.value.onError(({
      raw
    }) => {
      console.error('WebSocket error', raw);
      isConnected.value = false;
      stopCallback();
    });

    await screenStream.value.openWebsocket();
  }

  function closeWebsocket (): Promise<void> {
    isConnected.value = false;

    return new Promise(resolve => {
      if (screenStream.value) {
        screenStream.value.onStop(() => {
          console.log('WebSocket disconnect');

          resolve();
        });
        screenStream.value.closeWebsocket();
      } else {
        resolve();
      }
    });
  }

  function startScreenStream (
    dataCallback: DataCallback,
    stopCallback?: StopCallback
  ) {
    return openWebsocket(currentScreen.value, dataCallback, stopCallback);
  }

  function stopScreenStream () {
    return closeWebsocket();
  }

  // function setCurrentScreen (screen: DeviceScreen) {
  //   if (currentScreen.value === screen) {
  //     return;
  //   }
  //   currentScreen.value = screen;
  //   if (socket.value) {
  //     socket.value.send(JSON.stringify({ display: screen }));
  //   }
  // }

  return {
    // state
    isConnected,
    currentScreen,
    // actions
    startScreenStream,
    stopScreenStream
    // setCurrentScreen
  };
}, {
  // https://prazdevs.github.io/pinia-plugin-persistedstate/guide/config
  persist: false
});
