#!/usr/bin/env python3

from io import TextIOWrapper
from typing import List, Dict, Tuple
from pathlib import Path
from os import path
import re
import csv

from flipper.app import App

# "en-US" -> "EN_US"
def locale_as_snake(locale: str) -> str:
    parts = locale.split("-")
    parts = [x.upper() for x in parts]
    return "_".join(parts)

# "en-US" -> "EnUs"
def locale_as_pascal(locale: str) -> str:
    parts = locale.split("-")
    parts = [x[0].upper() + x[1:].lower() for x in parts]
    return "".join(parts)

# "foo.bar.baz" -> "FOO_BAR_BAZ"
def key_as_snake(locale: str) -> str:
    parts = locale.split(".")
    parts = [x.upper() for x in parts]
    return "_".join(parts)

# "settings", "en-US" -> "L10N_SETTINGS_EN_US_TABLE"
def table_name(app: str, locale: str) -> str:
    return f"L10N_{app.upper()}_{locale_as_snake(locale)}_TABLE"

DEFAULT_LOCALE = "en-US"

# "% 10s hello %02d world %zu" -> ["s", "d", "zu"]
def extract_placeholder_types(template: str) -> List[str]:
    raw = re.findall(r"%(.?\d{0,2}(s|d|zu))", template)
    return [t for _, t in raw]

