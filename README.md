# Sim 5.7 — Character Pipeline: MPFB → MetaHuman in UE5

Unreal Engine 5.7 project with a **custom REST API plugin** (UETerminalBridge) and a **full character pipeline** from MPFB/MakeHuman to UE5 skeletal meshes and MetaHuman conversion.

Built entirely via AI-assisted development (opencode + Gemini) — every import, spawn, property change, and pipeline step was executed through HTTP API calls, never through the UE editor UI.

---

## Overview

```
Blender (MPFB) ──FBX──► UE5 (UETerminalBridge API) ──► Skeletal Mesh
                              │
                              ├──► MetaHumanCharacter ──► Blueprint
                              │
                              └──► Placed in Level
```

The pipeline:
1. **Blender 5.1 + MPFB v2.0.16** — parametric character creation (age, gender, muscle, weight, height)
2. **FBX export** — with embedded textures, `default_no_toes` rig
3. **UETerminalBridge API** — import FBX via `POST /api/asset/import`
4. **MetaHuman conversion** — create MetaHumanCharacter from imported mesh, conform, auto-rig, build Blueprint

---

## API Reference — UETerminalBridge (Port 8090)

All endpoints accept/send JSON. Run while the UE editor is open.

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/status` | Server health check |
| GET | `/api/assets` | List assets in content browser |
| GET | `/api/actors` | List all actors in the level |
| POST | `/api/asset/import` | Import FBX/GLTF/OBJ into content browser |
| POST | `/api/asset/place` | Spawn an asset as an actor in the level |
| POST | `/api/asset/export` | Export an asset to disk |
| POST | `/api/exec` | Execute Python code in the editor |
| POST | `/api/property` | Set an actor's property |
| GET | `/api/property` | Read an actor's property |
| POST | `/api/spawn` | Spawn actor by class name |

### Key endpoints

**Import an FBX:**
```bash
curl -X POST http://127.0.0.1:8090/api/asset/import \
  -H "Content-Type: application/json" \
  -d '{"file":"C:/path/to/character.fbx","destination":"/Game/MyFolder"}'
```

**Execute Python in editor:**
```bash
curl -X POST http://127.0.0.1:8090/api/exec \
  -H "Content-Type: application/json" \
  -d '{"code":"import unreal; print(unreal.EditorLevelLibrary.get_editor_world())"}'
```

**Place an asset in the level:**
```bash
curl -X POST http://127.0.0.1:8090/api/asset/place \
  -H "Content-Type: application/json" \
  -d '{"asset":"/Game/MyFolder/Character.Character","label":"MyChar","location":[0,300,0]}'
```

---

## Pipeline Scripts

### Tools/makehuman_to_metahuman/

| Script | Purpose |
|---|---|
| **blender_create_character.py** | Blender headless script: creates MPFB character → FBX export. Parameters: `--name`, `--age` (0.0=baby..1.0=old), `--gender` (0.0=female..1.0=male), `--muscle`, `--weight`, `--height`, `--output` |
| **ue5_import_and_convert.py** | UE5 Python script: imports FBX → creates MetaHumanCharacter → conforms → auto-rigs → builds. Runs via `POST /api/exec` |
| **run_pipeline.ps1** | PowerShell orchestrator: triggers Blender export → UE5 API import → UE5 exec conversion |
| **pipeline_config.json** | Runtime config (name, fbx_path, build_path) |

**Usage:**
```powershell
# Full pipeline: create character and import
.\run_pipeline.ps1 -Name "OldMan" -Age 0.85 -Gender 0.9 -Muscle 0.3

# Standalone Blender export
blender --background --python blender_create_character.py -- ^
  --name "YoungMale" --age 0.3 --gender 1.0 --muscle 0.5 ^
  --output "exports/YoungMale.fbx"

# Standalone UE5 import (via API)
curl -X POST http://127.0.0.1:8090/api/exec ^
  -H "Content-Type: application/json" ^
  -d '{"code":"C:/path/to/ue5_import_and_convert.py"}'
