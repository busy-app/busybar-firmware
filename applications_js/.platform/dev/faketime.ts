// Dev-only time override: patches the global Date while an app runs. The clock keeps ticking from the given point.

const NativeDate = Date

/** Offset of the fake time from the real one, in milliseconds. */
let offset: number | null = null

/** Parses "HH:MM" or "HH:MM:SS" into an offset from now. */
export function parseTime(value: string): number | null {
  const match = /^(\d{1,2}):(\d{2})(?::(\d{2}))?$/.exec(value.trim())
  if (!match) return null

  const [h, m, s] = [Number(match[1]), Number(match[2]), Number(match[3] ?? 0)]
  if (h > 23 || m > 59 || s > 59) return null

  const target = new NativeDate()
  target.setHours(h, m, s, 0)
  return target.getTime() - NativeDate.now()
}

/** Enables the override; an empty string turns it off. */
export function setFakeTime(value: string): boolean {
  if (!value.trim()) {
    offset = null
    return true
  }
  const parsed = parseTime(value)
  if (parsed === null) return false
  offset = parsed
  return true
}

export function isActive(): boolean {
  return offset !== null
}

/** Patches the global Date. Returns a restore function. */
export function installFakeTime(): () => void {
  if (offset === null) return () => {}

  const shift = offset

  class FakeDate extends NativeDate {
    constructor(...args: unknown[]) {
      // Only the no-arg form means "now".
      if (args.length === 0) super(NativeDate.now() + shift)
      else super(...(args as ConstructorParameters<typeof Date>))
    }

    static now(): number {
      return NativeDate.now() + shift
    }
  }

  globalThis.Date = FakeDate as DateConstructor
  return () => {
    globalThis.Date = NativeDate
  }
}
