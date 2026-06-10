#!/usr/bin/env python3
"""Merge per-category YAML segments into a single OpenAPI spec.

Usage:
  openapi_merge.py merge <dest> <template> <segment...>
  openapi_merge.py validate <spec.yaml>

Segment files contribute top‑level keys (``paths``, ``schemas``, ``tags``, …)
which are merged into the template at ``# {NAME}`` marker comments.
"""

import re
from pathlib import Path

import yaml

from flipper.app import App

EXTENSION = ".yaml"
TEMPLATE_FILE_NAME = f"openapi{EXTENSION}"

# ---------------------------------------------------------------------------
# Marker pattern: ``  # {PATHS}`` or ``      # {SCHEMAS}``.
# The section name inside ``{}`` must match an uppercase key used in the
# template and as a key in the ``_build_replacements()`` result.
# ---------------------------------------------------------------------------
_MARKER_PATTERN = re.compile(r"^(\s*)#\s*\{([A-Z_]+)\}\s*$")

# ---------------------------------------------------------------------------
# Per‑section merge strategy.
# ``dict``  – merge keys, error on duplicate.
# ``list``  – concatenate lists.
# ``value`` – replace (template value wins if also present in segment).
# Sections not listed here are *template‑only* and ignored in segments.
# ---------------------------------------------------------------------------
_MERGE_STRATEGY = {
    "paths": "dict",
    "schemas": "dict",
    "tags": "list",
    "security": "list",
}

# Mapping between marker name (uppercase) and the template YAML key it
# replaces.  "SCHEMAS" is special: it inserts under ``components/schemas``,
# not at the top level.
_MARKER_KEY_MAP = {
    "PATHS": "paths",
    "SCHEMAS": "schemas",
    "TAGS": "tags",
    "SECURITY": "security",
}

# Sections whose marker appears under ``components`` in the template.
_COMPONENT_SECTIONS = {"schemas"}


# =============================================================================
# Validation helpers
# =============================================================================


def _collect_refs(obj, refs=None):
    """Recursively collect all ``$ref`` string values in a dict/list tree."""
    if refs is None:
        refs = set()
    if isinstance(obj, dict):
        if "$ref" in obj and isinstance(obj["$ref"], str):
            refs.add(obj["$ref"])
        for v in obj.values():
            _collect_refs(v, refs)
    elif isinstance(obj, list):
        for v in obj:
            _collect_refs(v, refs)
    return refs


def _validate_refs(spec):
    """Verify that every ``$ref`` targeting ``#/components/schemas/`` resolves."""
    schemas = set(spec.get("components", {}).get("schemas", {}).keys())
    all_refs = _collect_refs(spec)

    missing = []
    for ref in sorted(all_refs):
        if not ref.startswith("#/components/schemas/"):
            continue
        name = ref[len("#/components/schemas/") :]
        if name not in schemas:
            missing.append(ref)

    if missing:
        raise ValueError(
            "Unresolved $ref targets (missing from components/schemas):\n  "
            + "\n  ".join(missing)
        )

    return len(all_refs)


def _validate_structure(spec):
    """Check the merged spec has the required OpenAPI 3.1 shape."""
    if not isinstance(spec, dict):
        raise ValueError("Merged output is not a YAML mapping")

    if spec.get("openapi") != "3.1.0":
        raise ValueError(f"Expected OpenAPI version 3.1.0, got {spec.get('openapi')!r}")

    if "paths" not in spec:
        raise ValueError("Merged output has no 'paths' section")

    components = spec.get("components", {})
    if not isinstance(components, dict) or "schemas" not in components:
        raise ValueError("Merged output has no 'components/schemas' section")

    if not isinstance(components["schemas"], dict):
        raise ValueError("'components/schemas' is not a YAML mapping")


# =============================================================================
# Merge logic
# =============================================================================


