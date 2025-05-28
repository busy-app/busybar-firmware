#!/usr/bin/env python3

import sys
import os.path
import contextlib

# FIXME
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir, "lib", "lvgl", "scripts")))

from LVGLImage import main as convert  # noqa: E402


def main():
    with contextlib.redirect_stdout(None):
        ret = convert()

    return ret if ret else 0


if __name__ == "__main__":
    main()
