---
applyTo: "lib/**,fbt_layers/**,assets/proto/**"
---

# Third-party and submodule review rules

- A submodule pointer change (gitlink) produces no file diff. Do not infer what changed inside it: ask for a link to the upstream pull request or commit range, and check that the bump is justified in the PR description.
- Focus findings on our own code around the dependency: glue and `*_ext` extensions under `applications/`, and build wiring in `application.fam`, `targets/` and `scripts/`.
- Do not suggest broad refactors, reformatting or renames in third-party trees.
