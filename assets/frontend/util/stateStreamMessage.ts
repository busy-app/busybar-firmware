import protobuf from 'protobufjs';
import type { IConversionOptions, INamespace, Type } from 'protobufjs';
import stateDescriptor from '@/generated/protobuf/stateDescriptor';

const { Root } = protobuf;

type ProtoScalar = string | number | boolean | Uint8Array | null | undefined;

export type ProtoValue = ProtoScalar | ProtoMessage | ProtoValue[];

export interface ProtoMessage {
  [key: string]: ProtoValue;
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
  frame: ProtoMessage;
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
  frame?: ProtoMessage;
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

  return type.toObject(decodedMessage, conversionOptions) as StateMessage;
}

export function getStateUpdatePayload (update: StateUpdateMessage): ProtoMessage | undefined {
  if (!update.state) {
    return undefined;
  }

  return update[update.state] as ProtoMessage | undefined;
}