```

### Content/Scripts/

| Script | Purpose |
|---|---|
| **build_metahuman.py** | Build a MetaHuman Blueprint from an existing MHC asset (used for Hannah). Sets DefaultPawnClass on SimGameMode_BP |
| **check_mh_bp.py** | Inspect MetaHuman Blueprint class hierarchy |
| **check_mh_bp2.py** | Check superclass chain of BP_MHC_Hannah |
| **check_mh_bp3.py** | Load and verify BlueprintGeneratedClass |
| **hotreload.py** | Recompile C++ game module via editor |
| **set_default_pawn.py** | Attempt to set BP_MHC_Hannah as DefaultPawnClass via CDO |
| **ai_bridge.py** | AI command listener for editor automation |
| **ai_listener.py** | File-based AI command watcher |
| **ai_sender.py** | Ollama-based command sender |
| **ue_bridge.py** | HTTP bridge to Ollama for AI-assisted UE control |

---

## Plugin: UETerminalBridge

Custom C++ plugin (`Plugins/UETerminalBridge/`) that embeds an HTTP server in the UE editor using UE's built-in `HTTPServer` module.

**Source files:**
```
Plugins/UETerminalBridge/
├── UETerminalBridge.uplugin          # Plugin descriptor
├── Source/UETerminalBridge/
│   ├── UETerminalBridge.Build.cs     # Build configuration
│   ├── Public/
│   │   ├── TerminalServer.h          # HTTP server header
│   │   └── UETerminalBridgeModule.h  # Module header
│   └── Private/
│       ├── TerminalServer.cpp        # Server + route implementations
│       └── UETerminalBridgeModule.cpp # Module startup
└── CLI/
    └── uecmd.py                      # CLI helper
```

**Architecture:**
- Listens on port 8090
- Uses `FHttpServer` from `HTTPServer` module
- Routes defined as lambda handlers in `TerminalServer.cpp::Start()`
- `/api/exec` runs Python via `GEngine->Exec()` (writes inline code to temp file)
- No external dependencies — pure UE5 C++

**Building:**
```bash
UnrealBuildTool.exe simEditor Win64 Development ^
  -Project="path/to/sim.uproject" -SkipIniMerge
