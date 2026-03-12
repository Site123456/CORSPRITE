<p align="center">
  <img src="/logo/logo_banner_beta.png" width="300" style="margin-right:20px;" />
</p>
<img src="/images/Showcaseof2026-03-07.png" width="400" />
Compatible with <b>Ollama api</b> or a <b>custom Corsprite engine</b> (custom engine is possible but not recommended until Beta) and compatible with most modern <b>text-based models</b>.<br>
  <strong>Current builds target Windows x64.</strong><br><br>
<p align="center">
  <a href="https://docs.corsprite.com/"><img src="https://img.shields.io/badge/Docs-Beta-blue?style=for-the-badge"></a>
  <img src="https://img.shields.io/badge/Status-Alpha-orange?style=for-the-badge">
</p>

---

## 🚀 Features at a Glance

- **Local-first AI execution** — Keep all your data on your machine.  
- **Ollama integration** — Works with most text-based models (recommended: deepseek-v3:671b or better).  
- **GPU-accelerated rendering** — Smooth UI, minimal latency.  
- **Full Unicode & multi-language support** — Used: [Noto Fonts](https://github.com/notofonts/noto-fonts).  
- **Customizable interface** — Minimum window: 620×700 px.  
- **Automatic audio device detection** — Voice input ready out-of-the-box.  
- **Modular architecture** — Easy to extend or add custom scripts.  

---

## 🖥 Supported Platforms

| Status | Platform |
|--------|---------|
| ✅ Tested | Windows 11 (x64) |
| ⚙️ Planned | Linux (x64), Windows (x86 / 32-bit) |

> ⚠️ Training datasets will **not** be distributed.  
> ⚠️ Upload features remain disabled until security validation is complete.

---

## 🧱 Architecture Overview

| Layer | Language | Purpose |
|-------|----------|---------|
| UI Renderer | C++ | ImGui + GPU rendering |
| Logic Layer | Python | AI orchestration & scripting |
| System Bridge | Rust | OS integration + performance |
| AI Runtime | Ollama | Local model execution & inference |

> Modular design: swap models, add scripts, or extend safely.

---

## 🧠 Usage

CORSPRITE aims to provide a **native, fast, and personal AI assistant**:

- ⚡ Minimal latency interaction  
- 🖥 Native desktop feel  
- 🔤 Full Unicode & multi-language support  
- 🎛 Highly customizable UI  
- 🌍 Compatible with most text models
- 🔌 Local-first execution with privacy in mind  

---

## 🔧 Requirements

Before building, ensure you have:

- Vulkan SDK  
- GLFW  
- Ollama (for local model execution)  
- MinGW-w64 / GCC 11+ (MSVC support planned)

Verify compiler:

```bash
g++ --version
````

---

## ⚙️ Installation Guide

### 1. Clone Repository

```bash
git clone https://github.com/Site123456/CORSPRITE.git
cd CORSPRITE
```

### 2. Build (Recommended)

```bash
make
```

Produces:

```bash
Corsprite_launcher.exe
```

### 3. Manual Build (Advanced)

```bash
g++ -Wall -I. -Iinclude -Ibackends \
src/main.cpp \
imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp \
backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp \
-lglfw3 -lopengl32 -lgdi32 -ldxgi -ld3d11 -lole32 -luuid -lavrt -lwinmm \
-o Corsprite_launcher.exe
```

* Compiles core ImGui sources
* Includes GLFW + OpenGL3 backends
* Links Windows system libraries
* Produces `Corsprite_launcher.exe`

### 4. Run Launcher

```bash
Corsprite_launcher.exe
```

> Make sure Ollama is running with at least one installed model:

```bash
ollama run llama3
```

---

## 🔊 Automatic Audio Detection

CORSPRITE automatically detects and configures audio devices:

* Detects **microphones** at launch
* Monitors **real-time device changes**
* Switches to **active/highest-priority input**
* Prevents audio dropouts
* Future-ready for **noise profiling, gain control, and routing**

**Internal workflow:**

* **Rust bridge** → Low-latency device enumeration
* **Python layer** → Voice activation & Ollama acces
* **C++ UI** → Live feedback

---

## 🔐 Security & Updates

Current improvements:

* Secure Python ↔ Rust communication
* Auto-install Python modules from C++
* Bridge stability between Rust & C++
* Audio-to-Text pipelines
* Corsprite VD system
* Sandboxing external inputs
* Command validation

> Python & Rust modules will be uploaded after stability fixes.

---

## 🤝 Contributing

Guidelines will arrive during **Beta**.

* Bug reports: [GitHub Issues](https://github.com/Site123456/CORSPRITE/issues)
* Bug reports: [Official site](https://issues.corsprite.com)

---

## 📜 License

Will be finalized before **public Beta**.

---

## ⚠️ Disclaimer

CORSPRITE is **experimental alpha software**.
Expect crashes, incomplete features, and unstable APIs.
