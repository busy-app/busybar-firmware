/// <reference lib="webworker" />

import type { ClientMessage, WorkerMessage } from './stateStreamWorkerTypes';

declare const self: SharedWorkerGlobalScope | DedicatedWorkerGlobalScope;

type WorkerPort = MessagePort | DedicatedWorkerGlobalScope;

type Connection = {
  socket: WebSocket | null;
  state: {
    isConnected: boolean;
    isReconnected: boolean;
  };
  config: {
    wsEndpoint: string | null;
    apiToken: string | null;
  };
  opening: Promise<void> | null;
  closing: Promise<void> | null;
  reconnectAttempts: number;
  reconnectTimer: ReturnType<typeof setTimeout> | null;
  activityTimer: ReturnType<typeof setTimeout> | null;
  pendingConnectionCheck: ReturnType<typeof setTimeout> | null;
  isDeviceAvailable: boolean | null;
  ports: Set<WorkerPort>;
  hasConnected: boolean;
  activityPauseCount: number;
};

const MAX_RECONNECT_ATTEMPTS = 5;
const ACTIVITY_TIMEOUT_MS = 5000;
const CONNECTION_CHECK_SETTLE_MS = 5000;
const NORMAL_CLOSE_CODE = 1000;

let globalConnection: Connection | null = null;

function getOrInitConnection (wsEndpoint?: string, apiToken?: string | null): Connection {
  if (!globalConnection) {
    globalConnection = {
      socket: null,
      state: {
        isConnected: false,
        isReconnected: false
      },
      config: {
        wsEndpoint: wsEndpoint ?? null,
        apiToken: apiToken ?? null
      },
      opening: null,
      closing: null,
      reconnectAttempts: 0,
      reconnectTimer: null,
      activityTimer: null,
      pendingConnectionCheck: null,
      isDeviceAvailable: null,
      ports: new Set(),
      hasConnected: false,
      activityPauseCount: 0
    };
  }

  if (wsEndpoint && !globalConnection.config.wsEndpoint) {
    globalConnection.config.wsEndpoint = wsEndpoint;
  }

  if (apiToken !== undefined) {
    globalConnection.config.apiToken = apiToken;
  }

  return globalConnection;
}

function postToPort (port: WorkerPort, message: ClientMessage) {
  port.postMessage(message);
}

function broadcast (connection: Connection, message: ClientMessage) {
  connection.ports.forEach(port => {
    try {
      postToPort(port, message);
    } catch {
      // Ignore ports that are no longer reachable.
    }
  });
}

function broadcastStatus (connection: Connection, port?: WorkerPort) {
  const message: ClientMessage = {
    type: 'STATUS',
    isConnected: connection.state.isConnected,
    isReconnected: connection.state.isReconnected
  };

  if (port) {
    postToPort(port, message);
    return;
  }

  broadcast(connection, message);
}

function broadcastData (connection: Connection, data: ArrayBuffer) {
  broadcast(connection, {
    type: 'DATA',
    data
  });
}

function broadcastConnectError (connection: Connection, message: string) {
  broadcast(connection, {
    type: 'CONNECT_ERROR',
    message
  });
}

function clearReconnectTimer (connection: Connection) {
  if (!connection.reconnectTimer) {
    return;
  }

  clearTimeout(connection.reconnectTimer);
  connection.reconnectTimer = null;
}

function clearActivityTimer (connection: Connection) {
  if (!connection.activityTimer) {
    return;
  }

  clearTimeout(connection.activityTimer);
  connection.activityTimer = null;
}

function clearPendingConnectionCheck (connection: Connection) {
  if (!connection.pendingConnectionCheck) {
    return;
  }

  clearTimeout(connection.pendingConnectionCheck);
  connection.pendingConnectionCheck = null;
}

function requestConnectionCheck (connection: Connection) {
  if (connection.pendingConnectionCheck || connection.ports.size === 0 || connection.isDeviceAvailable === false) {
    return;
  }

  connection.pendingConnectionCheck = setTimeout(() => {
    connection.pendingConnectionCheck = null;
  }, CONNECTION_CHECK_SETTLE_MS);

  broadcast(connection, {
    type: 'CHECK_CONNECTION'
  });
}

function resetActivityTimer (connection: Connection) {
  clearActivityTimer(connection);

  if (
    connection.activityPauseCount > 0
    || !connection.socket
    || connection.socket.readyState !== WebSocket.OPEN
    || connection.ports.size === 0
  ) {
    return;
  }

  connection.activityTimer = setTimeout(() => {
    connection.activityTimer = null;
    requestConnectionCheck(connection);
  }, ACTIVITY_TIMEOUT_MS);
}

