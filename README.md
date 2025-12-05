# Nix template for 8051 projects

The Intel MCS-51 (commonly termed 8051) is a single chip microcontroller (MCU) series developed by Intel in 1980 for use in embedded systems.

This template provides a basic setup for developing 8051 projects using the SDCC (Small Device C Compiler) toolchain and the XMake build system. It includes configurations for compiling C and assembly code, linking, and optimizing for size.

## Features
- programming with C via [SDCC](https://sdcc.sourceforge.net/)
- build system with [XMake](https://xmake.io/#/)
- LSP support with [clangd](https://clangd.llvm.org/)
- emulation with [emu8051](https://github.com/cjacker/emu8051)
- support flashing

## Usage
1. Clone the repository
2. Enter the project directory
3. Run `direnv allow`if you have `direnv` installed or with `nix develop` to enter the development shell
4. Use `xmake f -c [chip]` to set the target chip (e.g., `xmake f -c STC15`)
5. Use `xmake` to build the project
6. Add `#include <clangd.h>` to  `main.c` file to support clangd LSP features and add `#include <keil.h>` to support keil style code. Or you can add `#include <utility.h>` to support both.

> [!WARNING]
> The `#include <clangd.h>` or `#include <utility.h>` must be the first line of your source file to activate clangd.

## Commands
- `xmake` - Build the project
- `xmake build` - Build the project
- `xmake format` - Format the source code

- `xmake emulator` - Run the project in the emulator
- `xmake flash` - Flash the compiled binary to the microcontroller

- `xmake clean` - Clean the build artifacts

## Flashing Tools
| platform | tool | aviliable |
| -------- | ---- | --------- |
| STC      | stcflash | yes   |

## Todo
- [ ] Support Debugging tool
- [ ] Support more flashing tools
- [ ] Support edsim51
- [ ] Support naken-asm

## Example

blink example
```C
#include <utility.h>
#include <softdelay.h>
#include <stc51.h>

int main(void) {
    for (;;) {
        P00 = !P00;
        delay500ms();
    }
}
```
# License
I am the author of every file with the following disclaimer:
```sh
# @author zjm54321
```
I license them under the [GNU GPL-3.0](./LICENSE). There is no warranty, to the extend permitted by applicable law.

A few scripts or config files come from others, and a credit to the respective authors should be spotted.

# References
[opensource-toolchain-8051](https://github.com/cjacker/opensource-toolchain-8051#for-stc8051-from-stc)