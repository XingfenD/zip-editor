# zip_editor

## Introduction

A tool to edit ZIP files. The tool can parse the ZIP file in standard mode or stream mode, and print the LFHs, CDHs and EOCDR.

## Compilation

```bash
git clone https://github.com/XingfenD/zip-editor.git
cd zip-editor
make
```

## Usage

```bash
./zip_editor.out -f <zip_file> [-m <mode>] [-e]
```

- `-f, --file <zip_file>`: Specify the ZIP file to analyze.
- `-m, --mode <mode>`: Specify the parsing mode. Valid values are "standard" (default) and "stream".
- `-e, --edit`: Edit the ZIP file in interactive mode. (Not implemented yet)
- `-h, --help`: Print help information.

## Status

- [ ] Add support for editing(hack) ZIP files.
  - [x] Edit entry and exit finished.

## Other Infomation

Detailed information about parsing mode can be found in [my study notes](https://github.com/XingfenD/zip-diff-notes.git).

Some demo ZIP files are provided in `zip_demos` folder.

### Third-Party Libraries

- cxxopts.hpp: A command line option parsing library licensed under the MIT License.
  - Author: Jarryd Beck
  - License: MIT License
  - Version: 3.3.1
  - Repository: [cxxopts — Lightweight C++ option parser](https://github.com/jarro2783/cxxopts)
