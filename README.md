<br/>
<div align="center">
  <img alt="ForbocAI logo" src="https://forboc.ai/logo.png" height="50" align="center">

  <br/>

# ForbocAI NPC SDK for Unreal Engine 5.8

Build genre-agnostic NPC interactions with persistent local memory, rules-aware actions, portable identity, and hosted ForbocAI inference, callable from C++ and Blueprints.

[![Documentation](https://img.shields.io/badge/docs-docs.forboc.ai-blue)](https://docs.forboc.ai)

</div>

---

## What you get

- **NPCs**: Game-defined NPC identities you can create, process, and update through immutable APIs.
- **Memory**: Embedding-backed recall so agents remember past interactions across sessions.
- **Bridge**: Validate agent-proposed actions against your game's ruleset before they fire.
- **Souls**: Export and re-import an agent's identity (JSON) — portable across projects and saves.
- **Presentation hooks**: A finalized-dialogue delegate that your own TTS, animation, or chat UI can consume.
- **Blueprint surface**: All public operations exposed as `BlueprintCallable` nodes.
- **CLI**: `doctor`, `status`, `npc create`, `npc process`, `soul export`, and friends, runnable from the SDK checkout through `scripts/forbocai-ue`.

NPC reasoning is hosted on the ForbocAI API; the plugin handles local capabilities (memory, identification, soul transport, command surface) and talks to the API over HTTP.

---

## Installation

### Option 1 — Fab

1. Find **ForbocAI NPC SDK** on [Fab](https://fab.com).
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

NPC inference requires a ForbocAI account and API key from <https://account.forboc.ai>. The plugin uses `https://api.forboc.ai` by default; configure another endpoint through `FORBOCAI_API_URL`, `FAgentConfig::ApiUrl`, or the SDK config file.

---

## Quick start (C++)

```cpp
#include "NPC/NPCModule.h"

// 1. Create an agent
FAgentConfig Config;
Config.Persona = TEXT("A careful local merchant who knows today's inventory.");
// Config.ApiUrl is optional and defaults to the SDK's configured API URL.

const FAgent Npc = AgentFactory::Create(Config);

// 2. Process player input asynchronously
AgentOps::Process(Npc, TEXT("What do you have available?"), {})
    .then([](const FAgentResponse& Response) {
        UE_LOG(LogTemp, Log, TEXT("Reply: %s"), *Response.Dialogue);
    })
    .catch_([](const std::string& Error) {
        UE_LOG(LogTemp, Error, TEXT("ForbocAI request failed: %s"),
               *FString(Error.c_str()));
    })
    .execute();

// 3. Update agent state — returns a NEW agent (originals stay untouched)
const FAgentState Suspicious = TypeFactory::AgentState(TEXT("Suspicious"));
const FAgent Updated = AgentOps::WithState(Npc, Suspicious);
```

> All public types are immutable structs. Operations return new values rather than mutating in place — assign the result back if you want to keep it.

## Quick start (Blueprint)

The full `demo-ue-5` runtime project lives at [`ForbocAI/demo-ue-5`](https://github.com/ForbocAI/demo-ue-5) and ships source-controlled Blueprint and UMG assets for the happy-path runtime map. Open `/Game/Map/Maps/Runtime`, press Play, and use the tracked `BP_LevelGameMode`, `BP_PlayerRuntimeController`, `BP_TownspersonView`, `BP_SpeechPresenter`, and `WBP_Chat` assets as the editor-facing integration points.

For SDK/API validation, call the `UForbocAIBlueprintLibrary` nodes from source-controlled Blueprint assets and keep runtime decisions in reducers/thunks rather than local widget graphs.

---

## CLI smoke tests

The SDK ships a dedicated CLI host project at `ForbocAI_CLI.uproject` and runner scripts under `scripts/`. The runner builds the commandlet host when needed and dispatches through the same `CLIOps`/RTK store path used by runtime C++ and Blueprint integrations. It does not use the micro-game.

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

## Micro-game CLI coverage

The separate `micro-game-cli/` host project wraps the SDK CLI path for full scenario coverage. It fetches the API-owned micro-game contract, runs every scenario command through `MicroGame::CommandSurface`, and `MicroGame::CommandSurface` delegates each command to the canonical SDK `CLIOps::DispatchCommand` boundary.

```bash
scripts/forbocai-ue-micro-game contract
scripts/forbocai-ue-micro-game --mode autoplay
```

On Windows, use `scripts\forbocai-ue-micro-game.cmd` with the same arguments. The runner loads `FORBOCAI_API_KEY` from the process environment or `FORBOCAI_TEST_ENV_FILE` (defaulting to `~/.config/forbocai/test-api.env`) and uses `https://api.forboc.ai` when `FORBOCAI_API_URL` is unset.

---

## Documentation & support

- Full reference, tutorials, and protocol docs: <https://docs.forboc.ai>
- Runtime project: <https://github.com/ForbocAI/demo-ue-5>
- Issues and feature requests: <https://github.com/ForbocAI/sdk-ue-5/issues>

---

## License

Fab acquisitions use the Fab Standard License. Other distribution channels require a separate ForbocAI license. See [LICENSE](./LICENSE).