```

---

## Character Pipeline Detail

### MPFB Character Parameters

| Parameter | Range | Effect |
|---|---|---|
| `age` | 0.0–1.0 | 0.0=baby, 0.1875=child, 0.5=young adult, 1.0=old |
| `gender` | 0.0–1.0 | 0.0=female, 1.0=male |
| `muscle` | 0.0–1.0 | 0.0=frail, 1.0=bodybuilder |
| `weight` | 0.0–1.0 | 0.0=thin, 1.0=fat |
| `height` | 0.0–1.0 | 0.0=short, 1.0=tall |

### Rig Options (MPFB `add_builtin_rig`)

| Rig Name | Use Case |
|---|---|
| `default` | Full rig with toes |
| `default_no_toes` | Standard UE5 compatible (used in pipeline) |
| `game_engine` | Game-ready simplified rig |
| `cmu_mb` | CMU Motion Builder compatible |
| `mixamo` | Mixamo-compatible rig |
| `mixamo_unity` | Mixamo + Unity compatible |
| `openpose` | OpenPose skeleton |

### Skeleton Strategy

Each FBX import can either:
- **Share `metahuman_base_skel`** — all characters use the same skeleton, one Animation Blueprint works for all. Done by setting `skeleton` in `FbxImportUI`
- **Use own skeleton** — each character gets unique skeleton + physics asset. Need separate ABP per character

---

## MetaHuman Conversion

The project has two working MetaHuman approaches:

1. **Built-in Blueprint (Hannah)** — `BP_MHC_Hannah` from `MHC_Hannah` assets. Build via `build_metahuman.py`. Full face/body meshes, baked textures, grooms, materials. Ready to use as player character.

2. **MPFB → MetaHuman (blocked by cloud)** — `MetaHumanCharacterFactoryNew` creates a blank character, but auto-rigging requires Epic online services (timeout crash). Pending non-blocking refactor.

---

## Project Structure

```
sim 5.7/
├── Content/
│   ├── Imports/               # Third-party imported assets (.gitignored)
│   ├── MetaHumans/
│   │   ├── Hannah/            # BP_MHC_Hannah Blueprint + face/body/grooms
│   │   └── MPFB/              # MPFB-created characters (OldMan, YoungMale)
│   ├── Scripts/               # UE5 Python pipeline scripts
│   ├── SimBlank/              # Starter content (levels, BPs)
│   └── SimLife/               # Life simulation framework
├── Source/
│   └── sim/
│       ├── SimGameMode.h/.cpp      # Game mode with MetaHuman pawn
│       ├── SimCharacter.h/.cpp     # Base character class
│       ├── SimPlayerController.*   # Player controller
│       ├── SimGameState.*          # Game state
│       └── SimPlayerState.*        # Player state
├── Plugins/
│   ├── UETerminalBridge/      # Custom C++ REST API plugin
│   └── SoftUEBridge/          # External 114-tool bridge
├── Tools/
│   └── makehuman_to_metahuman/ # Pipeline scripts (Blender + UE5)
├── Docs/                      # Screenshots
├── Config/                    # UE configuration
├── README.md                  # This file
└── sim.uproject               # Project file
```

---

## Requirements

| Component | Version |
|---|---|
| Unreal Engine | 5.7+ |
| Visual Studio | 2022 (for C++ builds) |
| Blender | 5.1+ (for MPFB character creation) |
| Python | 3.11+ (for pipeline scripts) |
| MPFB Addon | v2.0.16 (Blender extension) |

---

## Credits & Sources

### Core

| Component | Author / Source | License |
|---|---|---|
| **UETerminalBridge plugin** | This project (unicbda-droid) | MIT |
| **Pipeline scripts** | This project (unicbda-droid) | MIT |
| **SimLife framework** | This project (unicbda-droid) | MIT |

### Unreal Engine

| Component | Author / Source | License |
|---|---|---|
| **Unreal Engine 5.7** | Epic Games | [Proprietary EULA](https://www.unrealengine.com/eula) |
| **MetaHuman Framework** | Epic Games | Included with UE5 |
| **SoftUEBridge** | Community open-source | [MIT](https://github.com/anomalyco/soft-ue-bridge) |

### Blender & Character Creation

| Component | Author / Source | License |
|---|---|---|
| **Blender 5.1** | Blender Foundation | [GPL 2.0+](https://www.blender.org/about/license/) |
| **MPFB v2.0.16** | MPFB Community | [GPL 3.0](https://github.com/makehumancommunity/mpfb2) |
| **MakeHuman 1.3.0** | MakeHuman Community | [AGPL 3.0](https://github.com/makehumancommunity/makehuman) |

### AI Development Tools

| Component | Author / Source | License |
|---|---|---|
| **opencode** | anomalyco | [Apache 2.0](https://opencode.ai) |
| **Gemini** | Google DeepMind | Proprietary |
| **Claude** | Anthropic | Proprietary |

### Marketplace Assets

All third-party 3D assets (`Content/Imports/`, `Content/FabAssets/`) retain their original licenses and are excluded from version control.

| Asset | Source | License |
|---|---|---|
| Elephant Statue | [Fab](https://fab.com/s/2f16c85f3122) | Free download |
| Animal Pack (Stegasaurus, etc.) | [Sketchfab](https://sketchfab.com/) | CC Attribution |
| Snowy Mountain Road | Fab Marketplace | Standard |
| Various environment assets | Fab Marketplace | Standard |

---

## License

```
MIT License

Copyright (c) 2026 unicbda-droid

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

This license applies only to:
- C++ plugin code in `Plugins/UETerminalBridge/`
- Python scripts in `Tools/` and `Content/Scripts/`
- Build configuration and documentation

Third-party assets (`Content/Imports/`, `Content/FabAssets/`, `Content/MetaHumans/`) are **not** covered by this license and retain their original terms.
