# pi-protocol

Minimalist message protocol in C to exchange messages between MCU's. Includes a 
python generation utility for the header and source files.

## Description

### The protocol

A frame consists of:
```
| STX | MSG_ID | PAYLOAD_BYTE_1 | PAYLOAD_BYTE_2 | ... | PAYLOAD_BYTE_N | CHECKSUM |
```

`STX` bytes in `MSG_ID` and `PAYLOAD` are escaped by the sequence `ESC STX_ESC` and `ESC` bytes are escaped with `ESC ESC_ESC`.
This is simple and very robust to parse, but causes worst case overhead of `100%`, if every payload byte is either `STX` or `ESC`.

### Working principle of the generation of the protocol

- The `.yaml` files inside `./msgs/` define message fields and the filename defines the message name. The filename (and, as a result, the message name) should be formatted as all-caps-snake-case, like `ALL_CAPS_SNAKE_CASE.yaml`.
- A configuration `.yaml` file in the root of this repo defines which messages should be included and if they are to be received, sent or both (`RX`, `TX` or `RXTX`). Additionally, the `global_mode` option can be set to `RX` or `TX` instead of `RXTX` if it is known that all messages are `RX` or `TX`-only. In that case, the mode flags of each message have no effect.
- Python generates `c`-headers form `jinja2`-templates and define the message serialization ("packing") and parsing for receiving (packing for serial sending, parsing for serial reading).
- The resulting headers depend only on some standard libraries: `string.h`, `stdint.h` and `stdbool.h`. For testing and debugging output also `stdio.h`, of course.


## Generation and Usage

### Pre-requesites

[![Python 3.10](https://img.shields.io/badge/Python-3.10-3776AB?logo=python)](https://www.python.org/downloads/release/python-3100/)  

Tested on Ubuntu 22.04. Install `make`, `python3`, `pip` and then the required python modules for generation
```bash
sudo apt install make python3 python3-pip
pip install -r python/requirements.txt
```

### Generation of C files

Edit the `yaml` files to your liking, they should be self-explanatory. You can copy the `config.yaml` and give it a different name. Then generate the headers:

```bash
make clean && make CONFIG=config.yaml
```

### Usage in your project

Look at `tester.c`. Especially note how the `dummyWriter()` is passed to `piSerialWrite()`. For `TX`, this is the function that needs to be modified/replaced to actually get the bytes on the transmission line, byte-for-byte.

### Tests

You need `gcc` installed, then do

```bash
make clean && make CONFIG=config.yaml tester && ./tester
```

## History

* 2024-08-27 -- Version 2.0.0 -- Initial release


## Authors

* Till Blaha ([@tblaha](https://github.com/tblaha), Delft University of Technology, t.m.blaha - @ - tudelft.nl)


## Structure 

```shell
.
├── msgs/          # Message definitions
├── python/        # generation utility
├── src/           # static source files (only protocol implementation)
├── templates/     # source files to be generated
├── tests/
├── config.yaml    # configurations of which messages should be generated
├── LICENSE
├── Makefile
├── README.md
```


## License

[![License](https://img.shields.io/badge/License-LGPL--3.0--or--later-4398cc.svg?logo=spdx)](https://spdx.org/licenses/LGPL-3.0-or-later.html)
[![License](https://img.shields.io/badge/License-MIT-4398cc.svg?logo=spdx)](https://spdx.org/licenses/MIT.html)

The contents of this repository are licensed under a **GNU Lesser General Public License v3.0**, with the exception of the contents of `src/`. The contents of `src/`, as well as the generated output files are licensed under an **MIT License**. See the `LICENSE` file.

Technische Universiteit Delft hereby disclaims all copyright interest in the
program “Indiflight Support” (one line description of the content or function)
written by the Author(s).

Henri Werij, Dean of the Faculty of Aerospace Engineering

Copyright © 2024, Till Blaha


## Citation

