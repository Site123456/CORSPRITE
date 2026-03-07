# <p align="center"><img src="/logo/logo_banner_beta.png" width="480" /></p>

<p align="center">
  <strong>A local‑first interactive assistant engineered to feel native, alive, and unmistakably yours.</strong><br>
  <strong>Current builds target Windows x64.</strong>
</p>

<p align="center">
  <img src="/images/Showcaseof2026-03-07.png" width="720" />
</p>

<p align="center">
  <a href="https://corsprite-docs.vercel.app/"><img src="https://img.shields.io/badge/Docs-Beta-blue?style=for-the-badge"></a>
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

- Docs: https://corsprite-docs.vercel.app/  
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

Build:
```bash
make
```

---

## ⚙️ Installation

```bash
git clone https://github.com/Site123456/CORSPRITE.git
cd CORSPRITE
make
```

Run:
```bash
Corsprite_luncher.exe
```

---

## 🔐 Security

Active work includes:

- sandboxing external inputs  
- validating assistant commands  
- securing Python ↔ Rust communication  

---

## 🤝 Contributing

Contribution guidelines will arrive during **Beta**.  
Bug reports: https://github.com/Site123456/CORSPRITE/issues

---

## 📜 License

To be finalized before public beta.

---

## ⚠️ Disclaimer

CORSPRITE is **experimental alpha software**.  
Expect crashes, incomplete features, and unstable APIs.
