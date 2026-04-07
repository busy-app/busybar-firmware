import protobuf from 'protobufjs';
import type { IConversionOptions, INamespace, Type } from 'protobufjs';
import stateDescriptor from '@/generated/protobuf/stateDescriptor';

const { Root } = protobuf;

type ProtoScalar = string | number | boolean | Uint8Array | null | undefined;

export type ProtoValue = ProtoScalar | ProtoMessage | ProtoValue[];

export interface ProtoMessage {
  [key: string]: ProtoValue;
}

export interface StateFrameMessage extends ProtoMessage {
  screen?: 'FRONT' | 'BACK';
  width?: number;
  height?: number;
  encoding?: 'PLAIN' | 'RUN_LENGTH' | 'DEFLATE' | 'DEFLATE_RUN_LENGTH';
  pixelFormat?: 'RGB888' | 'L8' | 'L4';
  data?: Uint8Array;
}

export interface StateUpdatePayloadMap {
  deviceName: ProtoMessage;
  power: ProtoMessage;
  brightness: ProtoMessage;
  audioVolume: ProtoMessage;
  wifi: ProtoMessage;
  updateState: ProtoMessage;
  updateCheck: ProtoMessage;
  timezone: ProtoMessage;
  matter: ProtoMessage;
  frame: StateFrameMessage;
  input: ProtoMessage;
  timer: ProtoMessage;
}

export type StateUpdateKind = keyof StateUpdatePayloadMap;

export interface StateUpdateMessage extends ProtoMessage {
  state?: StateUpdateKind;
  deviceName?: ProtoMessage;
  power?: ProtoMessage;
  brightness?: ProtoMessage;
  audioVolume?: ProtoMessage;
  wifi?: ProtoMessage;
  updateState?: ProtoMessage;
  updateCheck?: ProtoMessage;
  timezone?: ProtoMessage;
  matter?: ProtoMessage;
  frame?: StateFrameMessage;
  input?: ProtoMessage;
  timer?: ProtoMessage;
}

export interface StateMessage extends ProtoMessage {
  timestamp?: string;
  updates: StateUpdateMessage[];
}

const conversionOptions: IConversionOptions = {
  arrays: true,
  enums: String,
  longs: String,
  objects: true,
  oneofs: true
};

let stateType: Type | null = null;

function normalizeFrameMessage (frame: ProtoMessage | undefined): StateFrameMessage | undefined {
  if (!frame) {
    return undefined;
  }

  return {
    ...frame,
    screen: (frame.screen as StateFrameMessage['screen']) ?? 'FRONT',
    encoding: (frame.encoding as StateFrameMessage['encoding']) ?? 'PLAIN',
    pixelFormat: (frame.pixelFormat as StateFrameMessage['pixelFormat']) ?? 'RGB888'
  };
}

function normalizeStateUpdate (update: StateUpdateMessage): StateUpdateMessage {
  if (update.state !== 'frame') {
    return update;
  }

  return {
    ...update,
    frame: normalizeFrameMessage(update.frame)
  };
}

function normalizeStateMessage (message: StateMessage): StateMessage {
  return {
    ...message,
    updates: (message.updates ?? []).map(update => normalizeStateUpdate(update as StateUpdateMessage))
  };
}

function getStateType (): Type {
  if (!stateType) {
    const root = Root.fromJSON(stateDescriptor as unknown as INamespace);
    root.resolveAll();
    stateType = root.lookupType('BSB_State.State');
  }

  return stateType;
}

function normalizeBinaryMessage (message: ArrayBuffer | Uint8Array): Uint8Array {
  if (message instanceof Uint8Array) {
    return message;
  }

  return new Uint8Array(message);
}

export function decodeStateMessage (message: ArrayBuffer | Uint8Array): StateMessage {
  const type = getStateType();
  const decodedMessage = type.decode(normalizeBinaryMessage(message));

  return normalizeStateMessage(type.toObject(decodedMessage, conversionOptions) as StateMessage);
}

export function getStateUpdatePayload (update: StateUpdateMessage): ProtoMessage | undefined {
  if (!update.state) {
    return undefined;
  }

  return update[update.state] as ProtoMessage | undefined;
}
