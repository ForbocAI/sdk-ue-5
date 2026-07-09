<br/>
<div align="center">
  <img alt="ForbocAI logo" src="https://forboc.ai/logo.png" height="50" align="center">

  <br/>

# ForbocAI SDK for Unreal Engine 5.8

Drop-in autonomous NPCs for Unreal Engine 5.8 — neuro-symbolic agents with persistent memory, identity, and ruleset-aware decision making, callable from C++ and Blueprints.

[![Documentation](https://img.shields.io/badge/docs-docs.forboc.ai-blue)](https://docs.forboc.ai)
[![Fab](https://img.shields.io/badge/Fab-ForbocAI-orange)](https://fab.com)

</div>

---

## What you get

- **Agents**: Persona-driven NPCs you can spawn, update, and delete from a single factory call.
- **Memory**: Embedding-backed recall so agents remember past interactions across sessions.
- **Bridge**: Validate agent-proposed actions against your game's ruleset before they fire.
- **Souls**: Export and re-import an agent's identity (JSON) — portable across projects and saves.
- **Speech & dialogue hooks**: Drop-in components for TTS, viseme blending, and chat UI.
- **Blueprint surface**: All public operations exposed as `BlueprintCallable` nodes.
- **CLI**: `doctor`, `status`, `npc create`, `npc process`, `soul export`, and friends, runnable from the SDK checkout through `scripts/forbocai-ue`.

NPC reasoning is hosted on the ForbocAI API; the plugin handles local capabilities (memory, identification, soul transport, command surface) and talks to the API over HTTP.

---

## Installation

### Option 1 — Fab (recommended)

1. Search for **ForbocAI** on [Fab](https://fab.com).
2. Add to library and install to your engine.
3. Enable the plugin in your project: `Edit → Plugins → ForbocAI`.

### Option 2 — Manual

1. Download a release archive from the [Releases](https://github.com/ForbocAI/sdk-ue-5/releases) page or clone this repository.
2. Place the repository contents into your project's `Plugins/ForbocAI_SDK/` directory.
3. Right-click your `.uproject` and **Generate Visual Studio project files**.
4. Build in `Development Editor`.

### Prerequisites

| Platform | Tools |
|---|---|
| Windows | UE 5.8, [VS Build Tools 2022](https://aka.ms/vs/17/release/vs_buildtools.exe) (C++ workload + Windows 11 SDK) |
| macOS | UE 5.8, Xcode 15+ |
| Linux | UE 5.8, Clang 16+ |

The plugin reaches an API endpoint at runtime. By default it tries `http://localhost:8080`, then uses `https://api.forboc.ai` when localhost is unavailable. Override with `FAgentConfig::ApiUrl` or via the SDK config.

---

## Quick start (C++)

```cpp
#include "AgentModule.h"
#include "MemoryModule.h"

// 1. Create an agent
FAgentConfig Config;
Config.Persona = TEXT("Cyber-Merchant");
// Config.ApiUrl is optional — defaults to localhost, then api.forboc.ai.

const FAgent Merchant = AgentFactory::Create(Config);

// 2. Process player input asynchronously
AgentOps::Process(
    Merchant, TEXT("What wares do you have?"), {},
    [](FAgentResponse Response) {
        UE_LOG(LogTemp, Log, TEXT("Reply: %s"), *Response.Dialogue);
    });

// 3. Update agent state — returns a NEW agent (originals stay untouched)
const FAgentState Suspicious = TypeFactory::AgentState(TEXT("Suspicious"));
const FAgent Updated = AgentOps::WithState(Merchant, Suspicious);

// 4. Memory store — add an interaction
const FMemoryStore Store = MemoryOps::CreateStore();
const FMemoryStore After = MemoryOps::Store(
    Store, TEXT("Customer asked about wares"), TEXT("interaction"), 0.8f);
```

> All public types are immutable structs. Operations return new values rather than mutating in place — assign the result back if you want to keep it.

## Quick start (Blueprint)

The full `demo-ue-5` runtime project lives at [`ForbocAI/demo-ue-5`](https://github.com/ForbocAI/demo-ue-5) and ships source-controlled Blueprint and UMG assets for the happy-path runtime map. Open `/Game/Map/Maps/Runtime`, press Play, and use the tracked `BP_LevelGameMode`, `BP_PlayerRuntimeController`, `BP_TownspersonView`, `BP_SpeechPresenter`, and `WBP_Chat` assets as the editor-facing integration points.

For SDK/API validation, call the `UForbocAIBlueprintLibrary` nodes from source-controlled Blueprint assets and keep runtime decisions in reducers/thunks rather than local widget graphs.

---

## CLI smoke tests

The SDK ships a dedicated CLI host project at `ForbocAI_CLI.uproject` and runner scripts under `scripts/`. The runner builds the commandlet host when needed and dispatches through the same `CLIOps`/RTK store path used by runtime C++ and Blueprint integrations. It does not use the test game.

### Windows

```powershell
.\scripts\forbocai-ue.cmd status
.\scripts\forbocai-ue.cmd doctor
.\scripts\forbocai-ue.cmd --api-url https://api.forboc.ai status
```

### macOS / Linux

```bash
scripts/forbocai-ue status
scripts/forbocai-ue doctor
scripts/forbocai-ue --api-url https://api.forboc.ai status
```

Set `UE_ROOT` if Unreal Engine 5.8 is not installed in the default location. The first run may take a few minutes while Unreal builds the CLI host; subsequent runs are faster. Set `FORBOCAI_UE_SKIP_BUILD=1` in CI after the host target has already been built.

| Command | Purpose |
|---|---|
| `doctor` | Check API connectivity and report SDK version |
| `status` | Check API connectivity |
| `npc create "..."` | Create a new agent |
| `npc process <id> "..."` | Send input to an agent |
| `soul export <id>` | Export an agent's soul |
| `config set apiUrl https://api.forboc.ai` | Persist a CLI config value |
| `config get apiUrl` | Read a stored CLI config value |

Sample `doctor` output:

```
ForbocAI CLI (UE5) — Command: doctor
API Status: online (v0.4.0)
```

Advanced users can still call the underlying commandlet directly with `UnrealEditor-Cmd`, `-run=ForbocAI`, `-Command=<command_key>`, and named parameters such as `-Id=` or `-Input=`.

---

## Documentation & support

- Full reference, tutorials, and protocol docs: <https://docs.forboc.ai>
- Runtime project: <https://github.com/ForbocAI/demo-ue-5>
- Issues and feature requests: <https://github.com/ForbocAI/sdk-ue-5/issues>

---

## License

© 2026 ForbocAI, Inc. All rights reserved. See [LICENSE](./LICENSE) for full terms.
