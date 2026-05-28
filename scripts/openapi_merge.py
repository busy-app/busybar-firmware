#!/usr/bin/env python3

import sys
from pathlib import Path

def error(txt):
    print(txt, file=sys.stderr)
    exit(1)

if len(sys.argv) < 2:
    error(f"Usage: {sys.argv[0]} destination source source source...")

dst_file = sys.argv[1]
src_files = sys.argv[2:]

TEMPLATE_FILE_NAME = "openapi.yaml"
template_files = []
segment_files = []

for path in src_files:
    basename = Path(path).name
    extension = Path(path).suffix

    if extension != ".yaml":
        error(f"All source files must have the `.yaml` extension.\nThis one didn't:\n  {path}")

    if basename == TEMPLATE_FILE_NAME:
        template_files.append(path)
    else:
        segment_files.append(path)

if len(template_files) > 1:
    names_for_printing = "\n".join(f"  - {path}" for path in template_files)
    error(f"There must only be one template file.\nThe template file is distinguished by its `{TEMPLATE_FILE_NAME}` name.\nThe following sources files have this name:\n{names_for_printing}")

if not template_files:
    error(f"There must be a template file.\nThe template file is distinguished by its `{TEMPLATE_FILE_NAME}` name.\nNone of the provided source files had this name.")

template_file = template_files[0]
substitutions = {}

for path in segment_files:
    with open(path, "r") as file:
        lines = file.readlines()
        current_section = ""

        for line in lines:
            line = line.strip("\r\n")
            if line and not line.startswith(" "):
                current_section = line
                if current_section not in substitutions:
                    substitutions[current_section] = []
                continue

            substitutions[current_section].append(line)

SUBSTITUTION_DESCRIPTORS = [
    { "section": "paths:", "placeholder": "# PATHS_CONTENT_PLACEHOLDER #", "indentation_change": 0 },
    { "section": "schemas:", "placeholder": "# SCHEMAS_CONTENT_PLACEHOLDER #", "indentation_change": +2 },
]

with open(template_file, "r") as file:
    template = file.read()

def format_line(string, indentation_change):
    contents = string.lstrip(" ")
    original_indentation = len(string) - len(contents)
    new_indentation = " " * (original_indentation + indentation_change)
    return f"\n{new_indentation}{contents}"

for descriptor in SUBSTITUTION_DESCRIPTORS:
    lines = substitutions[descriptor["section"]]
    lines = [format_line(line, descriptor["indentation_change"]) for line in lines]
    text = "".join(lines)
    template = template.replace(descriptor["placeholder"], text)

with open(dst_file, "w") as file:
    file.write(template)
