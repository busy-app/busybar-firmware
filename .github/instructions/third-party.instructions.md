---
applyTo: "lib/**,fbt_layers/**,assets/proto/**"
---

# Third-party and submodule review rules

 - A submodule pointer change (gitlink) produces no file diff. Do not infer what changed inside it: ask for a link to the upstream pull request or commit range, and check that the bump is justified in the PR description.
 - For third-party/submodule changes, focus findings on our dependency integration, including glue and `*_ext` extensions under `applications/` and `lib/`, and build wiring in `application.fam`, `targets/` and `scripts/`. Review first-party libraries under `lib/` (such as `setting_provider` and `toolbox`) normally; they are not third-party code.
 - Do not suggest broad refactors, reformatting or renames in third-party trees.
