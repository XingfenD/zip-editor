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
./zip_editor.out -f <zip_file> [-m <mode>]
```

- `-f, --file <zip_file>`: Specify the ZIP file to analyze.
- `-m, --mode <mode>`: Specify the parsing mode. Valid values are "standard" (default) and "stream".
- `-h, --help`: Print help information.

## Other Infomation

Detailed information about parsing mode can be found in `notes` folder.

Some demo ZIP files are provided in `zip_demos` folder.
