// Dev hub: finds every <app>/main.ts at the project root and runs/stops it.
// App contract: `export default function run()`. Stop kills the timers, rAF and requests the app started, and clears its screen.

import { device } from '@shared/device'
import { uploadResources } from './assets.ts'
import { descriptorOf, installSettingsApi, renderSettings } from './settings.ts'
import { installFakeTime, isActive as fakeTimeActive, setFakeTime } from './faketime.ts'

type AppModule = { default: () => void }

const modules = import.meta.glob<AppModule>('/*/main.ts')

// `id` is the name the app draws under on the device.
const MANIFESTS = import.meta.glob<{ id: string; name?: string }>(
  '/*/appmeta/manifest.json',
  { eager: true, import: 'default' },
)

const apps = Object.keys(modules)
  .map((path) => {
    const folder = path.match(/^\/([^/]+)\/main\.ts$/)?.[1] ?? path
    return {
      path,
      name: folder,
      // Without a manifest the app still runs, but may not get cleared.
      appId: MANIFESTS[`/${folder}/appmeta/manifest.json`]?.id ?? folder,
    }
  })
  .sort((a, b) => a.name.localeCompare(b.name))

// ── Sandbox: timer/rAF interception while an app runs ──────────────────────

const native = {
  setInterval: window.setInterval.bind(window),
  clearInterval: window.clearInterval.bind(window),
  setTimeout: window.setTimeout.bind(window),
  clearTimeout: window.clearTimeout.bind(window),
  requestAnimationFrame: window.requestAnimationFrame.bind(window),
  cancelAnimationFrame: window.cancelAnimationFrame.bind(window),
}

type Running = {
  name: string
  /** Device-side name (manifest.id) used for elements and assets. */
  appId: string
  intervals: Set<number>
  timeouts: Set<number>
  rafs: Set<number>
  abort: AbortController
  restoreTime: () => void
}

let current: Running | null = null

// Preserved across the render() that rebuilds the list.
const openSettings = new Set<string>()

function installSandbox(run: Running): void {
  window.setInterval = ((handler: TimerHandler, timeout?: number, ...args: unknown[]) => {
    const id = native.setInterval(handler, timeout, ...args)
    run.intervals.add(id)
    return id
  }) as typeof window.setInterval

  window.setTimeout = ((handler: TimerHandler, timeout?: number, ...args: unknown[]) => {
    const id = native.setTimeout(handler, timeout, ...args)
    run.timeouts.add(id)
    return id
  }) as typeof window.setTimeout

  window.clearInterval = ((id?: number) => {
    if (id != null) run.intervals.delete(id)
    native.clearInterval(id)
  }) as typeof window.clearInterval

  window.clearTimeout = ((id?: number) => {
    if (id != null) run.timeouts.delete(id)
    native.clearTimeout(id)
  }) as typeof window.clearTimeout

  window.requestAnimationFrame = ((cb: FrameRequestCallback) => {
    const id = native.requestAnimationFrame(cb)
    run.rafs.add(id)
    return id
  }) as typeof window.requestAnimationFrame

  window.cancelAnimationFrame = ((id: number) => {
    run.rafs.delete(id)
    native.cancelAnimationFrame(id)
  }) as typeof window.cancelAnimationFrame
}

function restoreSandbox(): void {
  Object.assign(window, native)
}

function stop(): void {
  if (!current) return
  const run = current
  current = null

  for (const id of run.intervals) native.clearInterval(id)
  for (const id of run.timeouts) native.clearTimeout(id)
  for (const id of run.rafs) native.cancelAnimationFrame(id)

  // Before abort().
  void device
    .DisplayClear({ application_name: run.appId })
    .catch((err) =>
      write(
        `${run.name}: failed to clear screen — ${err instanceof Error ? err.message : String(err)}`,
      ),
    )

  run.abort.abort()
  run.restoreTime()
  restoreSandbox()

  write(`■ ${run.name}: stopped.`)
  render()
}

async function start(app: { name: string; path: string; appId: string }): Promise<void> {
  if (current) stop()

  const run: Running = {
    name: app.name,
    appId: app.appId,
    intervals: new Set(),
    timeouts: new Set(),
    rafs: new Set(),
    abort: new AbortController(),
    // Before run().
    restoreTime: installFakeTime(),
  }
  current = run
  installSandbox(run)
  if (fakeTimeActive()) write(`${app.name}: time overridden`)
  render()

  write(`▶ ${app.name}: starting…`)
  try {
    // Before run().
    installSettingsApi(app.name, descriptorOf(app.name))

    // Before run().
    const uploaded = await uploadResources(app.name, app.appId, write)
    if (uploaded > 0) write(`${app.name}: uploaded ${uploaded} resource(s)`)

    const mod = await modules[app.path]()
    if (typeof mod.default !== 'function') {
      throw new Error('module must export `export default function run()`')
    }
    // Apps may read the abort signal if they want it.
    ;(globalThis as { __appSignal?: AbortSignal }).__appSignal = run.abort.signal
    mod.default()
    write(`${app.name}: run() done.`)
  } catch (err) {
    write(`${app.name}: error — ${err instanceof Error ? err.message : String(err)}`)
    console.error(err)
    stop()
  }
}