class Main(App):
    def init(self):
        sub = self.parser.add_subparsers(dest="command", help="Subcommand")

        merge_parser = sub.add_parser(
            "merge", help="Merge segment files into a single spec"
        )
        merge_parser.add_argument("destination", help="Destination file")
        merge_parser.add_argument(
            "source", help="Source files (template and segments)", nargs="+"
        )

        validate_parser = sub.add_parser(
            "validate", help="Validate a merged OpenAPI spec"
        )
        validate_parser.add_argument(
            "spec_file", help="Path to the merged openapi.yaml"
        )

        self.parser.set_defaults(func=self.main)
        self.args = self.parser.parse_args()

    # ------------------------------------------------------------------
    # Segment parsing – generic, strategy‑driven
    # ------------------------------------------------------------------

    def read_segments(self, segment_files):
        """Parse all segment files; return a dict ``{section_name: merged_data}``.

        Returns ``None`` on error (duplicate key in a ``dict`` section).
        """
        merged = {}

        for path in segment_files:
            with open(path, "r") as f:
                doc = yaml.safe_load(f)

            if not isinstance(doc, dict):
                self.logger.warning("Skipping %s: not a YAML mapping", path)
                continue

            for section, strategy in _MERGE_STRATEGY.items():
                if section not in doc or doc[section] is None:
                    continue
                data = doc[section]
                merged.setdefault(section, {} if strategy == "dict" else [])

                if strategy == "dict":
                    if not isinstance(data, dict):
                        self.logger.error(
                            "%s: '%s' must be a mapping, got %s",
                            path,
                            section,
                            type(data).__name__,
                        )
                        return None
                    for key, value in data.items():
                        if key in merged[section]:
                            self.logger.error(
                                "Duplicate %s %r in %s (already defined in another segment)",
                                section,
                                key,
                                path,
                            )
                            return None
                        merged[section][key] = value

                elif strategy == "list":
                    if not isinstance(data, list):
                        self.logger.error(
                            "%s: '%s' must be a list, got %s",
                            path,
                            section,
                            type(data).__name__,
                        )
                        return None
                    merged[section].extend(data)

        return merged

    # ------------------------------------------------------------------
    # Build the flat replacements dict for the template markers
    # ------------------------------------------------------------------

    def _build_replacements(self, merged):
        """Convert merged section dict into marker‑ready replacements.

        * ``paths``     → ``{"PATHS": {…}}``
        * ``schemas``   → ``{"SCHEMAS": {…}}``  (will be nested under components
          by the marker in the template)
        * ``tags``      → ``{"TAGS": […]}``
        """
        replacements = {}
        for marker, yaml_key in _MARKER_KEY_MAP.items():
            if yaml_key in merged:
                replacements[marker] = merged[yaml_key]
        return replacements

    # ------------------------------------------------------------------
    # Marker-based substitution
    # ------------------------------------------------------------------

    @staticmethod
    def _apply_markers(template_lines, replacements):
        """Insert replacement content at ``# {NAME}`` marker comments.

        Each marker is replaced by the YAML‑serialised value at the same
        indentation level.  The marker line itself is removed.
        """
        result = []
        for line in template_lines:
            m = _MARKER_PATTERN.match(line)
            if m:
                name = m.group(2)
                if name in replacements:
                    indent = m.group(1)
                    yaml_text = yaml.dump(
                        replacements[name],
                        default_flow_style=False,
                        sort_keys=False,
                        allow_unicode=True,
                    )
                    for content_line in yaml_text.rstrip("\n").split("\n"):
                        result.append(f"{indent}{content_line}\n")
                continue
            result.append(line)
        return result

    # ------------------------------------------------------------------
    # Entry points
    # ------------------------------------------------------------------

    def main(self):
        if self.args.command == "validate":
            return self._cmd_validate()
        return self._cmd_merge()

    @staticmethod
    def _validate_spec(logger, path):
        with open(path, "r") as f:
            spec = yaml.safe_load(f)
        try:
            _validate_structure(spec)
        except ValueError as e:
            logger.error("%s", e)
            return 1
        try:
            ref_count = _validate_refs(spec)
            schemas = spec.get("components", {}).get("schemas", {})
            tags_count = len(spec.get("tags", []))
            logger.info(
                "OK — %d paths, %d schemas, %d tags, %d $ref targets resolved",
                len(spec.get("paths", {})),
                len(schemas),
                tags_count,
                ref_count,
            )
        except ValueError as e:
            logger.error("%s", e)
            return 1
        return 0

    def _cmd_validate(self):
        return self._validate_spec(self.logger, self.args.spec_file)

    def _cmd_merge(self):
        args = self.args

        template_files = []
        segment_files = []

        for sp in args.source:
            p = Path(sp)
            if p.suffix != EXTENSION:
                self.logger.error(
                    "All source files must have the `.yaml` extension. This one didn't:\n  %s",
                    sp,
                )
                return 1
            if p.name == TEMPLATE_FILE_NAME:
                template_files.append(p)
            else:
                segment_files.append(p)

        if len(template_files) > 1:
            names = "\n".join(f"  - {p}" for p in template_files)
            self.logger.error(
                "There must only be one template file (`%s`). Found:\n%s",
                TEMPLATE_FILE_NAME,
                names,
            )
            return 1
        if not template_files:
            self.logger.error(
                "There must be a template file (`%s`). None provided.",
                TEMPLATE_FILE_NAME,
            )
            return 1

        # --- Merge segments ----------------------------------------------------
        merged = self.read_segments(segment_files)
        if merged is None:
            return 1

        self.logger.info(
            "Merged %d paths, %d schemas, %d tags from %d segment files",
            len(merged.get("paths", {})),
            len(merged.get("schemas", {})),
            len(merged.get("tags", [])),
            len(segment_files),
        )

        # --- Apply markers to template -----------------------------------------
        with open(template_files[0], "r") as f:
            template_lines = f.readlines()

        replacements = self._build_replacements(merged)
        merged_lines = self._apply_markers(template_lines, replacements)

        # --- Write merged spec -------------------------------------------------
        with open(args.destination, "w") as f:
            f.writelines(merged_lines)

        # --- Validate ----------------------------------------------------------
        if self._validate_spec(self.logger, args.destination) != 0:
            return 1

        self.logger.info("Successfully wrote merged spec to %s", args.destination)
        return 0


if __name__ == "__main__":
    Main()()
