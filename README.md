# DialScript

Experimental dialog scripting language for games.

## Installation

### macOS / Linux

```bash
git clone https://github.com/arsmotorin/dialscript.git
cd dialscript
mkdir build && cd build
cmake .. && make
sudo make install
```

### Windows

```powershell
git clone https://github.com/arsmotorin/dialscript.git
cd dialscript
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Binary will be in `build\Release\dialscript.exe`. Add it to PATH or copy to `C:\Windows\`.

## Syntax

| Element | Description                        |
|---------|------------------------------------|
| `[Scene.N]` | Scene block header                 |
| `[Dialog.N]` | Dialog block header                |
| `Level`, `Location`, `Characters` | Scene metadata                     |
| `Name: Text` | Dialog line                        |
| `{Key: Value}` | Line metadata (will be in v.0.0.2) |
| `// comment` | Comment                            |
