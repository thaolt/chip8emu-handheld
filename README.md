# chip8emu-handheld

## Quick start

Download (Prebuilt GNU ARM Embedded Toolchain)[https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm]

Please remember to change `/path/to/gnu-arm-embed-toolchain/bin` to directory path your machine where the extracted toolchain located.

```
git clone --recursive https://github.com/thaolt/chip8emu-handheld.git
cd chip8emu-handheld/chip8emu-riot
export PATH=/path/to/gnu-arm-embed-toolchain/bin:$PATH
make
```
