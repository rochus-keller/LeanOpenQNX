# LeanOpenQNX

A **lean, modern fork** of the vocho/openqnx repository (on GitHub since 2009) 
optimized for **Raspberry Pi** and **x86/64** platforms using the
BUSY build system.

## What Makes It "Lean"?

- 🎯 **Focused scope**: Raspberry Pi (ARM) and x86/64 only
- 🔨 **Modern build system**: BUSY based cross-compilation
- 📚 **Educational focus**: Streamlined for learning microkernel and RTOS concepts
- ⚡ **Faster builds**: reduction in compile time
- 🧹 **Removed complexity**: Stripped unnecessary drivers and platforms, reorganized source tree

## Relationship to OpenQNX

This project is a derivative work of QNX® Neutrino RTOS source code 
made available by QSS in 2009 under the openQNX initiative. 

**Legal status**: Educational and non-commercial use only, consistent 
with the original program's stated objectives.

**Original announcement**: https://www.openqnx.com/node/471

## Key Modifications

- ✅ Modern build system replacing legacy QNX Momentics workflow
- ✅ Cross-compilation support for ARM (Raspberry Pi 3/4/5)
- ✅ Cross-compilation support for x86/64
- ✅ Removed: PowerPC, MIPS, SH-4, ARM variants not used in Raspi
- ✅ Documentation: Modern guides for contemporary developers

## Building

TBD, see LeanQt


## Educational Goals

This fork aims to make QNX Neutrino's excellent microkernel architecture 
accessible to:
- Computer science students learning RTOS design
- Hobbyists experimenting with Raspberry Pi
- Developers wanting to understand microkernel concepts
- Anyone interested in real-time operating systems

## License & Copyright

**Original QNX Code**: Copyright © QNX Software Systems Limited  
**Build System & Modifications**: Copyright © 2025 Rochus Keller

QNX® is a registered trademark of BlackBerry Limited.  
This project is not affiliated with or endorsed by BlackBerry/QNX.

Educational and non-commercial use only. See original OpenQNX NCEULA 
for detailed terms.

## Credits

- QNX Software Systems for making the source code available (2007)
- The OpenQNX community for maintaining repositories since 2009


