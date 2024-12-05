## Samba Build System 🚀  
- Version: 1.0  
- Author(s): ZHRXXgroup  
- License: [Our Open Source License](http://src.zhrxxgroup.com/OPENSOURCE_LICENSE)

---

## About  
The `samba.h` library is a lightweight and dynamic build system designed to streamline the compilation process for C projects. Inspired by simplicity and efficiency, it includes features for setting compiler options, managing dependencies, and configuring build environments through a set of intuitive macros and functions.

---
## Documentation
Currently there is no Documentation, i think i will make in the next big update (1.1)

When one will be released the under this [URL](https://zhrxxgroup.com/samba.h/docs)

NOTE: to install samba.h to /usr/include run samba as admin with flag --install

---

## Features
- Dynamic Compiler Selection: Choose between GCC and Clang with optional `ccache` acceleration.
- Verbose Logging: Easily toggle detailed logging for debugging and monitoring builds.
- Library & Include Management: Add, remove, and manage libraries, include paths, and library paths programmatically.
- Automatic Build Mode Configuration: Set release and debug flags through simple macros.
- Rebuild Detection: Check if a rebuild is needed based on source and executable timestamps.
- Utility Functions: Includes commands for finding libraries, flags, and checking available tools.
- Customizability: Use flags, variables, and macros to tailor the build process to your needs.

---

## How to Use

1. **Include the Header**  
   Add `#include "samba.h"` to your source code to access the build system.

2. **Set Up Your Environment**  
   Define your desired macros before including the library

3. **Initialize Build Flags**  
   Configure the build system to set release or debug flags using the `initialize_build_flags()` function.

4. **Define Libraries and Includes**  
   Use functions like `define_library()`, `define_include()`, `add_flag()`, etc., to specify your build dependencies.

5. **Compile Your Code**  
   Use the `compile()` function to build your project with all the defined settings.

---

## Key Macros

| Macro                 | Description                               | Default  |  
|-----------------------|-------------------------------------------|----------|  
| `S_VERBOSE_MODE`      | Enables verbose logging                   | Disabled |  
| `S_CMP_CLANG`         | Sets Clang as the compiler                | GCC      |  
| `S_CACHE_COMPILATION` | Uses `ccache` to cache compilations       | Disabled |  
| `S_RELEASE_MODE`      | Enables release flags (`-O2`, `-DNDEBUG`) | Disabled |  
| `S_DEBUG_MODE`        | Enables debug flags (`-g`, `-O0`)         | Disabled |  

---

## Additional Information

**Tools Used**
- `pkg-config`: Automatically find libraries and flags.
- `ccache`: Accelerate recompilation by caching results.

**Rebuild Automation**  
Use the `SAMBA_GO_REBUILD_URSELF()` function to automate self-rebuilds when samba.c changes.

---

## Contribution  
We welcome contributions!
- Submit issues: issues.zhrxxgroup.com
- GitHub: https://github.com/ZHRXXgroup/samba.h

---

## License  
Free to use, modify, and share under [our Open Source License](https://src.zhrxxgroup.com/OPENSOURCE_LICENSE).

Enjoy building with Samba! 🚀  
