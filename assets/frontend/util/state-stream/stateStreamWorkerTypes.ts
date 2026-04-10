export type WorkerMessage = { type: 'SUBSCRIBE'; wsEndpoint: string; apiToken?: string | null } | { type: 'UNSUBSCRIBE' } | { type: 'DEVICE_DISCONNECTED' } | { type: 'DEVICE_RECONNECTED' } | { type: 'PAUSE_ACTIVITY_CHECKS' } | { type: 'RESUME_ACTIVITY_CHECKS' };

export type ClientMessage = { type: 'STATUS'; isConnected: boolean; isReconnected: boolean } | { type: 'DATA'; data: ArrayBuffer } | { type: 'CONNECT_ERROR'; message: string } | { type: 'CHECK_CONNECTION' };
