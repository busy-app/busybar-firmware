import { defineStore } from 'pinia';
import { ScreenStream, DeviceScreen } from '@busy-app/busy-lib';

export const useScreenStreamStore = defineStore('screenStream', () => {
  const deviceStore = useDeviceStore();

  const currentScreen = ref<DeviceScreen>(DeviceScreen.FRONT);

  const screenStream = ref<ScreenStream | null>(null);
  const isWebSocketConnected = ref(false);

  type DataCallback = (data: Uint8Array) => void;
  type StopCallback = () => void;

  const restartTimeout = ref<NodeJS.Timeout | null>(null);

  async function openWebsocket (
    deviceScreen: DeviceScreen,
    dataCallback: DataCallback,
    stopCallback: StopCallback | undefined = () => {
    }
  ) {
    screenStream.value = new ScreenStream({
      addr: useRuntimeConfig().public.barUrl || window.location.origin,
      apiKey: useApiStore().apiKey || '',
      deviceScreen
    });

    screenStream.value.onData((data: Uint8Array) => {
      isWebSocketConnected.value = true;
      dataCallback(data);

      if (restartTimeout.value) {
        clearTimeout(restartTimeout.value);
        restartTimeout.value = null;
      }
      restartTimeout.value = setTimeout(() => {
        console.warn('WebSocket connection seems to be lost, restarting...');
        screenStream.value?.closeWebsocket();
        window.dispatchEvent(new CustomEvent('screen-stream-restart'));
      }, 3000);
    });

    screenStream.value.onStop(() => {
      isWebSocketConnected.value = false;
      deviceStore.checkConnection();
      stopCallback();
    });

    screenStream.value.onError(({
      raw
    }) => {
      console.error('WebSocket error', raw);
      isWebSocketConnected.value = false;
      deviceStore.checkConnection();
      stopCallback();
    });

    await screenStream.value.openWebsocket();
  }

  function closeWebsocket (): Promise<void> {
    isWebSocketConnected.value = false;

    return new Promise(resolve => {
      if (screenStream.value) {
        screenStream.value.onStop(() => {
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
    isWebSocketConnected,
    currentScreen,

    startScreenStream,
    stopScreenStream
    // setCurrentScreen
  };
}, {
  // https://prazdevs.github.io/pinia-plugin-persistedstate/guide/config
  persist: false
});
