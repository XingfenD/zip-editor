# zip_editor

## Introduction

A tool to analyze and edit ZIP files. The tool can parse the ZIP file and provides both interactive editing mode (default) and direct printing mode. It can display and modify LFHs, CDHs and EOCDR sections of ZIP files.

## Compilation

```bash
git clone https://github.com/XingfenD/zip-editor.git
cd zip-editor
make
```

## Usage

```bash
./zip_editor.out -f <zip_file> [-p] [-m <mode>]
```

- `-f, --file <zip_file>`: Specify the ZIP file to analyze.
- `-p, --print`: Print the parsed results directly. Without this option, the tool enters interactive edit mode by default.
- `-m, --mode <mode>`: Specify the parsing mode. Valid values are "standard" (default) and "stream". This option is only valid when using -p.
- `-h, --help`: Print help information.

## Status

- [x] Add support for editing(hack) ZIP files.
  - [x] Edit entry and exit functionality.
  - [x] Interactive editing mode (default).
  - [x] Direct printing mode (-p option).

## Other Infomation

Detailed information about parsing mode can be found in [my study notes](https://github.com/XingfenD/zip-diff-notes.git).

Some demo ZIP files are provided in `zip_demos` folder.

### Third-Party Libraries

- cxxopts.hpp: A command line option parsing library licensed under the MIT License.
  - Author: Jarryd Beck
  - License: MIT License
  - Version: 3.3.1
  - Repository: [cxxopts — Lightweight C++ option parser](https://github.com/jarro2783/cxxopts)
- ncurses: A library for creating text-based user interfaces.
  - Author: Zeyd Ben-Halim, Pavel Curtis, Juergen Pfeifer, Thomas E. Dickey and others
  - License: [MIT-X11 License](https://invisible-island.net/ncurses/ncurses-license.html)
  - Version: 5.4
  - Official Website: [ncurses](https://www.gnu.org/software/ncurses/)