function pauseActivityChecks (connection: Connection) {
  connection.activityPauseCount++;
  clearActivityTimer(connection);
  clearPendingConnectionCheck(connection);
}

function resumeActivityChecks (connection: Connection) {
  if (connection.activityPauseCount === 0) {
    return;
  }

  connection.activityPauseCount--;

  if (connection.activityPauseCount > 0) {
    return;
  }

  resetActivityTimer(connection);
}

function disconnectForDeviceLoss (connection: Connection) {
  clearReconnectTimer(connection);
  clearActivityTimer(connection);
  clearPendingConnectionCheck(connection);
  connection.opening = null;

  const socket = connection.socket;
  connection.socket = null;
  connection.state.isConnected = false;
  connection.state.isReconnected = false;
  connection.hasConnected = false;
  connection.reconnectAttempts = 0;
  broadcastStatus(connection);

  if (!socket) {
    return;
  }

  socket.onopen = null;
  socket.onmessage = null;
  socket.onerror = null;
  socket.onclose = null;

  if (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN) {
    socket.close(NORMAL_CLOSE_CODE);
  }
}

function toStateStreamWebSocketUrl (wsEndpoint: string, apiToken?: string | null): string {
  const stateStreamUrl = new URL(wsEndpoint);

  stateStreamUrl.protocol = stateStreamUrl.protocol === 'https:' ? 'wss:' : 'ws:';
  stateStreamUrl.pathname = `${stateStreamUrl.pathname.replace(/\/$/, '')}/api/status/ws`;

  if (apiToken) {
    stateStreamUrl.searchParams.set('x-api-token', apiToken);
  }

  return stateStreamUrl.toString();
}

function closeConnection (connection: Connection): Promise<void> {
  clearReconnectTimer(connection);
  clearActivityTimer(connection);
  clearPendingConnectionCheck(connection);
  connection.state.isConnected = false;
  connection.state.isReconnected = false;

  if (connection.closing) {
    return connection.closing;
  }

  const socket = connection.socket;
  connection.socket = null;

  if (!socket) {
    connection.hasConnected = false;
    connection.reconnectAttempts = 0;
    return Promise.resolve();
  }

  const closing = new Promise<void>(resolve => {
    socket.onopen = null;
    socket.onmessage = null;
    socket.onerror = null;
    socket.onclose = () => {
      resolve();
    };

    if (socket.readyState === WebSocket.CLOSING || socket.readyState === WebSocket.CLOSED) {
      resolve();
      return;
    }

    socket.close(NORMAL_CLOSE_CODE);
  }).finally(() => {
    connection.hasConnected = false;
    connection.reconnectAttempts = 0;
    connection.closing = null;
  });

  connection.closing = closing;

  return closing;
}

function scheduleReconnect (connection: Connection) {
  clearActivityTimer(connection);
  clearPendingConnectionCheck(connection);

  if (connection.isDeviceAvailable === false) {
    connection.state.isReconnected = false;
    broadcastStatus(connection);
    return;
  }

  if (connection.reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
    connection.state.isReconnected = false;
    broadcastStatus(connection);
    broadcastConnectError(connection, 'State stream WebSocket disconnected.');
    return;
  }

  connection.state.isReconnected = true;
  broadcastStatus(connection);

  const delay = Math.min(30000, 2 ** connection.reconnectAttempts * 1000) + Math.random() * 500;
  connection.reconnectAttempts++;
  clearReconnectTimer(connection);
  connection.reconnectTimer = setTimeout(() => {
    connection.reconnectTimer = null;
    openConnection(connection).catch(() => {
      // Final failures are reported from the socket close handler.
    });
  }, delay);
}

