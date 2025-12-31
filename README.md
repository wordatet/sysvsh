# sysvsh: Classic SVR4 Bourne Shell for Modern Unix

`sysvsh` is a robust, 64-bit port of the legendary **System V Release 4 (SVR4) Bourne Shell** to modern Linux and Unix-like operating systems. It aims to provide a pure, authentic SVR4 experience while taking full advantage of modern hardware and kernel capabilities.

This project is intended for **Research and Educational purposes**, preserving a vital piece of computing history through active maintenance and modernization.

---

## 🚀 Key Features & Modernizations

- **64-bit Native Stability**: Completely audited for pointer truncation and 64-bit safety. No more legacy `sbrk` hacks or memory corruption.
- **Bulletproof I/O path**: Deeply refactored heredoc, pipe, and loop synchronization logic ensures reliability on modern multi-core systems.
- **Dynamic Signal Engine**: Replaced static 1990s-era signal arrays with a dynamic mapping system that respects the 64+ signals of modern kernels.
- **"Vintage Mode" Support**: Compiles out-of-the-box with both `gcc` (modern) and the **Portable C Compiler (`pcc`)** for an authentic 90s build experience.
- **Paranoid Mode Testing**: Subjected to an intense stress suite involving recursive subshells, deep pipe chains, and signal churn to ensure "bulletproof" operation.

---

## 🛠️ Building and Testing

### Quick Start (Modern GCC)
To build with modern optimizations:
```bash
./configure
make
```

### Advanced Options
The `configure` script supports standard cross-compilation and installation options:
```bash
./configure --prefix=/usr/local --host=aarch64-linux-gnu
make && make install
```

### Android/Termux Notice
Native compilation is fully supported in Termux.
- **Prerequisite**: `pkg install gcc-13` (Required for K&R C compatibility).
- **Build**: `./configure && make`

### Vintage Mode (PCC)
To build using the Portable C Compiler (recreating the classic development environment):
```bash
./configure --cc=pcc
make
# OR use the vintage target directly
make vintage
```

### Verification
Run the stability suite to verify the port on your architecture:
```bash
cd testz
./verify_port.sh
./paranoid_stress.sh
```

---

## 📂 Repository Structure

| File/Dir | Description |
| :--- | :--- |
| `configure` | Portable POSIX sh script for building on various platforms |
| `main.c`, `args.c` | Shell initialization and argument parsing |
| `macro.c`, `expand.c` | Variable substitution, expansion, and "MARK" bit logic |
| `xec.c`, `service.c` | Command execution, forking, and pipe management |
| `jobs.c` | Job control and process group management |
| `fault.c` | Signal handling and the Dynamic Signal Engine |
| `stak.c`, `blok.c` | Custom shell stack and memory allocation logic |
| `io.c`, `word.c` | Input/Output routines and character processing |
| `mac.h`, `defs.h` | Core shell definitions and portability shims |
| `testz/` | "Paranoid Mode" stress tests and verification scripts |
| `Makefile` | **(Generated)** Build system for GCC/PCC |

---

## ⚖️ License

The modifications, shims, and modernization layer of this port are provided under the **BSD 3-Clause License**. See the [LICENSE](LICENSE) file for the full text.

## 🏛️ Heritage & Disclaimer

*This project is for private research and educational use only.*

*This project is for private research and educational use only. It is a derivative of historical source code and is not intended for commercial distribution. Users should respect the original licensing terms of the base SVR4 sources.*

---
*Built with ❤️ for the past, running on the ⚡ of the future.*
