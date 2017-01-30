# Brainfuck-Compiler
A simple Brainfuck compiler that targets C, MIPS, and i386.

## Compiling the source
```
make
```

File extension doesn't matter, any filename is fine.

## C
```
./brainfuck filename.bf -c
or
./brainfuck filename.bf
```

#### Executable
```
gcc out.c -o filename
```

## i386
```
./brainfuck filename.bf -i386
```
#### Executable
```
nasm out.asm -f elf
gcc out.o -o filename -m32
```

## MIPS
```
./brainfuck filename.bf -mips
```

[MARS](http://courses.missouristate.edu/kenvollmar/mars/download.htm) simulator was used to run the code.
