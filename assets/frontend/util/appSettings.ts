import type { AppSettingsNode } from '@/stores/appsStore';

type StringSettingNode = Extract<AppSettingsNode, { type: 'string' }>;

// Firmware measures string settings in UTF-8 bytes and defaults max_length to 64 (lib/js_app/js_app_settings.c).
const STRING_SETTING_DEFAULT_MAX_LENGTH = 64;

const utf8Encoder = new TextEncoder();

export function getUtf8Length (value: string): number {
  return utf8Encoder.encode(value).length;
}

export function truncateUtf8 (value: string, maxBytes: number): string {
  const { read } = utf8Encoder.encodeInto(value, new Uint8Array(maxBytes));

  return value.slice(0, read);
}

export function getStringSettingError (field: StringSettingNode, value: unknown): string | undefined {
  const length = getUtf8Length(typeof value === 'string' ? value : '');
  const minLength = field.min_length ?? 0;
  const maxLength = field.max_length ?? STRING_SETTING_DEFAULT_MAX_LENGTH;

  if (length < minLength) {
    return minLength === 1 ? 'Required' : `Too short: ${length}/${minLength} bytes`;
  }

  if (length > maxLength) {
    return `Too long: ${length}/${maxLength} bytes`;
  }

  return undefined;
}

export function hasInvalidAppSettings (fields: Record<string, AppSettingsNode>, values: Record<string, unknown>): boolean {
  return Object.entries(fields).some(([key, field]) => {
    if (field.type === 'group') {
      return hasInvalidAppSettings(field.fields, (values[key] ?? {}) as Record<string, unknown>);
    }

    return field.type === 'string' && getStringSettingError(field, values[key]) !== undefined;
  });
}
