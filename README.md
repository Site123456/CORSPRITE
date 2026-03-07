<p align="center">
  <img src="/logo/logo_banner_beta.png" alt="CORSPRITE Logo" width="480" />
</p>
<p align="center">
<strong>An interactive assistant engineered to feel native, alive, and unmistakably yours.</strong>
</p>

<p align="center">
<b>Current builds target Windows x64 architecture.</b>
</p>

---
# 🖥 Supported Platforms

Currently tested on:

- Windows 11 (x64)

⚠️ **Architecture Notice**

The current public build is compiled for **64-bit (x86_64) systems only**.

32-bit Windows systems are **not supported**.

Planned platforms:

- Linux (x64)
- Windows (X32-bit)

⚠️ **Important**

- Training datasets will **NOT be publicly distributed**
- Upload features will be disabled until security validation is complete

---

# 📘 Documentation

Full documentation is available during beta:

Docs (Temporary link):
https://corsprite-docs.vercel.app/

Development updates:  
https://www.youtube.com/@CORSPRITE

Made possible thanks to: 
https://github.com/nlohmann/json/tree/develop
https://github.com/ocornut/imgui

---

# 🧠 Project Vision

CORSPRITE is designed to be a **local-first interactive assistant** that feels:

- native to the system
- responsive not entirely minimum screen: 620 width X 700 height
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
Corsprite Luncher (ImGui) ~140mo
 │
 ▼
C++ UI Layer (ImGui) ~40GO
 │
 ▼
Python Assistant Logic ~180GO
 │
 ▼
Rust System Bridge ~6GO
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

Compile app:
```bash
make
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

Verify installation:

```bash
g++ --version
```

Compile app:

```bash
make
```
---

# Run

Run the application:

```bash
Corsprite_luncher.exe
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

# 🔐 Security

Current security work includes:

- sandboxing external inputs
- validating assistant commands
- preventing unsafe execution
- securing Python ↔ Rust communication

Security audits are ongoing during alpha.

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

# ⚠️ Naming Notice

The name **CORSPRITE**, logo, and domain may change before the official beta release.