// ── UI ─────────────────────────────────────────────────────────────────────

const hub = document.getElementById('hub')!
hub.innerHTML = `
  <h1>BUSY Apps — dev</h1>
  <p>Device: <code>${import.meta.env.VITE_BUSY_ADDR ?? '10.0.4.20'}</code></p>
  <p style="margin:.5rem 0">
    <label>Time: <input id="faketime" type="text" placeholder="now"
      style="padding:.3rem;font-size:.9rem;width:7rem"></label>
    <span style="font-size:.8rem;opacity:.7">HH:MM[:SS] — override start time; empty — real time</span>
  </p>
  <div id="apps"></div>
  <pre id="log" style="margin-top:1rem;padding:.75rem;background:#111;color:#0f0;min-height:6rem;white-space:pre-wrap;overflow:auto"></pre>
`

const list = document.getElementById('apps')!
const log = document.getElementById('log')!

// Time override survives a page reload.
const faketime = document.getElementById('faketime') as HTMLInputElement
faketime.value = localStorage.getItem('busy-dev-faketime') ?? ''
setFakeTime(faketime.value)
faketime.addEventListener('change', () => {
  if (!setFakeTime(faketime.value)) {
    write(`Could not parse time "${faketime.value}" — expected HH:MM or HH:MM:SS`)
    return
  }
  localStorage.setItem('busy-dev-faketime', faketime.value)
  write(
    faketime.value.trim()
      ? `Time overridden to ${faketime.value} — restart the app`
      : 'Real time — restart the app',
  )
})

function write(msg: string): void {
  log.textContent += `${new Date().toLocaleTimeString()}  ${msg}\n`
  log.scrollTop = log.scrollHeight
}

// Mirror app errors into the log, not just the browser console.
const nativeError = console.error.bind(console)
console.error = (...args: unknown[]) => {
  nativeError(...args)
  if (current) {
    write(
      `⚠ ${args.map((a) => (a instanceof Error ? a.message : String(a))).join(' ')}`,
    )
  }
}

window.addEventListener('unhandledrejection', (event) => {
  if (!current) return
  const reason = event.reason
  write(`⚠ ${reason instanceof Error ? reason.message : String(reason)}`)
})

function render(): void {
  list.innerHTML = ''
  if (apps.length === 0) {
    list.textContent = 'No apps found (<name>/main.ts).'
    return
  }
  for (const app of apps) {
    const running = current?.name === app.name

    const card = document.createElement('div')
    card.style.cssText = 'margin:.5rem 0;padding:.5rem;border:1px solid #ddd;border-radius:6px'

    const btn = document.createElement('button')
    btn.textContent = running ? `■ ${app.name}` : `▶ ${app.name}`
    btn.style.cssText =
      `padding:.5rem 1rem;font-size:1rem;cursor:pointer;` +
      (running ? 'background:#c0392b;color:#fff' : '')
    btn.addEventListener('click', () => (running ? stop() : void start(app)))
    card.appendChild(btn)

    // Built from the app's appmeta/settings.json.
    const descriptor = descriptorOf(app.name)
    if (descriptor) {
      const toggle = document.createElement('button')
      toggle.textContent = '⚙ Settings'
      toggle.style.cssText = 'margin-left:.4rem;padding:.5rem .8rem;font-size:.9rem;cursor:pointer'

      const panel = document.createElement('div')
      panel.style.display = openSettings.has(app.name) ? 'block' : 'none'

      toggle.addEventListener('click', () => {
        const open = panel.style.display === 'none'
        panel.style.display = open ? 'block' : 'none'
        if (open) openSettings.add(app.name)
        else openSettings.delete(app.name)
      })

      renderSettings(panel, app.name, descriptor, () => {
        // Restart to apply the change.
        if (current?.name === app.name) {
          write(`${app.name}: settings changed — restarting`)
          // Deferred: stop()/start() destroy the input being handled now.
          native.setTimeout(() => {
            stop()
            start(app)
          }, 0)
        } else {
          write(`${app.name}: settings saved`)
        }
      })

      card.append(toggle, panel)
    }

    list.appendChild(card)
  }
}

render()
