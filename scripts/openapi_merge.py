#!/usr/bin/env python3
#
# How to use:
#   openapi_merge.py path/to/destination.yaml path/to/template/openapi.yaml segment1.yaml segment2.yaml ...
#                    \______________________/ \___________________________________________________________/
#                          merged file                           source files to combine
#                                             \___________________________/ \_____________________________/
#                                                template (distinguished             segment files
#                                                     by basename)

from flipper.app import App
from pathlib import Path
from dataclasses import dataclass

@dataclass
class SubstitutionDescriptor:
    section: str
    placeholder: str
    indentation_change: int

EXTENSION = ".yaml"
TEMPLATE_FILE_NAME = f"openapi{EXTENSION}"
SUBSTITUTION_DESCRIPTORS = [
    SubstitutionDescriptor(section="paths:", placeholder="# PATHS_CONTENT_PLACEHOLDER #", indentation_change=0),
    SubstitutionDescriptor(section="schemas:", placeholder="# SCHEMAS_CONTENT_PLACEHOLDER #", indentation_change=+2),
]

class Main(App):
    def init(self):
        self.parser.add_argument("destination", help="Destination file")
        self.parser.add_argument("source", help="Source files (template and segments)", nargs="+")
        self.parser.set_defaults(func=self.main)

        self.args = self.parser.parse_args()

    @staticmethod
    def change_indentation(string, delta):
        contents = string.lstrip(" ")
        original_indentation = len(string) - len(contents)
        new_indentation = " " * (original_indentation + delta)
        return f"\n{new_indentation}{contents}"

    def read_segments(self, segment_files):
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

        return substitutions

    def main(self):
        args = self.args

        template_files = []
        segment_files = []

        for path in args.source:
            if Path(path).suffix != EXTENSION:
                self.logger.error(f"All source files must have the `{EXTENSION}` extension.\nThis one didn't:\n  {path}")
                return 1

            if Path(path).name == TEMPLATE_FILE_NAME:
                template_files.append(path)
            else:
                segment_files.append(path)

        if len(template_files) > 1:
            names_for_printing = "\n".join(f"  - {path}" for path in template_files)
            self.logger.error(f"There must only be one template file.\nThe template file is distinguished by its `{TEMPLATE_FILE_NAME}` name.\nThe following sources files have this name:\n{names_for_printing}")
            return 1

        if not template_files:
            self.logger.error(f"There must be a template file.\nThe template file is distinguished by its `{TEMPLATE_FILE_NAME}` name.\nNone of the provided source files had this name.")
            return 1

        template = Path(template_files[0]).read_text()
        substitutions = self.read_segments(segment_files)

        for descriptor in SUBSTITUTION_DESCRIPTORS:
            lines = substitutions[descriptor.section]
            lines = [self.change_indentation(line, descriptor.indentation_change) for line in lines]
            text = "".join(lines)
            template = template.replace(descriptor.placeholder, text)

        with open(args.destination, "w") as file:
            file.write(template)

        return 0

if __name__ == "__main__":
    Main()()
