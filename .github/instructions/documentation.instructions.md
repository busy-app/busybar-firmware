---
applyTo: "documentation/**"
---

# Documentation review rules

- `documentation/*.dox.md` is Doxygen-flavoured Markdown consumed by the docs build. Keep the existing heading hierarchy, Doxygen commands (`@brief`, `@note`, ...) and cross-references intact.
- A change of public behaviour or API that is described in `documentation/` must update the matching document in the same PR; `AGENTS.md` requires it. Flag PRs that change behaviour but not the document.
- Document contracts rather than implementation narration: who owns returned memory and which function frees it, how long a pointer stays valid, which parameters may be `NULL`, and what counts as success or failure.
- When a module or application is added or renamed, check that the index pages (`documentation/Index.dox.md`, `documentation/README.md`) stay accurate.
