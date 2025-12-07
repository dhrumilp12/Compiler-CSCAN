# Compiler-CSCAN

A C-based scanner component for a compiler course/project, with build rules, preprocessing utilities, example inputs, and a reference executable for validation. The repository includes a PDF report, source code, headers, and artifacts under the `cscan/` directory.

Languages: C (95%), Makefile (4.3%), Shell (0.7)

## Table of Contents
- Overview
- Repository layout
- Setup and build
- Run
- Preprocessing inputs
- Validation against reference
- Development notes
- License

## Overview

This project implements a lexical scanner named `scan` for a simple language. You can build the scanner from sources, preprocess raw inputs, and compare your scanner’s output against a provided reference binary. The report [cscan.pdf](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan.pdf) summarizes the design and results.

## Repository layout

Top-level:
- [.gitattributes](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/.gitattributes) — Git attributes.
- [cscan.pdf](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan.pdf) — Project report.

Project directory: [cscan/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan)
- Build and sources:
  - [makefile](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan/makefile) — Build rules for the scanner.
  - [src/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/src) — C source files for the scanner (implementation).
  - [include/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/include) — Header files for the scanner.
  - [build/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/build) — Build outputs/intermediates (created by make).
- Executables and libraries:
  - [scan](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan/scan) — Built scanner binary (artifact).
  - [ref_scan](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan/ref_scan) — Reference scanner binary for validation.
  - [ref_lib/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/ref_lib) — Reference libraries (if required by `ref_scan`).
- Inputs and preprocessing:
  - [inputs/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/inputs) — Example/program inputs for scanning.
  - [pp/](https://github.com/dhrumilp12/Compiler-CSCAN/tree/main/cscan/pp) — Preprocessed inputs/output artifacts (destination).
  - [pp_inputs.sh](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan/pp_inputs.sh) — Shell script to preprocess inputs.

## Setup and build

Requirements:
- GCC or Clang (C compiler)
- Make
- A POSIX shell (for `pp_inputs.sh`)

Build the scanner:
```sh
git clone https://github.com/dhrumilp12/Compiler-CSCAN.git
cd Compiler-CSCAN/cscan
make
```

This should produce the `scan` executable and may populate `build/` with intermediates.

If you modify `src/` or `include/`, re-run `make`. Inspect `makefile` for available targets (e.g., `clean`, `all`).

## Run

Run the scanner on an input file:
```sh
./scan < inputs/example.txt > output.tokens
```

If the scanner expects command-line flags or specific formats, check the sources in `src/` for usage details. Many scanners read input from `stdin` and write tokens or diagnostics to `stdout`.

## Preprocessing inputs

Use the provided script to preprocess the raw inputs (e.g., stripping comments, normalizing whitespace) before scanning:
```sh
bash pp_inputs.sh
```

By default, the script will read from `inputs/` and write processed files into `pp/`. Review the script to confirm paths and processing steps.

## Validation against reference

Compare your scanner’s output against the reference implementation:
```sh
# Your scanner
./scan < inputs/example.txt > my_output.tokens

# Reference scanner
./ref_scan < inputs/example.txt > ref_output.tokens

# Diff outputs
diff -u ref_output.tokens my_output.tokens
```

If differences appear, investigate:
- Input preprocessing (run `pp_inputs.sh` if needed)
- Token formatting (whitespace, delimiters)
- Compiler flags or environment differences

If `ref_scan` depends on libraries in `ref_lib/`, ensure they’re available on your system (or run it directly from the repository).

## Development notes

- Core logic lives under `src/` with interfaces in `include/`. Extend token kinds, regular expressions/DFAs, and error handling there.
- Consider adding:
  - A usage message (`--help`)
  - Structured output (CSV/JSON) for downstream tooling
  - Unit tests for tokenization edge cases (empty input, Unicode, numeric literals, strings)
- Keep large generated artifacts out of version control; use `build/` for intermediates and `pp/` for preprocessed files.

## License

No explicit license file is present. If you plan to share or extend this work, consider adding a LICENSE (e.g., MIT, Apache-2.0, GPL-3.0) to clarify usage terms.

---
For background and details, see [cscan.pdf](https://github.com/dhrumilp12/Compiler-CSCAN/blob/main/cscan.pdf).
