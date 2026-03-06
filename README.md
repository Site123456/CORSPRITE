<p align="center">
  <img src="/logo/logo_banner_beta.png" alt="CORSPRITE Logo" width="480" />
</p>

<p align="center">
<strong>An interactive assistant engineered to feel native, alive, and unmistakably yours.</strong>
</p>

---

# 🚧 Alpha Status

**Current Phase:** Active Alpha Testing
// Note current published version is an older one so it may not have all features
CORSPRITE is currently undergoing intensive development focused on:

- C++ rendering stability
- Python communication layer
- Rust system bridge
- memory handling & threading
- Vulkan / OpenGL backend testing

Public builds will be released once the system achieves a **90%+ stability benchmark** across test environments.

⚠️ **Important**

- Training datasets will **NOT be publicly distributed**
- Upload features will be disabled until security validation is complete

---

# 📘 Documentation

Full documentation is available during beta:

Docs  
https://corsprite-docs.vercel.app/

Development updates  
https://www.youtube.com/@CORSPRITE

---

# 🧠 Project Vision

CORSPRITE is designed to be a **local-first interactive assistant** that feels:

- native to the system
- responsive
- visually alive
- fully customizable

Key design principles:

- Local execution
- Minimal latency
- Full Unicode support
- GPU accelerated rendering
- Cross language architecture

---

# 🧱 Architecture

CORSPRITE uses a **multi-language architecture**:

| Layer | Language | Purpose |
|------|------|------|
| UI Renderer | C++ | ImGui + GPU rendering |
| Logic Layer | Python | AI interaction & scripting |
| System Bridge | Rust | Performance + OS access |

```
User
 │
 ▼
C++ UI Layer (ImGui)
 │
 ▼
Python Assistant Logic
 │
 ▼
Rust System Bridge
 │
 ▼
Operating System
```

---

# 🖥 Supported Platforms

Currently tested on:
- Windows 11

Planned:

- Linux support

---

# 🔧 Dependencies

Before building CORSPRITE install the following:

### Vulkan SDK

https://vulkan.lunarg.com/doc/sdk/latest/windows/getting_started.html

### GLFW

https://www.glfw.org/download.html

### Compiler

Recommended:

- MinGW-w64
- GCC 11+
- MSVC (future support)

Verify installation:

```bash
g++ --version
```

---

# 📂 Repository Structure
Not finished

---

# ⚙️ Installation

Clone the repository:

```bash
git clone https://github.com/Site123456/CORSPRITE.git
```

Enter the directory:

```bash
cd CORSPRITE
```

---

# 🛠 Build & Run

Compile using **g++**:

```bash
g++ -Wall src/main.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp -I. -Iinclude -Ibackends -lglfw3 -lopengl32 -lgdi32 -o app.exe
```

Run the application:

```bash
app.exe
```

---

# 🔤 Font & Glyph Support

CORSPRITE uses:

- Windows default fonts
- Unicode compatible fonts
- ImGui font renderer

This ensures support for a wide range of scripts.

---

### Supported Character Sets

- English (uppercase / lowercase)
- digits
- punctuation
- Latin Extended A
- Latin Extended B
- IPA extensions
- Greek
- Cyrillic
- mathematical symbols
- arrows
- currency symbols
- geometric shapes
- box drawing
- braille patterns
- Japanese (Hiragana + Katakana)
- Korean (Hangul)
- Chinese (CJK Ideographs)
- emoji

Examples below illustrate the current glyph coverage.

---

### Uppercase English

<p align="center">
  <img src="/fonts/supported_fonts_0.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_0.jpeg" width="420" />
</p>

---

### Lowercase English

<p align="center">
  <img src="/fonts/supported_fonts_1.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_1.jpeg" width="420" />
</p>

---

### Digits

<p align="center">
  <img src="/fonts/supported_fonts_2.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_2.jpeg" width="420" />
</p>

---

### Basic Punctuation

<p align="center">
  <img src="/fonts/supported_fonts_3.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_3.jpeg" width="420" />
</p>

---

### Latin Extended A

<p align="center">
  <img src="/fonts/supported_fonts_7.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_7.jpeg" width="420" />
</p>

---

### Greek

<p align="center">
  <img src="/fonts/supported_fonts_10.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_10.jpeg" width="420" />
</p>

---

### Cyrillic

<p align="center">
  <img src="/fonts/supported_fonts_11.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_11.jpeg" width="420" />
</p>

---

### Emoji Faces

<p align="center">
  <img src="/fonts/supported_fonts_27.jpeg" width="420" />
  <img src="/fonts/bw_supported_fonts_27.jpeg" width="420" />
</p>

---

# 🎤 Voice Support

⚠️ Voice training is currently in progress.

Voice datasets and models will be integrated in a later alpha stage.

Sample voices will be published once the training pipeline is finalized.

---

# 🔐 Security

Current security work includes:

- sandboxing external inputs
- validating assistant commands
- preventing unsafe execution
- securing Python ↔ Rust communication

Security audits are ongoing during alpha.

---

# 🧪 Development Status

| Module | Status |
|------|------|
| UI Renderer | Stable |
| Font System | Stable |
| Unicode Coverage | Stable |
| Python Bridge | In Progress |
| Rust Bridge | In Progress |
| Voice Engine | Training |
| Plugin System | Planned |

---

# 🤝 Contributing

Contribution guidelines will be released during **Beta Phase**.

Until then:

- bug reports are welcome
- stability testing feedback appreciated

Report issues here:

https://github.com/Site123456/CORSPRITE/issues

---

# 📜 License

License will be finalized before **public beta release**.

---

# ⚠️ Disclaimer

CORSPRITE is currently **experimental alpha software**.

Expect:

- crashes
- incomplete features
- unstable APIs

Use at your own risk.

---

# 🧭 Roadmap

### Alpha
- core UI
- rendering
- Unicode support
- system architecture

### Beta
- voice engine
- plugin system
- scripting
- stability improvements

### Release
- public builds
- installer
- documentation
- customization tools

---

# ⚠️ Naming Notice

The name **CORSPRITE**, logo, and domain may change before the official beta release.
