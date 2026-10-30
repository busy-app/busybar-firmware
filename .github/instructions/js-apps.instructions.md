---
applyTo: "applications_js/**"
---

# JavaScript application review rules

Read `applications_js/AGENTS.md` and `documentation/JavaScript Applications.dox.md` before commenting; they are the reference for the runtime, package layout, manifest and settings schemas, and API limits.

- JavaScript apps run on stock firmware. A PR in this tree must not modify `applications/`, `lib/`, `targets/`, `fbt_layers/` and must not require `./fbt` builds. Flag it if it does, because that would force users onto a firmware fork.
- One app, one repository: app sources are scaffolded with `busy-tools` elsewhere. Do not ask for new apps to be added here unless the PR is explicitly about the examples.
- Apps reach the device only through `fetch` on `http://localhost/api/...`, and only a subset of the HTTP API is exposed. Check that every call belongs to that subset.
- The default button press event is `short`, not `press`.
- Check that `src/appmeta/manifest.json` matches the sources and the declared API version.
