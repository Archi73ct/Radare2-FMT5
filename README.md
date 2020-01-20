# Radare2 plugin for the fictional arch of femtium.
This plugin disassembles and analyses the femtium 2 arch binaries.

### Features
The plugin will disassembler and colorcode the binary, as well as annotate jumps.
An attempt is also made at annotating call parameters with varying success so far.
Varius error messeges may be displayed.

### Usage
The fmt5.r2 contains some setup.
run `r2 -i fmt5.r2 [binary]`

### Building
Run `./install.sh`

### Contribution
Pull requests are more than welcome

### Shoutouts
Shoutout to @Ollypwn for cleaning up and structuring my original code (I'm sorry for messing it up again).