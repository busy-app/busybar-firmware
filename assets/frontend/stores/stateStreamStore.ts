import { defineStore } from 'pinia';

export const useStateStreamStore = defineStore('stateStream', () => {
  const deviceStore = useDeviceStore();
  const barUrl = useRuntimeConfig().public.barUrl || window.location.origin;

  const isWebSocketConnected = ref(false);

  const websocket = ref<WebSocket | null>(null);

  type DataCallback = (data: Uint8Array) => void;
  type StopCallback = () => void;

  // const restartTimeout = ref<NodeJS.Timeout | null>(null);

  async function openStateWebsocket (
    dataCallback: DataCallback,
    stopCallback: StopCallback | undefined = () => {
    }
  ) {
    websocket.value = new WebSocket(barUrl.replace(/^http/, 'ws') + '/api/status/ws');
    websocket.value.binaryType = 'arraybuffer';
    websocket.value.onopen = () => {
      isWebSocketConnected.value = true;
    };

    websocket.value.onmessage = event => {
      const data = new Uint8Array(event.data);
      dataCallback(data);

      /* if (restartTimeout.value) {
        clearTimeout(restartTimeout.value);
        restartTimeout.value = null;
      }
      restartTimeout.value = setTimeout(() => {
        console.warn('State publisher websocket connection seems to be lost, restarting...');
        closeWebsocket();
        window.dispatchEvent(new CustomEvent('protobuf-websocket-restart'));
      }, 10000); */
    };

    websocket.value.onclose = () => {
      isWebSocketConnected.value = false;
      deviceStore.checkConnection();
      stopCallback();
    };

    websocket.value.onerror = error => {
      console.error('State publisher websocket error', error);
      isWebSocketConnected.value = false;
      deviceStore.checkConnection();
      stopCallback();
    };

    await new Promise((resolve, reject) => {
      websocket.value?.addEventListener('open', resolve);
      websocket.value?.addEventListener('error', reject);
    });

    // open websocket and send {"enable":true}
    websocket.value.send(JSON.stringify({ enable: true }));
  }

  function closeWebsocket (): Promise<void> {
    isWebSocketConnected.value = false;

    return new Promise(resolve => {
      if (websocket.value) {
        websocket.value.onclose = () => {
          resolve();
        };
        websocket.value.close();
      } else {
        resolve();
      }
    });
  }

  function startStateStream (
    dataCallback: DataCallback,
    stopCallback?: StopCallback
  ) {
    return openStateWebsocket(dataCallback, stopCallback);
  }

  function stopStateStream () {
    return closeWebsocket();
  }

  return {
    isWebSocketConnected,

    startStateStream,
    stopStateStream
  };
});
