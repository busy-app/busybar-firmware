# Docs HowTo {#docs-howto}

This is a stub page on contributing to the docs.

# Introduction

The @bsb project uses [Doxygen](https://doxygen.nl) as its main means of creating and maintaining the documentation.

For detailed info on using *Doxygen*, please refer to the [official manual](https://www.doxygen.nl/manual/).

Below are some rules, conventions and tips and tricks relevant to the present documentation.

# Building

To build the documentation from the source, run: `./fbt doxygen` from the project root directory. The rendered HTML files will be placed in the `documentation/doxygen/build` directory.

To open the generated documentation in the default browser, run `./fbt doxy` from the project root directory.

If there were problems with the *Doxygen* markup (e.g. missing function parameters, mismatching labels, etc.) error messages will be printed during the build process.

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
- Use `@command` instead of `\command` syntax.
- Use a `@file` [command](https://www.doxygen.nl/manual/commands.html#cmdfile) at the top of each file to be processed by *Doxygen*.
- Provide function [parameter](https://www.doxygen.nl/manual/commands.html#cmdparam) directions (`[in]`, `[out]` or `[in,out]`).
- Use [groups](https://www.doxygen.nl/manual/commands.html#cmdaddtogroup) to link related entities togeter (they will show up in the [Topics](topics.html) section).
- Use group nesting in order to provide additional structure to the Topics section.
- Do not simply repeat the function signature in *Doxygen* blocks, instead provide some actual information.

### Grouping

Group labels should be in kebab case, e.g. `@defgroup my-awesome-group My awesome group`. Care must be taken to ensure that they do not conflict with page labels (see below).

## Standalone files

Standalone files are hybrid [Markdown](https://www.doxygen.nl/manual/markdown.html) files with [custom](https://www.doxygen.nl/manual/additional.html) `.dox.md` extension. They are found in the `documentation` folder in the project root directory and their purpose is to provide top-level structure for the documentation.

### Labeling
Each standalone file must contan a title in the form of a Markdown top-level heading with a page label, e.g: `# My awesome page {#my-awesome-page}`. Page labels must be short, single word or several words in kebab case.

### Hierarchy
Standalone files may also include other files as [subpages](https://www.doxygen.nl/manual/commands.html#cmdsubpage) or [reference](https://www.doxygen.nl/manual/commands.html#cmdref) any other label or symbol in the project.

### Scope
According to *Doxygen* philosophy, all documentation must be as close to the source code as possible, so there will rarely be a need of creating a new standalone file.
Instead, one should try to keep all code-related docs inside the respective source files and only reference them in standalone files as needed.

## Description files

Description files are regular [Markdown](https://www.doxygen.nl/manual/markdown.html) files that are strategically placed inside some of the project folders.

Their main purpose is to aid browsing the project on platforms like *GitHub* by **describing** the current directory the reader is at. Thus, they are usually called `README.md` in order to get the aforementioned sites to render and show them automatically upon entering a directory.

# Sidebar

The sidebar is a convenient and obvious place for newcomers to start, so its number of items should be kept to absolute minimum.

Since any standalone file will appear in the sidebar as a separate entry, it usually should be referenced as a [subpage](https://www.doxygen.nl/manual/commands.html#cmdsubpage) in another file.