class Main(App):
    def init(self):
        self.parser.add_argument("-f", "--format", required=True, choices=["key_header", "internal_tables", "internal_index", "external_tables"])
        self.parser.add_argument("source")
        self.parser.add_argument("target")
        self.parser.set_defaults(func=self.main)

    def validate_csv(self, rows: List[Dict[str, str]]) -> bool:
        if len(rows) == 0:
            self.logger.error(f"source must have at least one non-header row")
            return False
            
        for i, row in enumerate(rows):
            if "key" not in row:
                self.logger.error(f"row number {i} has no key")
                return False

            if DEFAULT_LOCALE not in row:
                self.logger.error(f"key {row['key']} has no value for default locale ({DEFAULT_LOCALE})")
                return False
            
        placeholders_by_locale = {}
        for locale, template in row.items():
            if locale == "key":
                continue
            placeholders = extract_placeholder_types(template)
            placeholders_by_locale[locale] = placeholders

        expected_placeholders = placeholders_by_locale[DEFAULT_LOCALE]
        for placeholders in placeholders_by_locale.values():
            if placeholders != expected_placeholders:
                self.logger.error(f"Placeholder mismatch between locales for key \"{row['key']}\": {placeholders_by_locale}")
                return False

        return True

    def gen_key_header(self, app_name: str, rows: List[Dict[str, str]], target: TextIOWrapper) -> bool:
        app_name = app_name.upper()
        target.write("// IMPORTANT: generated! do not edit\n\n")
        target.write("#include <l10n/l10n_common.h>\n\n")
        for i, row in enumerate(rows):
            en_us_for_comment = row[DEFAULT_LOCALE].replace("\n", "\\n")
            comment = f"// {DEFAULT_LOCALE}: {en_us_for_comment}\n"
            placeholders = extract_placeholder_types(row[DEFAULT_LOCALE])

            if placeholders:
                target.write(comment)
                target.write(f"#define L10N_RAW_KEY_{app_name}_{key_as_snake(row['key'])} ((L10nKey){i})\n")
                PLACEHOLDER_PREFIXES = {
                    "s": "str_",
                    "d": "int_",
                    "zu": "size_t_",
                }
                placeholders = [f"{PLACEHOLDER_PREFIXES[type]}{i}" for i, type in enumerate(placeholders)]
                macro_args = ", ".join(placeholders)
                macro_expansion = ", ".join([f"((L10nKey){i})"] + [f"({p})" for p in placeholders])
                target.write(comment)
                target.write(f"#define L10N_KEY_{app_name}_{key_as_snake(row['key'])}({macro_args}) {macro_expansion}\n")

            else:
                target.write(comment)
                target.write(f"#define L10N_KEY_{app_name}_{key_as_snake(row['key'])} ((L10nKey){i})\n")

            target.write("\n")

        return True

    def gen_internal_tables(self, app_name: str, rows: List[Dict[str, str]], target: TextIOWrapper) -> bool:
        target.write("// IMPORTANT: generated! do not edit\n\n")
        target.write("#include <l10n/l10n_generated.h>\n")
        target.write("#include <l10n/l10n_table_i.h>\n")

        app_name = app_name.upper()
        locales = list(rows[0].keys())
        locales.remove("key")
        for locale in locales:
            templates_name = f"L10N_{app_name}_{locale_as_snake(locale)}_TEMPLATES"

            target.write(f"static const char* const {templates_name}[{len(rows)}] = {{\n")
            for row in rows:
                v = row[locale].replace("\n", "\\n").replace("\"", "\\\"")
                target.write(f"    \"{v}\",\n")
            target.write("};\n")

            target.write(f"const L10nTable {table_name(app_name, locale)} = {{\n")
            target.write(f"    .entries = {templates_name},\n")
            target.write(f"    .entry_cnt = {len(rows)},\n")
            target.write("    .is_owned = false,\n")
            target.write("};\n\n")

        return True

    def gen_index(self, args) -> int:
        # FIXME:
        source = Path(args.source).parent

        entries: List[Tuple[str, str]] = []
        for path in source.iterdir():
            name = path.stem
            with open(path, "r", encoding="utf-8") as file:
                locales = list(csv.reader(file))[0]
                locales.remove("key")
                for locale in locales:
                    entries.append((name, locale))

        with open(args.target, "w", encoding="utf-8") as target:
            target.write("// IMPORTANT: generated! do not edit\n\n")
            target.write("#include <l10n/l10n_generated.h>\n")
            target.write("#include <l10n/l10n_table_i.h>\n\n")

            for name, locale in entries:
                target.write(f"extern const L10nTable {table_name(name, locale)};\n")
            target.write("\n")

            target.write(f"const L10nAppListEntry L10N_APP_LIST[{len(entries)}] = {{\n")

            for name, locale in entries:
                target.write(f"    {{\"{name}\", L10nLocale{locale_as_pascal(locale)}, &{table_name(name, locale)}}},\n")

            target.write("};\n")
            target.write(f"const size_t L10N_APP_LIST_COUNT = {len(entries)};\n")

        return True

    def gen_external_tables(self, args) -> int:
        rows = []
        with open(args.source, "r", encoding="utf-8") as source:
            reader = csv.DictReader(source)
            rows = list(reader)
        if not self.validate_csv(rows):
            return False

        locales = list(rows[0].keys())
        locales.remove("key")
        base = Path(args.target)

        base.mkdir(parents=True, exist_ok=True)

        for locale in locales:
            with open(base / locale, "w", encoding="utf-8") as target:
                for row in rows:
                    target.write(f"{row[locale]}\0")

        return True

    def gen_one_to_one(self, args) -> bool:
        rows = []
        with open(args.source, "r", encoding="utf-8") as source:
            reader = csv.DictReader(source)
            rows = list(reader)
        if not self.validate_csv(rows):
            return False

        app_name = Path(args.source).stem

        with open(args.target, "w", encoding="utf-8") as target:
            functions = {
                "key_header": self.gen_key_header,
                "internal_tables": self.gen_internal_tables,
            }
            return functions[args.format](app_name, rows, target)

    def main(self):
        args = self.parser.parse_args()
        success = None
        if args.format == "internal_index":
            success = self.gen_index(args)
        elif args.format == "external_tables":
            success = self.gen_external_tables(args)
        else:
            success = self.gen_one_to_one(args)

        return 0 if success else 128

if __name__ == "__main__":
    Main()()
