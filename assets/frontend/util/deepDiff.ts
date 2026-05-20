function isObjectValue (value: unknown): value is Record<string, unknown> {
  return !!value && typeof value === 'object' && !Array.isArray(value) && !(value instanceof Uint8Array);
}

type DiffLeaf = {
  from: unknown;
  to: unknown;
};

function isDiffLeaf (value: unknown): value is DiffLeaf {
  return isObjectValue(value) && 'from' in value && 'to' in value && Object.keys(value).length === 2;
}

function formatDiffValue (value: unknown): string {
  if (typeof value === 'string') {
    return value;
  }

  if (value === undefined) {
    return 'undefined';
  }

  if (value === null) {
    return 'null';
  }

  if (typeof value === 'number' || typeof value === 'boolean' || typeof value === 'bigint') {
    return String(value);
  }

  try {
    return JSON.stringify(value);
  } catch {
    return String(value);
  }
}

export function deepDiff (oldVal: unknown, newVal: unknown): unknown | undefined {
  if (oldVal === newVal) {
    return undefined;
  }

  if (isObjectValue(oldVal) && isObjectValue(newVal)) {
    const keys = new Set<string>([...Object.keys(oldVal), ...Object.keys(newVal)]);
    const out: Record<string, unknown> = {};
    for (const k of keys) {
      const diff = deepDiff(oldVal[k], newVal[k]);
      if (diff !== undefined) {
        out[k] = diff;
      }
    }
    return Object.keys(out).length ? out : undefined;
  }

  if (Array.isArray(oldVal) && Array.isArray(newVal)) {
    const len = Math.max(oldVal.length, newVal.length);
    const arr: unknown[] = [];
    let changed = false;
    for (let i = 0; i < len; i++) {
      const diff = deepDiff(oldVal[i], newVal[i]);
      if (diff !== undefined) {
        arr[i] = diff;
        changed = true;
      }
    }
    return changed ? arr : undefined;
  }

  return { from: oldVal as unknown, to: newVal as unknown };
}

export function flattenDeepDiff (diff: unknown, prefix = ''): string[] {
  if (diff === undefined) {
    return [];
  }

  if (isDiffLeaf(diff)) {
    const label = prefix || 'value';
    return [`${label}: ${formatDiffValue(diff.from)} -> ${formatDiffValue(diff.to)}`];
  }

  if (Array.isArray(diff)) {
    return diff.flatMap((item, index) => flattenDeepDiff(item, `${prefix}[${index}]`));
  }

  if (isObjectValue(diff)) {
    return Object.entries(diff).flatMap(([key, value]) => {
      const nextPrefix = prefix ? `${prefix}.${key}` : key;
      return flattenDeepDiff(value, nextPrefix);
    });
  }

  return prefix ? [`${prefix}: ${formatDiffValue(diff)}`] : [formatDiffValue(diff)];
}
