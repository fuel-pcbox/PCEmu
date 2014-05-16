PCEmu is owned by nobody. It is public domain.

[![Stories in Ready](https://badge.waffle.io/alegend45/pcemu.png?label=ready&title=Ready)](http://waffle.io/alegend45/pcemu)

TODO
====

[ ] 808x emulation

USAGE
=====

Once PCEmu is compiled, you need to make a configuration file.

Once this file is made, run PCEmu like this, assuming that the file is
called default.cfg:

Windows:

pcemu default.cfg > log.txt

Unix:

./pcemu default.cfg > log.txt

log.txt will contain debug information. Use it wisely.

The syntax of the configuration file is as follows:

property = value;

Whitespace is ignored, of course.

The properties are as follows:

bios: Value is the path to the BIOS file.
