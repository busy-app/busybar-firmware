@page doxguide Docs HowTo

# Introduction

The @bsb project uses [Doxygen](https://doxygen.nl) as its main means of creating and maintaining the documentation.

For detailed info on using *Doxygen*, please refer to the [official manual](https://www.doxygen.nl/manual/).

Below are some rules, conventions and tips and tricks relevant to the present documentation.

# Building

In order to build the documentation from the source, run: `./fbt doxygen` from the project root directory.

The rendered HTML docs will be placed in the `documentation/doxygen/build` directory.

To open the generated documentation in the default browser, run `./fbt doxy` from the project root directory.

In case of any problems with the *Doxygen* markup (e.g. missing function parameters, mismatching labels, etc.) errors will be printed during the build process.

# General considerations

When writing the free-form text, it is advised to follow these instructions:
- The writing style must be neutral, without excessive jargon, jokes and overall wittiness.
- The wording must be simple, concise and straight to the point.
- If using an LLM, proof-read its output and make sure it is not overly verbose.
- Prefer impersonal forms instead of first and second person, e.g.:
    - *"It is required ..."* instead of *"You need ..."*
    - *"It was implemented ..."* instead of *"I made ..."*
- If an imperative form is needed, omit the pronouns, e.g.:
    - *"Do X"* instead of *"You need to do X"*
- Avoid using controversial terminology, if this does not reduce clarity.

# File structure

## Source files

The main way of documenting the source code in *Doxygen* is to place [specially formatted](https://www.doxygen.nl/manual/docblocks.html) comment blocks inside the header and source files.

For general guidelines on *Doxygen* style, please see [Zephyr docs style guide](https://docs.zephyrproject.org/latest/contribute/style/doxygen.html) as a good example.

In short:
- Use `@command` instead of `\command` syntax
- Use a `@file` [command](https://www.doxygen.nl/manual/commands.html#cmdfile) at the top of each file to be processed by *Doxygen*
- Use function [parameter](https://www.doxygen.nl/manual/commands.html#cmdparam) directions (`[in]`, `[out]` or `[in,out]`)
- Use [groups](https://www.doxygen.nl/manual/commands.html#cmdaddtogroup) to link related entities togeter (they will show up in the [topics](topics.html) section)
- Use group nesting in order to provide additional structure to the topics section
- Do NOT simply repeat the function signature in *Doxygen* blocks, provide some actual information instead

## Standalone files

Standalone files are [custom](https://www.doxygen.nl/manual/additional.html) `.dox.md` files which are found in the `documentation` folder in the project root directory.

Their main purpose is to provide top-level structure for the documentation.

According to *Doxygen* philosophy, all documentation must be as close to the source code as possible, so there will rarely be a need of creating a new standalone file.
Instead, one should try to keep all code-related docs inside the respective source files and only reference them in standalone files as needed.

Each standalone file must contan a single *Doxygen* [page](https://www.doxygen.nl/manual/commands.html#cmdpage). It may also include other files as [subpages](https://www.doxygen.nl/manual/commands.html#cmdsubpage) or [reference](https://www.doxygen.nl/manual/commands.html#cmdref) any other label or symbol in the project.

Page labels must be short, single or several run-together words in lower case. This is due to the following:
- Rendered HTML pages get their file names from labels, so it is preferable to keep them short;
- The number of pages will always be limited, so there is virtually no possibility of running out of short names;
- Documented source code symbols are extremely unlikely to have the same naming patterns, which reduces the chance of a reference collision.

## Description files

Description files are [Markdown](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax) files that are strategically placed inside some of the project folders.

Their main purpose is to aid browsing the project on platforms like *GitHub* by **describing** the current directory the reader is at. Thus, they are usually called `README.md` in order to get the aforementioned sites to render and show them automatically upon entering a directory.

# Sidebar

The sidebar is a convenient and obvious place for newcomers to start, so its number of items should be kept to absolute minimum.

Since any standalone file will appear in the sidebar as a separate entry, it usually should be referenced as a [subpage](https://www.doxygen.nl/manual/commands.html#cmdsubpage) in another file.
