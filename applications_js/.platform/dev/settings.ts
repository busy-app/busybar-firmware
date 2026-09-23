// Dev-only settings editor: builds a form from the app's appmeta/settings.json and exposes the values through globalThis.Settings, as the device does. Values persist across page reloads.

export interface Field {
  id: string
  type: 'boolean' | 'number' | 'string' | 'enum' | 'color' | 'time'
  label: string
  default: unknown
  options?: { value: string; label: string }[]
  min?: number
  max?: number
  step?: number
  min_length?: number
  max_length?: number
  sensitive?: boolean
}

export interface Section {
  id: string
  label: string
  description?: string
  fields: Field[]
}

export interface Descriptor {
  format_version: number
  version: number
  sections: Section[]
}

type Values = Record<string, unknown>

// Descriptors of all apps; eager, they're tiny.
const DESCRIPTORS = import.meta.glob<Descriptor>('/*/appmeta/settings.json', {
  eager: true,
  import: 'default',
})

/** The app's settings descriptor, if it has one. */
export function descriptorOf(app: string): Descriptor | null {
  return DESCRIPTORS[`/${app}/appmeta/settings.json`] ?? null
}

/** All descriptor fields, flattened across sections. */
function allFields(descriptor: Descriptor): Field[] {
  return descriptor.sections.flatMap((section) => section.fields)
}

/** Default values from the descriptor. */
function defaults(descriptor: Descriptor): Values {
  const values: Values = {}
  for (const field of allFields(descriptor)) values[field.id] = field.default
  return values
}

const storageKey = (app: string) => `busy-dev-settings:${app}`

/** localStorage on top of defaults, keeping only fields still in the schema. */
export function loadValues(app: string, descriptor: Descriptor): Values {
  const values = defaults(descriptor)
  try {
    const stored = JSON.parse(localStorage.getItem(storageKey(app)) ?? '{}') as Values
    for (const field of allFields(descriptor)) {
      if (field.id in stored) values[field.id] = stored[field.id]
    }
  } catch {
    // Corrupt localStorage; fall back to defaults.
  }
  return values
}

export function saveValues(app: string, values: Values): void {
  localStorage.setItem(storageKey(app), JSON.stringify(values))
}

export function resetValues(app: string): void {
  localStorage.removeItem(storageKey(app))
}

/** Exposes the values via globalThis.Settings, as the device runtime does. */
export function installSettingsApi(app: string, descriptor: Descriptor | null): void {
  if (!descriptor) {
    delete (globalThis as { Settings?: unknown }).Settings
    return
  }
  ;(globalThis as { Settings?: unknown }).Settings = {
    load: async () => ({
      format_version: descriptor.format_version,
      version: descriptor.version,
      values: loadValues(app, descriptor),
    }),
    save: async (values: Values) => saveValues(app, values),
  }
}

// ── Form ───────────────────────────────────────────────────────────────────

/** Field → input element, plus a getter for its current value. */
function control(field: Field, value: unknown): { el: HTMLElement; read: () => unknown } {
  const common = 'padding:.3rem;font-size:.9rem;min-width:12rem'

  if (field.type === 'boolean') {
    const input = document.createElement('input')
    input.type = 'checkbox'
    input.checked = Boolean(value)
    input.style.cssText = 'width:1.1rem;height:1.1rem;cursor:pointer'
    return { el: input, read: () => input.checked }
  }

  if (field.type === 'enum') {
    const select = document.createElement('select')
    select.style.cssText = common
    for (const option of field.options ?? []) {
      const el = document.createElement('option')
      el.value = option.value
      el.textContent = option.label
      select.appendChild(el)
    }
    select.value = String(value)
    return { el: select, read: () => select.value }
  }

  const input = document.createElement('input')
  input.style.cssText = common

  switch (field.type) {
    case 'number':
      input.type = 'number'
      if (field.min != null) input.min = String(field.min)
      if (field.max != null) input.max = String(field.max)
      if (field.step != null) input.step = String(field.step)
      input.value = String(value ?? '')
      return { el: input, read: () => input.valueAsNumber }
    case 'color':
      input.type = 'color'
      // <input type=color> only takes #RRGGBB, so alpha round-trips manually.
      {
        const raw = String(value ?? '#000000')
        const alpha = raw.length === 9 ? raw.slice(7) : ''
        input.value = raw.slice(0, 7)
        return { el: input, read: () => `${input.value}${alpha}` }
      }
    case 'time':
      input.type = 'time'
      input.value = String(value ?? '')
      return { el: input, read: () => input.value }
    default:
      input.type = field.sensitive ? 'password' : 'text'
      if (field.max_length != null) input.maxLength = field.max_length
      input.value = String(value ?? '')
      return { el: input, read: () => input.value }
  }
}

/** Renders the settings form; changes are saved immediately. */
export function renderSettings(
  container: HTMLElement,
  app: string,
  descriptor: Descriptor,
  onChange?: () => void,
): void {
  container.innerHTML = ''
  const values = loadValues(app, descriptor)
  const readers = new Map<string, () => unknown>()

  const commit = (): void => {
    const next: Values = {}
    for (const [id, read] of readers) next[id] = read()
    saveValues(app, next)
    onChange?.()
  }

  for (const section of descriptor.sections) {
    const box = document.createElement('fieldset')
    box.style.cssText = 'margin:.5rem 0;padding:.5rem .75rem;border:1px solid #ccc;border-radius:4px'

    const legend = document.createElement('legend')
    legend.textContent = section.label
    legend.style.cssText = 'padding:0 .3rem;font-weight:600'
    box.appendChild(legend)

    if (section.description) {
      const hint = document.createElement('div')
      hint.textContent = section.description
      hint.style.cssText = 'font-size:.8rem;opacity:.7;margin-bottom:.4rem'
      box.appendChild(hint)
    }

    for (const field of section.fields) {
      const row = document.createElement('label')
      row.style.cssText =
        'display:flex;align-items:center;gap:.6rem;justify-content:space-between;margin:.35rem 0'

      const name = document.createElement('span')
      name.textContent = field.label
      name.style.cssText = 'font-size:.9rem'

      const { el, read } = control(field, values[field.id])
      readers.set(field.id, read)
      el.addEventListener('change', commit)

      row.append(name, el)
      box.appendChild(row)
    }

    container.appendChild(box)
  }

  const reset = document.createElement('button')
  reset.textContent = 'Reset to defaults'
  reset.style.cssText = 'margin-top:.3rem;padding:.35rem .7rem;font-size:.85rem;cursor:pointer'
  reset.addEventListener('click', () => {
    resetValues(app)
    renderSettings(container, app, descriptor, onChange)
    onChange?.()
  })
  container.appendChild(reset)
}
