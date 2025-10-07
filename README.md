# LeanOpenQNX

A **lean, modern fork** of the vocho/openqnx repository (on GitHub since Aug. 2009) 
optimized for **Raspberry Pi** and **x86/64** platforms using the
BUSY build system.

The original vocho/openqnx repository is the result of the MONARTIS (MONitoring Application for Real-Time Industrial Systems)
project conducted in 2009 at [HEIG-VD](https://heig-vd.ch/), Switzerland. The \_NTO\_VERSION define in
<sys/neutrino.h> indicates that it was based on QNX version 6.4.1.

NOTE that this project is in an early stage and work in progress for some time to come.

## What Makes It "Lean"?

- 🎯 **Focused scope**: Raspberry Pi (ARM) and PC (x86) only
- 🔨 **Lean build system**: BUSY & TCC based cross-compilation
- 📚 **Educational focus**: Streamlined for learning microkernel and RTOS concepts
- ⚡ **Faster builds**: reduction in compile time
- 🧹 **Removed complexity**: Stripped unnecessary drivers and platforms, simplified source tree

## Relationship to OpenQNX

This project is a derivative work of QNX® Neutrino RTOS source code 
made available by QSS in 2009 under the openQNX initiative. 

No QNX file is modified by this project, no QNX code is copied.

**Legal status**: Educational and non-commercial use only, consistent 
with the original program's stated objectives.

**Original announcement**: https://www.openqnx.com/node/471

## Planned Work

- New build system based on TCC and BUSY replacing legacy QNX Momentics workflow
- Cross-compilation support for ARM (Raspberry Pi 3/4/5)
- Cross-compilation support for x86/64
- Removed: PowerPC, MIPS, SH-4, ARM variants, and other QNX files not required
- Added: new files are added to the pre-existing repository and clearly identified

## Building

TBD, see LeanQt


## Educational Goals

This fork aims to make QNX Neutrino's excellent microkernel architecture 
accessible to:
- Computer science students learning RTOS design
- Hobbyists experimenting with Raspberry Pi and PC
- Developers wanting to understand microkernel concepts
- Anyone interested in real-time operating systems

## License & Copyright

**Original QNX Code**: Copyright © QNX Software Systems Limited  
**Build System & Additions**: Copyright © 2025 Rochus Keller

QNX® is a registered trademark of BlackBerry Limited.  
This project is not affiliated with or endorsed by BlackBerry/QNX.

Educational and non-commercial use only. See original OpenQNX NCEULA 
for detailed terms.

## Credits

- QNX Software Systems for making the source code available (2009)
- The HEIG-VD MONARTIS project for maintaining the vocho/openqnx repository since 2009


