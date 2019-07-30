# AGC emulator
This project presents an Emulator of Apollo Guidance Computer reference to this [MIT](http://ibiblio.org/apollo/NARA-SW/E-2052.pdf) document.

It's possible use an [AGC assembler](https://github.com/djqwert/agc-assembler) to assembly easily the code.

Features:
  - AGC
    - CPU
    - MEMS
    - Exceptions (not exist in original project, useful for debugging)
    - Interruptions
  - DKSY
    - GUI
    - Interaction with AGC through keyboard
  - Extra:
    - BIOS code
    - Added main interrupts
    - Added some tasks

# Installation
To build project:

```sh
make
```
To execute project:

```sh
./agc -v
```

# Contributors
[Antonio Di Tecco](https://github.com/djqwert)<br>
[Alexander De Roberto](https://github.com/alexanderderoberto)
