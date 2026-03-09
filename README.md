# <p align="center"><img src="/logo/logo_banner_beta.png" width="480" /></p>

<p align="center">
  <strong>A local‑first interactive assistant engineered to feel native, alive, and unmistakably yours.</strong><br>
  <strong>Current builds target Windows x64.</strong>
</p>

<p align="center">
  <img src="/images/Showcaseof2026-03-07.png" width="720" />
</p>

<p align="center">
  <a href="https://docs.corsprite.com/"><img src="https://img.shields.io/badge/Docs-Beta-blue?style=for-the-badge"></a>
  <a href="https://www.youtube.com/@CORSPRITE"><img src="https://img.shields.io/badge/YouTube-Updates-red?style=for-the-badge"></a>
  <img src="https://img.shields.io/badge/Platform-Windows%2011%20x64-00adef?style=for-the-badge">
  <img src="https://img.shields.io/badge/Status-Alpha-orange?style=for-the-badge">
</p>

---

## 🖥 Supported Platforms

**Tested**
- Windows 11 (x64)

**Planned**
- Linux (x64)
- Windows (x86 / 32‑bit)

⚠️ Training datasets will **not** be distributed.  
⚠️ Upload features remain disabled until security validation is complete.

---

## 📘 Documentation

- Installation: https://corsprite.com/  
- Docs: https://docs.corsprite.com/  
- Dev updates: https://www.youtube.com/@CORSPRITE

**Built with**
- [nlohmann/json](https://github.com/nlohmann/json)
- [Dear ImGui](https://github.com/ocornut/imgui)

---

## 🧠 Vision

CORSPRITE aims to deliver a **local, fast, GPU‑accelerated assistant** with:

- native system feel  
- minimal latency  
- full Unicode support  
- customizable UI (min size: 620×700)  
- multi‑language architecture  

---

## 🧱 Architecture

| Layer | Language | Purpose |
|-------|----------|---------|
| UI Renderer | C++ | ImGui + GPU rendering |
| Logic Layer | Python | AI interaction & scripting |
| System Bridge | Rust | OS access + performance |

---

## 🔧 Requirements

Install before building:

- Vulkan SDK  
- GLFW  
- MinGW‑w64 / GCC 11+ (MSVC planned)

Check compiler:

```bash
g++ --version
```

---

## ⚙️ Installation

### 1. Clone the repository

```bash
git clone https://github.com/Site123456/CORSPRITE.git
cd CORSPRITE
```

---

### 2. Build (recommended)

```bash
make
```

This produces:

```
Corsprite_launcher.exe
```

---

### 3. Manual build (advanced)

If you prefer to compile manually without `make`, use the full command:

```bash
g++ -Wall -I. -Iinclude -Ibackends \
src/main.cpp \
imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp \
backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp \
-lglfw3 -lopengl32 -lgdi32 -ldxgi -ld3d11 -lole32 -luuid -lavrt -lwinmm \
-o Corsprite_launcher.exe
```

This command:

- Compiles all core ImGui sources  
- Includes GLFW + OpenGL3 backends  
- Links required Windows system libraries  
- Produces the final executable  

---

### 4. Run the launcher

```bash
Corsprite_launcher.exe
```

---

## 🔐 Security & Updates

Active work includes:

- securing Python ↔ Rust communication  
- fixing auto installation for python directly from c++
- bridge connection between Rust & C++
- Audio to Text
- Corsprite VD
- sandboxing external inputs  
- validating assistant commands  

Note: Python & Rust files will be uploaded only after fix
---

## 🔊 Audio Auto‑Detection

CORSPRITE includes an **automatic audio device detection system** designed to make voice interaction seamless and reliable across different hardware setups.

### What the system does
- Detects available **microphones** at launch  
- Monitors **device changes** in real time  
- Automatically switches to the **active / highest‑priority** input  
- Prevents audio dropouts  
- Ensures the assistant is always ready to listen  

### How it works internally
- Rust bridge for **low‑latency device enumeration**  
- Python layer for **voice activation**  
- C++ UI for **live visual feedback**  
- Future‑ready for noise profiling, gain control, and routing  

### Current status
- Auto‑detection is **enabled in alpha builds**  
- Manual device selection UI is in development  
- Advanced audio processing (AEC, AGC, noise suppression) is planned for beta  

---

## 🤝 Contributing

Contribution guidelines will arrive during **Beta**.  
Bug reports: https://github.com/Site123456/CORSPRITE/issues
Bug reports: https://issues.corsprite.com

---

## 📜 License

To be finalized before public beta.

---

## ⚠️ Disclaimer

CORSPRITE is **experimental alpha software**.  
Expect crashes, incomplete features, and unstable APIs.