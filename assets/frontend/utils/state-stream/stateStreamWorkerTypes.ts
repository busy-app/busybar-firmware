export type WorkerMessage = { type: 'SUBSCRIBE'; wsEndpoint: string } | { type: 'UNSUBSCRIBE' } | { type: 'DEVICE_DISCONNECTED' } | { type: 'DEVICE_RECONNECTED' };

export type ClientMessage = { type: 'STATUS'; isConnected: boolean; isReconnected: boolean } | { type: 'DATA'; data: ArrayBuffer } | { type: 'CONNECT_ERROR'; message: string } | { type: 'CHECK_CONNECTION' };
