//===----------------------------------------------------------------------===//
// KATCH
//===----------------------------------------------------------------------===//

KATCH is a tool for automatic and comprehensive testing of code patches.
It combines symbolic execution with several novel heuristics based on
static and dynamic program analysis, and leverages existing test suites
to quickly reach the new code.

KATCH is based on the klee symbolic virtual machine.

For details, see http://srg.doc.ic.ac.uk/projects/katch/

//===----------------------------------------------------------------------===//
// Klee Symbolic Virtual Machine
//===----------------------------------------------------------------------===//

klee is a symbolic virtual machine built on top of the LLVM compiler
infrastructure. Currently, there are two primary components:

  1. The core symbolic virtual machine engine; this is responsible for
     executing LLVM bitcode modules with support for symbolic
     values. This is comprised of the code in lib/.

  2. A POSIX/Linux emulation layer oriented towards supporting uClibc,
     with additional support for making parts of the operating system
     environment symbolic.

Additionally, there is a simple library for replaying computed inputs
on native code (for closed programs). There is also a more complicated
infrastructure for replaying the inputs generated for the POSIX/Linux
emulation layer, which handles running native programs in an
environment that matches a computed test input, including setting up
files, pipes, environment variables, and passing command line
arguments.

For further information, see the webpage or docs in www/.