async function openConnection (connection: Connection): Promise<void> {
  if (connection.closing) {
    await connection.closing;
  }

  if (!connection.config.wsEndpoint) {
    throw new Error('State stream WebSocket endpoint is missing.');
  }

  if (connection.isDeviceAvailable === false) {
    return;
  }

  if (connection.socket) {
    if (connection.socket.readyState === WebSocket.OPEN || connection.socket.readyState === WebSocket.CONNECTING) {
      return;
    }

    connection.socket.close();
  }

  if (connection.opening) {
    await connection.opening;
    return;
  }

  clearReconnectTimer(connection);

  const opening = new Promise<void>((resolve, reject) => {
    const socket = new WebSocket(toStateStreamWebSocketUrl(connection.config.wsEndpoint!, connection.config.apiToken));
    let hasOpened = false;

    socket.binaryType = 'arraybuffer';
    connection.socket = socket;

    socket.onopen = () => {
      hasOpened = true;
      connection.hasConnected = true;
      connection.reconnectAttempts = 0;
      connection.state.isConnected = true;
      connection.state.isReconnected = false;
      connection.isDeviceAvailable = true;
      clearPendingConnectionCheck(connection);

      socket.send(JSON.stringify({ enable: true }));
      broadcastStatus(connection);
      resetActivityTimer(connection);
      resolve();
    };

    socket.onmessage = event => {
      clearPendingConnectionCheck(connection);
      resetActivityTimer(connection);

      if (event.data instanceof ArrayBuffer) {
        broadcastData(connection, event.data);
        return;
      }

      if (event.data instanceof Blob) {
        void event.data.arrayBuffer().then(data => {
          broadcastData(connection, data);
        }).catch(() => {
          // Ignore malformed payloads.
        });
      }
    };

    socket.onerror = () => {
      if (!hasOpened) {
        reject(new Error('Failed to open the state stream WebSocket.'));
      }
    };

    socket.onclose = event => {
      connection.socket = null;
      connection.state.isConnected = false;
      clearActivityTimer(connection);
      clearPendingConnectionCheck(connection);

      if (connection.ports.size === 0) {
        connection.state.isReconnected = false;
        return;
      }

      const shouldReconnect = connection.hasConnected
        && event.code !== NORMAL_CLOSE_CODE
        && connection.reconnectAttempts < MAX_RECONNECT_ATTEMPTS;

      if (shouldReconnect) {
        scheduleReconnect(connection);
        return;
      }

      connection.state.isReconnected = false;
      broadcastStatus(connection);

      const message = connection.hasConnected
        ? `State stream WebSocket disconnected (${event.code}).`
        : `Failed to connect the state stream WebSocket (${event.code}).`;

      broadcastConnectError(connection, message);

      if (!hasOpened) {
        reject(new Error(message));
      }
    };
  }).finally(() => {
    connection.opening = null;
  });

  connection.opening = opening;

  await opening;
}

function handleSubscribe (port: WorkerPort, wsEndpoint: string, apiToken?: string | null) {
  const connection = getOrInitConnection(wsEndpoint, apiToken);

  if (connection.config.wsEndpoint && connection.config.wsEndpoint !== wsEndpoint) {
    console.warn(`Ignoring mismatched state stream endpoint: ${wsEndpoint}`);
  }

  connection.ports.add(port);
  broadcastStatus(connection, port);

  if (!connection.socket || connection.socket.readyState !== WebSocket.OPEN) {
    openConnection(connection).catch(() => {
      // Initial connection failures are reported to clients through CONNECT_ERROR.
    });
  }
}

function handleDeviceDisconnected (connection: Connection) {
  if (connection.isDeviceAvailable === false) {
    return;
  }

  connection.isDeviceAvailable = false;
  disconnectForDeviceLoss(connection);
}

function handleDeviceReconnected (connection: Connection) {
  connection.isDeviceAvailable = true;
  clearPendingConnectionCheck(connection);

  if (connection.ports.size === 0 || connection.socket || connection.opening) {
    return;
  }

  void openConnection(connection).catch(() => {
    // Initial connection failures are reported to clients through CONNECT_ERROR.
  });
}

function handleUnsubscribe (port: WorkerPort) {
  const connection = globalConnection;
  if (!connection) {
    return;
  }

  connection.ports.delete(port);

  if (connection.ports.size === 0) {
    void closeConnection(connection);
  }
}

function handleWorkerMessage (port: WorkerPort, msg: WorkerMessage) {
  console.debug('[message]', msg);

  const connection = globalConnection;

  if (msg.type === 'SUBSCRIBE') {
    handleSubscribe(port, msg.wsEndpoint, msg.apiToken);
    return;
  }

  if (msg.type === 'UNSUBSCRIBE') {
    handleUnsubscribe(port);
    return;
  }

  if (!connection) {
    return;
  }

  if (msg.type === 'DEVICE_DISCONNECTED') {
    handleDeviceDisconnected(connection);
    return;
  }

  if (msg.type === 'DEVICE_RECONNECTED') {
    handleDeviceReconnected(connection);
    return;
  }

  if (msg.type === 'PAUSE_ACTIVITY_CHECKS') {
    pauseActivityChecks(connection);
    return;
  }

  if (msg.type === 'RESUME_ACTIVITY_CHECKS') {
    resumeActivityChecks(connection);
  }
}

function onConnect (port: WorkerPort) {
  port.onmessage = (event: MessageEvent<WorkerMessage>) => {
    handleWorkerMessage(port, event.data);
  };
}

if ('onconnect' in self) {
  (self as SharedWorkerGlobalScope).onconnect = (event: MessageEvent) => {
    const port = event.ports[0];
    onConnect(port);
    port.start();
  };
} else {
  onConnect(self as DedicatedWorkerGlobalScope);
}
