#!/usr/bin/env python3
"""Audit TS SDK <-> UE SDK parity and keep the SDK map generated section current.

The strict checks here are for invariants that must already match, such as the
Node CLI command key matrix. Broader folder/file/function drift is reported into
the generated inventory so the rename/parity backlog stays visible while UE
moves incrementally toward the TS SDK shape.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from datetime import date
from pathlib import Path


GENERATED_START = "<!-- BEGIN GENERATED SDK PARITY INVENTORY -->"
GENERATED_END = "<!-- END GENERATED SDK PARITY INVENTORY -->"

TS_SOURCE_ROOTS = (
    ("core", Path("packages/core/src")),
    ("node", Path("packages/node/src")),
    ("test-game", Path("packages/test-game/src")),
)

UE_SOURCE_ROOTS = (
    Path("Source/ForbocAI_SDK"),
)

UE_EXTRA_FILES = (
    Path("ForbocAI_CLI.uproject"),
    Path("Source/ForbocAI_SDK/ForbocAI_SDK.Build.cs"),
    Path("scripts/forbocai-ue"),
    Path("scripts/forbocai-ue.cmd"),
    Path("scripts/forbocai-ue.ps1"),
)

IGNORED_PARTS = {"__tests__", "dist", "Binaries", "Intermediate", "Saved", "Tests", "ThirdParty"}

MANUAL_FILE_MIRRORS: dict[str, tuple[str, tuple[str, ...], str]] = {
    "packages/core/src/api/endpoints/arweave.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/Core/ThunkDetail.h"),
        "TS endpoint builder maps to UE endpoint wrappers plus Arweave thunk detail helpers.",
    ),
    "packages/core/src/api/endpoints/builder.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APICodecs.h",),
        "UE endpoint builder helpers live with API codecs.",
    ),
    "packages/core/src/api/endpoints/ghost.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/API/APICodecs.h"),
        "UE keeps Ghost endpoint declarations and codecs in shared API headers.",
    ),
    "packages/core/src/api/endpoints/npc.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/API/APICodecs.h"),
        "UE keeps NPC endpoint declarations and codecs in shared API headers.",
    ),
    "packages/core/src/api/endpoints/rules.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/API/APICodecs.h"),
        "UE keeps bridge/rules endpoint declarations and codecs in shared API headers.",
    ),
    "packages/core/src/api/endpoints/soul.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/API/APICodecs.h"),
        "UE keeps Soul endpoint declarations and codecs in shared API headers.",
    ),
    "packages/core/src/api/endpoints/system.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIEndpoints.h", "Source/ForbocAI_SDK/Public/API/APIThunks.h"),
        "UE exposes status through API endpoints plus status thunks.",
    ),
    "packages/core/src/apiSlice.ts": (
        "Analog",
        (
            "Source/ForbocAI_SDK/Public/API/APISlice.h",
            "Source/ForbocAI_SDK/Public/API/APIEndpoints.h",
            "Source/ForbocAI_SDK/Public/API/APICodecs.h",
        ),
        "TS RTK Query slice is split into UE slice, endpoint, and codec headers.",
    ),
    "packages/core/src/bridgeSlice.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/Bridge/BridgeSlice.h", "Source/ForbocAI_SDK/Public/Bridge/BridgeThunks.h"),
        "Bridge slice and async thunks are split in UE.",
    ),
    "packages/core/src/cliCommandMatrix.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h",),
        "UE mirrors the TS Node command key block.",
    ),
    "packages/core/src/configOps.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API/APIThunks.h", "Source/ForbocAI_SDK/Public/CLI/CliOperations.h"),
        "TS connectivity helper maps to UE status thunk and CLI operation.",
    ),
    "packages/core/src/directiveSlice.ts": ("Exact", ("Source/ForbocAI_SDK/Public/DirectiveSlice.h",), ""),
    "packages/core/src/errors.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Errors.h",), ""),
    "packages/core/src/functional_core.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Core/ue_fp.hpp", "Source/ForbocAI_SDK/Public/Core/rtk.hpp"),
        "UE splits FP helpers and RTK primitives.",
    ),
    "packages/core/src/ghostSlice.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/Ghost/GhostSlice.h", "Source/ForbocAI_SDK/Public/Ghost/GhostThunks.h"),
        "Ghost slice and async thunks are split in UE.",
    ),
    "packages/core/src/handlers/arweave.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Core/ThunkDetail.h",),
        "UE keeps Arweave upload/download helpers in thunk detail.",
    ),
    "packages/core/src/index.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Types.h", "Source/ForbocAI_SDK/Public/Thunks.h"),
        "TS package barrel maps to UE public umbrella headers.",
    ),
    "packages/core/src/listeners.ts": ("Analog", ("Source/ForbocAI_SDK/Public/RuntimeStore.h",), ""),
    "packages/core/src/memory/InMemoryMemoryEngine.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h", "Source/ForbocAI_SDK/Private/Memory/MemoryStorageOps.cpp"),
        "UE native memory backend differs from TS in-memory engine.",
    ),
    "packages/core/src/memory/MemoryEngine.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Memory/MemoryTypes.h", "Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h"),
        "UE expresses the memory engine through native thunks and types.",
    ),
    "packages/core/src/memory/SQLiteMemoryEngine.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/NativeStorage.h", "Source/ForbocAI_SDK/Private/Memory/MemoryStorageOps.cpp"),
        "UE SQLite-backed memory lives in native storage helpers.",
    ),
    "packages/core/src/memory/embed.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h", "Source/ForbocAI_SDK/Public/Core/ThunkDetail.h"),
        "UE embedding is routed through native vector thunks.",
    ),
    "packages/core/src/memorySlice.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/Memory/MemorySlice.h", "Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h"),
        "Memory slice and async thunks are split in UE.",
    ),
    "packages/core/src/npc.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/NPC/NPCModule.h", "Source/ForbocAI_SDK/Public/NPC/NPCTypes.h"),
        "UE splits NPC helpers and types into module/type headers.",
    ),
    "packages/core/src/npcSlice.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/NPC/NPCSlice.h", "Source/ForbocAI_SDK/Public/NPC/NPCSliceActions.h"),
        "NPC slice and action helpers are split in UE.",
    ),
    "packages/core/src/protocolHandlers/handlers.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Protocol/ProtocolThunks.h",),
        "UE protocol handlers are inline thunk helpers.",
    ),
    "packages/core/src/protocolHandlers/index.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Protocol/ProtocolThunks.h",),
        "UE dispatch table and handlers live together.",
    ),
    "packages/core/src/protocolHandlers/types.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Protocol/ProtocolTypes.h",),
        "",
    ),
    "packages/core/src/protocolLogger.ts": ("Analog", ("Source/ForbocAI_SDK/Public/RuntimeStore.h",), ""),
    "packages/core/src/runtimeRegistry.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Core/ThunkDetail.h",),
        "UE uses native singleton handles instead of a TS runtime registry object.",
    ),
    "packages/core/src/soulSlice.ts": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/Soul/SoulSlice.h", "Source/ForbocAI_SDK/Public/Soul/SoulThunks.h"),
        "Soul slice and async thunks are split in UE.",
    ),
    "packages/core/src/store.ts": ("Exact", ("Source/ForbocAI_SDK/Public/RuntimeStore.h",), ""),
    "packages/core/src/thunks.ts": (
        "Analog",
        (
            "Source/ForbocAI_SDK/Public/Protocol/ProtocolThunks.h",
            "Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h",
            "Source/ForbocAI_SDK/Public/Soul/SoulThunks.h",
        ),
        "TS shared thunks are split by UE domain.",
    ),
    "packages/core/src/types.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Types.h",), ""),
    "packages/core/src/types/arweave.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Soul/SoulTypes.h",), ""),
    "packages/core/src/types/bridge.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Bridge/BridgeTypes.h",), ""),
    "packages/core/src/types/cortex.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Protocol/ProtocolTypes.h",),
        "Prompt constraints remain protocol data; local Cortex runtime is retired.",
    ),
    "packages/core/src/types/ghost.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Ghost/GhostTypes.h",), ""),
    "packages/core/src/types/memory.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Memory/MemoryTypes.h",), ""),
    "packages/core/src/types/protocol.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Protocol/ProtocolTypes.h",), ""),
    "packages/core/src/types/soul.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Soul/SoulTypes.h",), ""),
    "packages/core/src/types/system.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Core/SystemTypes.h",), ""),
    "packages/core/src/utils.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Core/ue_fp.hpp",), ""),
    "packages/core/src/utils/delay.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Core/ue_fp.hpp",), ""),
    "packages/core/src/utils/generateNPCId.ts": ("Exact", ("Source/ForbocAI_SDK/Public/NPC/NPCId.h",), ""),
    "packages/core/src/utils/memoise.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Core/rtk.hpp",), ""),
    "packages/core/src/utils/memoiseAsync.ts": ("Gap", (), "No dedicated UE async memoise helper yet."),
    "packages/core/src/utils/pipe.ts": ("Exact", ("Source/ForbocAI_SDK/Public/Core/ue_fp.hpp",), ""),
    "packages/core/src/utils/version.ts": ("Analog", ("Source/ForbocAI_SDK/Public/RuntimeConfig.h",), ""),
    "packages/node/src/cli.ts": (
        "Analog",
        (
            "Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h",
            "Source/ForbocAI_SDK/Private/CLI/CLIModule.cpp",
            "Source/ForbocAI_SDK/Private/Commandlet.cpp",
            "scripts/forbocai-ue",
            "scripts/forbocai-ue.cmd",
            "scripts/forbocai-ue.ps1",
        ),
        "UE CLI is an Unreal commandlet plus wrapper scripts.",
    ),
    "packages/node/src/cli/commands/bridge.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/BridgeCommands.cpp",), ""),
    "packages/node/src/cli/commands/common.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliHandlers.h",), ""),
    "packages/node/src/cli/commands/ghost.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/GhostCommands.cpp",), ""),
    "packages/node/src/cli/commands/memory.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/MemoryCommands.cpp",), ""),
    "packages/node/src/cli/commands/npc.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/NpcCommands.cpp",), ""),
    "packages/node/src/cli/commands/rules.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/BridgeCommands.cpp",), ""),
    "packages/node/src/cli/commands/setup.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/SetupCommands.cpp",), ""),
    "packages/node/src/cli/commands/soul.ts": ("Analog", ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/SoulCommands.cpp",), ""),
    "packages/node/src/cli/ops.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/bridgeRules.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/config.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h", "Source/ForbocAI_SDK/Public/RuntimeConfig.h"), ""),
    "packages/node/src/cli/ops/ghost.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/memory.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/native.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/SetupCommands.cpp", "Source/ForbocAI_SDK/ForbocAI_SDK.Build.cs"),
        "",
    ),
    "packages/node/src/cli/ops/npc.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/shared.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h", "Source/ForbocAI_SDK/Public/RuntimeStore.h"), ""),
    "packages/node/src/cli/ops/soul.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/cli/ops/system.ts": ("Analog", ("Source/ForbocAI_SDK/Public/CLI/CliOperations.h",), ""),
    "packages/node/src/index.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Types.h", "Source/ForbocAI_SDK/Public/Thunks.h"), ""),
    "packages/node/src/nativeDeps.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Private/CLI/CliHandlers/SetupCommands.cpp", "Source/ForbocAI_SDK/ForbocAI_SDK.Build.cs"),
        "",
    ),
    "packages/node/src/nodeConfig.ts": ("Exact", ("Source/ForbocAI_SDK/Public/RuntimeConfig.h",), ""),
    "packages/node/src/nodeMemorySlice.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h", "Source/ForbocAI_SDK/Public/NativeStorage.h"), ""),
    "packages/node/src/nodeVectorSlice.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Memory/MemoryThunks.h", "Source/ForbocAI_SDK/Public/Core/ThunkDetail.h"), ""),
    "packages/node/src/registry.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Core/ThunkDetail.h",), ""),
    "packages/node/src/store.ts": ("Analog", ("Source/ForbocAI_SDK/Public/RuntimeStore.h",), ""),
    "packages/node/src/utils.ts": ("Analog", ("Source/ForbocAI_SDK/Public/Core/ue_fp.hpp",), ""),
    "packages/test-game/src/cli.ts": (
        "Gap",
        (),
        "UE has a commandlet/wrapper but no dedicated forbocai-test-game style wrapper yet.",
    ),
    "packages/test-game/src/cli/commands/testGame.ts": (
        "Analog",
        ("Source/ForbocAI_SDK/Private/Commandlet.cpp", "Source/ForbocAI_SDK/Public/TestGame/TestGameOrchestrator.h"),
        "UE routes test-game execution through the main commandlet.",
    ),
    "packages/test-game/src/cli/ops/testGame.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameContract.h",), ""),
    "packages/test-game/src/contract.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameContract.h",), ""),
    "packages/test-game/src/game.ts": ("Analog", ("Source/ForbocAI_SDK/Public/TestGame/TestGameOrchestrator.h",), ""),
    "packages/test-game/src/index.ts": ("Analog", ("Source/ForbocAI_SDK/Public/TestGame/TestGameTypes.h",), ""),
    "packages/test-game/src/lib/commandRunner.ts": ("Analog", ("Source/ForbocAI_SDK/Public/TestGame/TestGameCommandSurface.h",), ""),
    "packages/test-game/src/lib/coverage.ts": (
        "Analog",
        (
            "Source/ForbocAI_SDK/Public/TestGame/TestGameTypes.h",
            "Source/ForbocAI_SDK/Public/TestGame/TestGameSlices.h",
            "Source/ForbocAI_SDK/Public/TestGame/TestGameRuntime.h",
        ),
        "",
    ),
    "packages/test-game/src/lib/render.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameGridRender.h",), ""),
    "packages/test-game/src/lib/theme.ts": ("Gap", (), "UE logs plain text and has no chalk/theme layer."),
    "packages/test-game/src/listeners.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameListeners.h",), ""),
    "packages/test-game/src/store.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameStore.h",), ""),
    "packages/test-game/src/types.ts": ("Exact", ("Source/ForbocAI_SDK/Public/TestGame/TestGameTypes.h",), ""),
}

TEST_GAME_SLICE_FILES = {
    "packages/test-game/src/features/autoplay/slices/harnessSlice.ts",
    "packages/test-game/src/features/autoplay/slices/scenarioSlice.ts",
    "packages/test-game/src/features/entities/slices/npcsSlice.ts",
    "packages/test-game/src/features/entities/slices/playerSlice.ts",
    "packages/test-game/src/features/entities/slices/types.ts",
    "packages/test-game/src/features/mechanics/slices/bridgeSlice.ts",
    "packages/test-game/src/features/mechanics/slices/gridSlice.ts",
    "packages/test-game/src/features/mechanics/slices/socialSlice.ts",
    "packages/test-game/src/features/mechanics/slices/stealthSlice.ts",
    "packages/test-game/src/features/store/slices/inventorySlice.ts",
    "packages/test-game/src/features/store/slices/memorySlice.ts",
    "packages/test-game/src/features/store/slices/soulSlice.ts",
    "packages/test-game/src/features/terminal/slices/transcriptSlice.ts",
    "packages/test-game/src/features/terminal/slices/uiSlice.ts",
}

MANUAL_FOLDER_MIRRORS: dict[str, tuple[str, tuple[str, ...], str]] = {
    "packages/core/src": (
        "Analog",
        ("Source/ForbocAI_SDK/Public", "Source/ForbocAI_SDK/Private"),
        "UE splits TS core source into Unreal public/private module folders.",
    ),
    "packages/core/src/api": (
        "Exact",
        ("Source/ForbocAI_SDK/Public/API", "Source/ForbocAI_SDK/Private/API"),
        "UE keeps API declarations and implementation under matching API folders.",
    ),
    "packages/core/src/api/endpoints": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/API",),
        "TS endpoint folders collapse into shared UE API endpoint/codecs headers.",
    ),
    "packages/core/src/handlers": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Core",),
        "TS generic handlers map to UE thunk detail helpers.",
    ),
    "packages/core/src/protocolHandlers": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Protocol",),
        "TS protocol handler files map to UE Protocol thunk/type headers.",
    ),
    "packages/core/src/types": (
        "Analog",
        (
            "Source/ForbocAI_SDK/Public",
            "Source/ForbocAI_SDK/Public/Bridge",
            "Source/ForbocAI_SDK/Public/Ghost",
            "Source/ForbocAI_SDK/Public/Memory",
            "Source/ForbocAI_SDK/Public/NPC",
            "Source/ForbocAI_SDK/Public/Protocol",
            "Source/ForbocAI_SDK/Public/Soul",
        ),
        "UE distributes TS shared/domain types into reflected domain headers.",
    ),
    "packages/core/src/utils": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/Core",),
        "TS utility modules map to UE core FP/RTK helper headers.",
    ),
    "packages/node/src": (
        "Analog",
        ("Source/ForbocAI_SDK/Public", "Source/ForbocAI_SDK/Private", "scripts"),
        "UE implements Node runtime and CLI responsibilities through module headers, commandlets, and wrapper scripts.",
    ),
    "packages/node/src/cli": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/CLI", "Source/ForbocAI_SDK/Private/CLI", "scripts"),
        "UE CLI is commandlet-backed with wrapper scripts.",
    ),
    "packages/node/src/cli/commands": (
        "Analog",
        ("Source/ForbocAI_SDK/Private/CLI/CliHandlers",),
        "TS command files map to UE CLI handler implementations.",
    ),
    "packages/node/src/cli/ops": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/CLI",),
        "TS CLI ops map to UE CLI operation headers.",
    ),
    "packages/test-game/src": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/TestGame", "Source/ForbocAI_SDK/Private"),
        "UE test-game harness lives under TestGame headers plus commandlet wiring.",
    ),
    "packages/test-game/src/cli": (
        "Analog",
        ("Source/ForbocAI_SDK/Private", "Source/ForbocAI_SDK/Public/TestGame"),
        "UE routes test-game CLI execution through the main commandlet.",
    ),
    "packages/test-game/src/lib": (
        "Analog",
        ("Source/ForbocAI_SDK/Public/TestGame",),
        "TS test-game library helpers map to UE TestGame headers.",
    ),
}

MANUAL_SYMBOL_MIRRORS: dict[str, tuple[str, tuple[str, ...], str]] = {
    "api": ("Analog", ("ForbocAiApi",), "TS RTK Query api maps to UE rtk::Api instance."),
    "listenerMiddleware": ("Analog", ("createNpcRemovalListener",), "UE listener is specialized to NPC removal cleanup."),
    "checkApiConnectivity": ("Analog", ("checkApiStatus", "checkApiStatusThunk"), "UE checks status through the store thunk path."),
    "ValidationContext": ("Analog", ("FBridgeValidationContext",), ""),
    "NPCState": ("Analog", ("FAgentState",), ""),
    "NPCAction": ("Analog", ("FAgentAction",), ""),
    "CompletionOptions": ("Analog", ("FPromptConstraints",), ""),
    "createSQLiteMemoryEngine": ("Analog", ("initNodeMemoryThunk",), "UE initializes native sqlite memory through thunks."),
    "createInMemoryMemoryEngine": ("Gap", (), "UE has no in-memory memory engine mirror."),
    "MemoryEngine": ("Analog", ("NodeMemoryRuntime",), "UE exposes native memory runtime helpers instead of a TS interface."),
    "handler_ArweaveUpload": ("Analog", ("UploadSignedSoul",), ""),
    "handler_ArweaveDownload": ("Analog", ("DownloadSoulPayload",), ""),
    "runCommand": ("Analog", ("Execute", "ExecuteSpec"), "UE command surface executes mirrored command text in-process."),
    "runGame": ("Exact", ("RunGame",), ""),
    "REQUIRED_GROUPS": ("Analog", ("RequiredGroups",), ""),
    "checkRuntimeConnectivity": ("Exact", ("CheckRuntimeConnectivity",), ""),
    "resolveRuntimeUrl": ("Exact", ("ResolveRuntimeUrl",), ""),
    "getRandomFragment": ("Gap", (), "UE orchestrator does not mirror the TS text animation helper."),
    "hex": ("Gap", (), "UE does not mirror the TS terminal theme helper."),
}


@dataclass(frozen=True)
class Symbol:
    name: str
    kind: str
    file: str


@dataclass(frozen=True)
class FileMirror:
    ts_file: str
    ue_files: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class FolderMirror:
    ts_folder: str
    ue_folders: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class SymbolMirror:
    ts_symbol: Symbol
    ue_symbols: tuple[Symbol, ...]
    status: str
    notes: str


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def posix(path: Path) -> str:
    return path.as_posix()


def relative(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    return roots + [
        root.parent / "sdk",
        root.parent / "sdk-ts",
        root.parent / "sdk-node",
    ]


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    for ts_root in roots:
        if (ts_root / "packages/core/src/cliCommandMatrix.ts").is_file():
            return ts_root

    searched = "\n".join(
        f"  - {candidate / 'packages/core/src/cliCommandMatrix.ts'}"
        for candidate in roots
    )
    raise FileNotFoundError(
        "Could not find TS SDK root. Set FORBOCAI_TS_SDK_ROOT or pass "
        f"--ts-sdk-root.\nSearched:\n{searched}"
    )


def candidate_map_paths(root: Path) -> list[Path]:
    env_path = os.environ.get("FORBOCAI_SDK_MAP_PATH")
    paths = [Path(env_path)] if env_path else []
    return paths + [
        root.parent / "classified/docs/design/sdk/map.md",
        root.parent / "docs/design/sdk/map.md",
    ]


def resolve_map_path(root: Path, explicit_path: str | None) -> Path | None:
    if explicit_path:
        return Path(explicit_path)
    return next((path for path in candidate_map_paths(root) if path.is_file()), None)


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def has_ignored_part(path: Path) -> bool:
    return any(part in IGNORED_PARTS for part in path.parts)


def iter_ts_files(ts_root: Path) -> list[Path]:
    files: list[Path] = []
    for _, source_root in TS_SOURCE_ROOTS:
        root = ts_root / source_root
        if not root.is_dir():
            continue
        files.extend(
            path
            for path in root.rglob("*.ts")
            if not has_ignored_part(path.relative_to(ts_root))
            and not path.name.endswith(".d.ts")
        )
    return sorted(files, key=lambda path: relative(path, ts_root))


def iter_ue_files(root: Path) -> list[Path]:
    source_files: list[Path] = []
    for source_root in UE_SOURCE_ROOTS:
        base = root / source_root
        if not base.is_dir():
            continue
        source_files.extend(
            path
            for path in base.rglob("*")
            if path.is_file()
            and path.suffix in {".h", ".hpp", ".cpp"}
            and not has_ignored_part(path.relative_to(root))
        )

    source_files.extend(root / path for path in UE_EXTRA_FILES if (root / path).is_file())
    return sorted(source_files, key=lambda path: relative(path, root))


def source_root_for_ts_path(path: str) -> str | None:
    return next(
        (
            source_root.as_posix()
            for _, source_root in TS_SOURCE_ROOTS
            if path == source_root.as_posix() or path.startswith(f"{source_root.as_posix()}/")
        ),
        None,
    )


def iter_ts_folders(ts_files: list[Path], ts_root: Path) -> list[str]:
    folders: set[str] = set()
    for path in ts_files:
        rel = relative(path, ts_root)
        source_root = source_root_for_ts_path(rel)
        if not source_root:
            continue

        parent = Path(rel).parent
        while True:
            folder = parent.as_posix()
            if folder == "." or not (folder == source_root or folder.startswith(f"{source_root}/")):
                break
            folders.add(folder)
            if folder == source_root:
                break
            parent = parent.parent

    return sorted(folders)


def iter_ue_folders(ue_files: list[Path], ue_root: Path) -> list[str]:
    folders: set[str] = set()
    for path in ue_files:
        parent = Path(relative(path, ue_root)).parent
        while True:
            folder = parent.as_posix()
            if folder == ".":
                break
            folders.add(folder)
            parent = parent.parent

    return sorted(folders)


def normalize_name(name: str) -> str:
    stripped = re.sub(r"^(F|E|U|I)(?=[A-Z])", "", name)
    return re.sub(r"[^a-z0-9]", "", stripped.lower())


def stem_key(path: str) -> str:
    return normalize_name(Path(path).stem)


def folder_key(path: str) -> str:
    return normalize_name(Path(path).name)


def extract_ts_node_keys(path: Path) -> list[str]:
    text = read_text(path)
    match = re.search(
        r"NODE_CLI_COMMAND_KEYS\s*=\s*\[(?P<body>.*?)\]\s+as\s+const",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find NODE_CLI_COMMAND_KEYS in {path}")
    return re.findall(r"'([^']+)'", match.group("body"))


def extract_ue_node_keys(path: Path) -> list[str]:
    text = read_text(path)
    match = re.search(
        r"BEGIN_NODE_CLI_COMMAND_KEYS(?P<body>.*?)END_NODE_CLI_COMMAND_KEYS",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find UE node CLI command marker block in {path}")
    return re.findall(r'\{\s*TEXT\("([^"]+)"\)', match.group("body"))


def strip_comments(text: str) -> str:
    without_block = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", without_block)


def split_names(body: str) -> list[str]:
    names: list[str] = []
    for raw in body.split(","):
        item = raw.strip()
        if not item:
            continue
        item = re.sub(r"\s+as\s+\w+$", "", item)
        item = item.split(":")[-1].strip()
        item = re.sub(r"=.*$", "", item).strip()
        if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", item):
            names.append(item)
    return names


def extract_ts_symbols(path: Path, ts_root: Path) -> list[Symbol]:
    rel = relative(path, ts_root)
    if path.name == "index.ts":
        return []

    text = strip_comments(read_text(path))
    symbols: list[Symbol] = []

    patterns = (
        (r"\bexport\s+(?:async\s+)?function\s+([A-Za-z_][A-Za-z0-9_]*)", "function"),
        (r"\bexport\s+const\s+([A-Za-z_][A-Za-z0-9_]*)\b", "const"),
        (r"\bexport\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", "class"),
        (r"\bexport\s+interface\s+([A-Za-z_][A-Za-z0-9_]*)", "interface"),
        (r"\bexport\s+type\s+([A-Za-z_][A-Za-z0-9_]*)", "type"),
        (r"\bexport\s+enum\s+([A-Za-z_][A-Za-z0-9_]*)", "enum"),
    )
    for pattern, kind in patterns:
        symbols.extend(Symbol(match, kind, rel) for match in re.findall(pattern, text))

    for body in re.findall(r"\bexport\s+const\s+\{(?P<body>.*?)\}\s*=", text, re.S):
        symbols.extend(Symbol(name, "const", rel) for name in split_names(body))

    for body in re.findall(r"\bexport\s+\{(?P<body>.*?)\}", text, re.S):
        symbols.extend(Symbol(name, "re-export", rel) for name in split_names(body))

    return sorted({symbol.name: symbol for symbol in symbols}.values(), key=lambda item: item.name)


def extract_ue_symbols(path: Path, root: Path) -> list[Symbol]:
    if path.suffix not in {".h", ".hpp", ".cpp"}:
        return []

    rel = relative(path, root)
    text = strip_comments(read_text(path))
    symbols: list[Symbol] = []

    lines = text.splitlines()
    for index, line in enumerate(lines):
        indent = len(line) - len(line.lstrip(" "))
        if indent > 2:
            continue
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or stripped.startswith("template"):
            continue

        type_match = re.match(r"(?:USTRUCT\([^)]*\)\s*)?(?:struct|class)\s+(?:FORBOCAI_SDK_API\s+)?([A-Za-z_][A-Za-z0-9_]*)", stripped)
        enum_match = re.match(r"enum\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", stripped)
        if type_match:
            symbols.append(Symbol(type_match.group(1), "type", rel))
            continue
        if enum_match:
            symbols.append(Symbol(enum_match.group(1), "enum", rel))
            continue

        next_stripped = ""
        if "(" not in stripped and index + 1 < len(lines):
            next_line = lines[index + 1]
            next_indent = len(next_line) - len(next_line.lstrip(" "))
            next_stripped = next_line.strip() if next_indent <= 2 else ""
        declaration = f"{stripped} {next_stripped}".strip()

        function_match = re.match(
            r"(?:FORBOCAI_SDK_API\s+)?(?:virtual\s+)?(?:static\s+)?(?:inline\s+)?"
            r"(?:[A-Za-z_][A-Za-z0-9_:<>*&,\s]+\s+)+([A-Za-z_][A-Za-z0-9_]*)\s*\(",
            declaration,
        )
        if function_match:
            name = function_match.group(1)
            if name not in {"if", "for", "while", "switch", "return"}:
                symbols.append(Symbol(name, "function", rel))

    return sorted({f"{symbol.name}:{symbol.kind}:{symbol.file}": symbol for symbol in symbols}.values(), key=lambda item: item.name)


def build_file_mirrors(ts_files: list[Path], ts_root: Path, ue_files: list[Path], ue_root: Path) -> list[FileMirror]:
    ue_rel = [relative(path, ue_root) for path in ue_files]
    ue_by_key: dict[str, list[str]] = {}
    for file in ue_rel:
        ue_by_key.setdefault(stem_key(file), []).append(file)

    mirrors: list[FileMirror] = []
    for path in ts_files:
        rel = relative(path, ts_root)
        if rel in TEST_GAME_SLICE_FILES:
            mirrors.append(
                FileMirror(
                    rel,
                    ("Source/ForbocAI_SDK/Public/TestGame/TestGameSlices.h",),
                    "Analog",
                    "UE collapses the TS test-game feature slices into one header.",
                )
            )
            continue

        manual = MANUAL_FILE_MIRRORS.get(rel)
        if manual:
            status, candidates, notes = manual
            mirrors.append(FileMirror(rel, candidates, status, notes))
            continue

        candidates = tuple(sorted(ue_by_key.get(stem_key(rel), [])))
        mirrors.append(
            FileMirror(
                rel,
                candidates,
                "Exact" if candidates else "Gap",
                "" if candidates else "No UE source file with a matching normalized name is known yet.",
            )
        )
    return mirrors


def build_folder_mirrors(
    ts_folders: list[str],
    ue_folders: list[str],
    file_mirrors: list[FileMirror],
) -> list[FolderMirror]:
    ue_by_key: dict[str, list[str]] = {}
    for folder in ue_folders:
        ue_by_key.setdefault(folder_key(folder), []).append(folder)

    mirrors: list[FolderMirror] = []
    for folder in ts_folders:
        manual = MANUAL_FOLDER_MIRRORS.get(folder)
        if manual:
            status, candidates, notes = manual
            mirrors.append(FolderMirror(folder, candidates, status, notes))
            continue

        exact_candidates = tuple(sorted(ue_by_key.get(folder_key(folder), [])))
        if exact_candidates:
            mirrors.append(FolderMirror(folder, exact_candidates, "Exact", ""))
            continue

        derived_candidates = tuple(sorted({
            Path(ue_file).parent.as_posix()
            for mirror in file_mirrors
            if mirror.ts_file.startswith(f"{folder}/")
            for ue_file in mirror.ue_files
            if Path(ue_file).parent.as_posix() != "."
        }))
        mirrors.append(
            FolderMirror(
                folder,
                derived_candidates,
                "Analog" if derived_candidates else "Gap",
                "Derived from descendant file mirrors." if derived_candidates else "No UE source folder with a matching normalized name is known yet.",
            )
        )

    return mirrors


def symbol_index(symbols: list[Symbol]) -> dict[str, list[Symbol]]:
    index: dict[str, list[Symbol]] = {}
    for symbol in symbols:
        index.setdefault(normalize_name(symbol.name), []).append(symbol)
    return index


def build_symbol_mirrors(ts_symbols: list[Symbol], ue_symbols: list[Symbol]) -> list[SymbolMirror]:
    ue_by_name = symbol_index(ue_symbols)
    ue_by_exact_name: dict[str, list[Symbol]] = {}
    for symbol in ue_symbols:
        ue_by_exact_name.setdefault(symbol.name, []).append(symbol)

    mirrors: list[SymbolMirror] = []
    for symbol in ts_symbols:
        manual = MANUAL_SYMBOL_MIRRORS.get(symbol.name)
        if manual:
            status, names, notes = manual
            candidates = tuple(
                candidate
                for name in names
                for candidate in ue_by_exact_name.get(name, ue_by_name.get(normalize_name(name), []))
            )
            mirrors.append(SymbolMirror(symbol, candidates, status, notes))
            continue

        candidates = tuple(sorted(ue_by_name.get(normalize_name(symbol.name), []), key=lambda item: (item.file, item.name)))
        mirrors.append(
            SymbolMirror(
                symbol,
                candidates,
                "Exact" if candidates else "Gap",
                "" if candidates else "No UE symbol with the same normalized name is known yet.",
            )
        )
    return mirrors


def used_ue_files(file_mirrors: list[FileMirror]) -> set[str]:
    return {ue_file for mirror in file_mirrors for ue_file in mirror.ue_files}


def used_ue_folders(folder_mirrors: list[FolderMirror]) -> set[str]:
    return {ue_folder for mirror in folder_mirrors for ue_folder in mirror.ue_folders}


def used_ue_symbols(symbol_mirrors: list[SymbolMirror]) -> set[tuple[str, str]]:
    return {(symbol.name, symbol.file) for mirror in symbol_mirrors for symbol in mirror.ue_symbols}


def md(value: str) -> str:
    escaped = value.replace("|", "\\|").replace("\n", " ")
    return escaped if escaped else "-"


def md_code(value: str) -> str:
    return f"`{md(value)}`" if value else "-"


def format_list(values: list[str] | tuple[str, ...]) -> str:
    return "<br>".join(md_code(value) for value in values) if values else "-"


def format_symbol_list(values: tuple[Symbol, ...]) -> str:
    if not values:
        return "-"
    return "<br>".join(md_code(f"{value.name} ({value.file})") for value in values)


def table(headers: tuple[str, ...], rows: list[tuple[str, ...]]) -> list[str]:
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join(":---" for _ in headers) + " |",
    ]
    lines.extend("| " + " | ".join(row) + " |" for row in rows)
    return lines


def package_for_ts_file(path: str) -> str:
    if path.startswith("packages/core/"):
        return "core"
    if path.startswith("packages/node/"):
        return "node-cli" if "/cli" in path else "node"
    if path.startswith("packages/test-game/"):
        return "test-game"
    return "other"


def package_label(package: str) -> str:
    labels = {
        "core": "Core",
        "node": "Node",
        "node-cli": "Node CLI",
        "test-game": "Test Game",
        "other": "Other",
    }
    return labels.get(package, package)


def package_order(package: str) -> int:
    order = {"core": 0, "node": 1, "node-cli": 2, "test-game": 3, "other": 4}
    return order.get(package, 99)


def grouped_file_mirrors(file_mirrors: list[FileMirror]) -> list[tuple[str, list[FileMirror]]]:
    packages = sorted(
        {package_for_ts_file(mirror.ts_file) for mirror in file_mirrors},
        key=package_order,
    )
    return [
        (
            package,
            [
                mirror
                for mirror in file_mirrors
                if package_for_ts_file(mirror.ts_file) == package
            ],
        )
        for package in packages
    ]


def grouped_folder_mirrors(folder_mirrors: list[FolderMirror]) -> list[tuple[str, list[FolderMirror]]]:
    packages = sorted(
        {package_for_ts_file(mirror.ts_folder) for mirror in folder_mirrors},
        key=package_order,
    )
    return [
        (
            package,
            [
                mirror
                for mirror in folder_mirrors
                if package_for_ts_file(mirror.ts_folder) == package
            ],
        )
        for package in packages
    ]


def grouped_symbol_mirrors(symbol_mirrors: list[SymbolMirror]) -> list[tuple[str, list[SymbolMirror]]]:
    packages = sorted(
        {package_for_ts_file(mirror.ts_symbol.file) for mirror in symbol_mirrors},
        key=package_order,
    )
    return [
        (
            package,
            [
                mirror
                for mirror in symbol_mirrors
                if package_for_ts_file(mirror.ts_symbol.file) == package
            ],
        )
        for package in packages
    ]


def file_mirror_rows(file_mirrors: list[FileMirror]) -> list[tuple[str, ...]]:
    return [
        (md_code(mirror.ts_file), format_list(mirror.ue_files), mirror.status, md(mirror.notes))
        for mirror in file_mirrors
    ]


def folder_mirror_rows(folder_mirrors: list[FolderMirror]) -> list[tuple[str, ...]]:
    return [
        (md_code(mirror.ts_folder), format_list(mirror.ue_folders), mirror.status, md(mirror.notes))
        for mirror in folder_mirrors
    ]


def symbol_mirror_rows(symbol_mirrors: list[SymbolMirror]) -> list[tuple[str, ...]]:
    return [
        (
            md_code(f"{mirror.ts_symbol.name} ({mirror.ts_symbol.file})"),
            format_symbol_list(mirror.ue_symbols),
            mirror.status,
            md(mirror.notes),
        )
        for mirror in symbol_mirrors
    ]


def package_summary_rows(
    folder_mirrors: list[FolderMirror],
    file_mirrors: list[FileMirror],
    symbol_mirrors: list[SymbolMirror],
) -> list[tuple[str, ...]]:
    packages = sorted(
        {
            *{package_for_ts_file(mirror.ts_folder) for mirror in folder_mirrors},
            *{package_for_ts_file(mirror.ts_file) for mirror in file_mirrors},
            *{package_for_ts_file(mirror.ts_symbol.file) for mirror in symbol_mirrors},
        },
        key=package_order,
    )
    rows: list[tuple[str, ...]] = []
    for package in packages:
        folders = [mirror for mirror in folder_mirrors if package_for_ts_file(mirror.ts_folder) == package]
        files = [mirror for mirror in file_mirrors if package_for_ts_file(mirror.ts_file) == package]
        symbols = [
            mirror
            for mirror in symbol_mirrors
            if package_for_ts_file(mirror.ts_symbol.file) == package
        ]
        rows.append(
            (
                package_label(package),
                str(len(folders)),
                str(sum(1 for mirror in folders if mirror.status == "Exact")),
                str(sum(1 for mirror in folders if mirror.status == "Analog")),
                str(sum(1 for mirror in folders if mirror.status == "Gap")),
                str(len(files)),
                str(sum(1 for mirror in files if mirror.status == "Exact")),
                str(sum(1 for mirror in files if mirror.status == "Analog")),
                str(sum(1 for mirror in files if mirror.status == "Gap")),
                str(len(symbols)),
                str(sum(1 for mirror in symbols if mirror.status == "Exact")),
                str(sum(1 for mirror in symbols if mirror.status == "Analog")),
                str(sum(1 for mirror in symbols if mirror.status == "Gap")),
            )
        )
    return rows


def build_generated_section(
    ts_root: Path,
    ue_root: Path,
    ts_folders: list[str],
    ue_folders: list[str],
    folder_mirrors: list[FolderMirror],
    ts_files: list[Path],
    ue_files: list[Path],
    file_mirrors: list[FileMirror],
    ts_symbols: list[Symbol],
    ue_symbols: list[Symbol],
    symbol_mirrors: list[SymbolMirror],
    ts_keys: list[str],
    ue_keys: list[str],
) -> str:
    generated_on = date.today().isoformat()
    folder_gaps = [mirror for mirror in folder_mirrors if mirror.status == "Gap"]
    analog_folders = [mirror for mirror in folder_mirrors if mirror.status == "Analog"]
    exact_folders = [mirror for mirror in folder_mirrors if mirror.status == "Exact"]
    file_gaps = [mirror for mirror in file_mirrors if mirror.status == "Gap"]
    analog_files = [mirror for mirror in file_mirrors if mirror.status == "Analog"]
    exact_files = [mirror for mirror in file_mirrors if mirror.status == "Exact"]
    symbol_gaps = [mirror for mirror in symbol_mirrors if mirror.status == "Gap"]
    analog_symbols = [mirror for mirror in symbol_mirrors if mirror.status == "Analog"]
    exact_symbols = [mirror for mirror in symbol_mirrors if mirror.status == "Exact"]
    used_folders = used_ue_folders(folder_mirrors)
    ue_only_folders = [folder for folder in ue_folders if folder not in used_folders]
    used_files = used_ue_files(file_mirrors)
    ue_only_files = [relative(path, ue_root) for path in ue_files if relative(path, ue_root) not in used_files]
    used_symbols = used_ue_symbols(symbol_mirrors)
    ue_only_symbols = [
        symbol
        for symbol in ue_symbols
        if (symbol.name, symbol.file) not in used_symbols
    ]

    lines: list[str] = [
        GENERATED_START,
        "## Generated SDK Parity Inventory",
        "",
        f"Generated by `scripts/check-sdk-parity.py --update-map` on {generated_on}.",
        "",
        "This section is source-derived. Edit the script or source names, then regenerate this block instead of hand-editing it.",
        "",
        "### Generated Summary",
        "",
        *table(
            ("Metric", "Count"),
            [
                ("TS source folders scanned", str(len(ts_folders))),
                ("UE source folders scanned", str(len(ue_folders))),
                ("TS folders with exact UE mirror", str(len(exact_folders))),
                ("TS folders with analog UE mirror", str(len(analog_folders))),
                ("TS folders with no UE mirror yet", str(len(folder_gaps))),
                ("UE-only folders", str(len(ue_only_folders))),
                ("TS source files scanned", str(len(ts_files))),
                ("UE source files scanned", str(len(ue_files))),
                ("TS files with exact UE mirror", str(len(exact_files))),
                ("TS files with analog UE mirror", str(len(analog_files))),
                ("TS files with no UE mirror yet", str(len(file_gaps))),
                ("UE-only files", str(len(ue_only_files))),
                ("TS exported symbols scanned", str(len(ts_symbols))),
                ("TS symbols with exact UE name mirror", str(len(exact_symbols))),
                ("TS symbols with analog UE mirror", str(len(analog_symbols))),
                ("TS symbols with no UE symbol yet", str(len(symbol_gaps))),
                ("UE-only symbols", str(len(ue_only_symbols))),
                ("Node CLI command keys", str(len(ts_keys))),
            ],
        ),
        "",
        f"- TS root: `{ts_root}`",
        f"- UE root: `{ue_root}`",
        "- `Exact` means the normalized folder, file, or symbol name already mirrors.",
        "- `Analog` means the responsibility mirrors but the path/name shape still differs.",
        "- `Gap` means the TS item has no UE mirror recorded yet.",
        "- Function names are included in the generated symbol mirrors.",
        "",
        "### Generated Package Coverage",
        "",
        *table(
            (
                "Area",
                "TS folders",
                "Folder exact",
                "Folder analog",
                "Folder gaps",
                "TS files",
                "File exact",
                "File analog",
                "File gaps",
                "TS symbols",
                "Symbol exact",
                "Symbol analog",
                "Symbol gaps",
            ),
            package_summary_rows(folder_mirrors, file_mirrors, symbol_mirrors),
        ),
        "",
        "### Generated Folder Mirrors",
        "",
    ]

    for package, mirrors in grouped_folder_mirrors(folder_mirrors):
        lines.extend(
            [
                f"#### {package_label(package)} Folder Mirrors",
                "",
                *table(
                    ("TypeScript folder", "UE folder(s)", "Status", "Notes"),
                    folder_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
        "### Generated UE-Only Folders",
        "",
        *table(
            ("UE folder", "Notes"),
            [(md_code(path), "No TS source folder currently maps to this UE folder.") for path in ue_only_folders],
        ),
        "",
        "### Generated File Mirrors",
        "",
        ]
    )

    for package, mirrors in grouped_file_mirrors(file_mirrors):
        lines.extend(
            [
                f"#### {package_label(package)} File Mirrors",
                "",
                *table(
                    ("TypeScript file", "UE file(s)", "Status", "Notes"),
                    file_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
        "### Generated UE-Only Files",
        "",
        *table(
            ("UE file", "Notes"),
            [(md_code(path), "No TS source file currently maps to this UE file.") for path in ue_only_files],
        ),
        "",
        "### Generated Symbol Mirrors",
        "",
        ]
    )

    for package, mirrors in grouped_symbol_mirrors(symbol_mirrors):
        lines.extend(
            [
                f"#### {package_label(package)} Symbol Mirrors",
                "",
                *table(
                    ("TypeScript symbol", "UE symbol(s)", "Status", "Notes"),
                    symbol_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "### Generated UE-Only Symbols",
        "",
        *table(
            ("UE symbol", "Notes"),
            [
                (
                    md_code(f"{symbol.name} ({symbol.file})"),
                    "No exported TS symbol currently maps to this UE symbol.",
                )
                for symbol in ue_only_symbols
            ],
        ),
        "",
        "### Generated Node CLI Command Keys",
        "",
        *table(
            ("Order", "TS key", "UE key", "Status"),
            [
                (
                    str(index + 1),
                    md_code(ts_key),
                    md_code(ue_keys[index]) if index < len(ue_keys) else "-",
                    "Exact" if index < len(ue_keys) and ts_key == ue_keys[index] else "Gap",
                )
                for index, ts_key in enumerate(ts_keys)
            ],
        ),
        GENERATED_END,
        ]
    )
    return "\n".join(lines) + "\n"


def replace_generated_section(text: str, section: str) -> str:
    if GENERATED_START in text and GENERATED_END in text:
        pattern = re.compile(
            re.escape(GENERATED_START) + r".*?" + re.escape(GENERATED_END) + r"\n?",
            re.S,
        )
        return pattern.sub(section, text)

    insert_before = "\n## 11. Maintenance Rule"
    if insert_before in text:
        return text.replace(insert_before, "\n" + section + insert_before, 1)

    return text.rstrip() + "\n\n" + section


def update_last_updated(text: str) -> str:
    today = date.today().isoformat()
    return re.sub(r"Last updated:\s*\d{4}-\d{2}-\d{2}", f"Last updated: {today}", text, count=1)


def check_cli_command_parity(ts_root: Path, ue_root: Path) -> tuple[int, list[str], list[str]]:
    ts_matrix = ts_root / "packages/core/src/cliCommandMatrix.ts"
    ue_matrix = ue_root / "Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h"

    ts_keys = extract_ts_node_keys(ts_matrix)
    ue_keys = extract_ue_node_keys(ue_matrix)

    missing = [key for key in ts_keys if key not in ue_keys]
    extra = [key for key in ue_keys if key not in ts_keys]
    order_matches = ts_keys == ue_keys

    print("[check] UE SDK parity")
    print(f"[info] TS SDK root: {ts_root}")
    print(f"[info] UE SDK root: {ue_root}")
    print(f"[info] TS CLI matrix: {ts_matrix}")
    print(f"[info] UE CLI matrix: {ue_matrix}")

    if not missing and not extra and order_matches:
        print(f"[ok] UE mirrors TS NODE_CLI_COMMAND_KEYS ({len(ts_keys)} commands)")
        return 0, ts_keys, ue_keys

    if missing:
        print("[fail] Commands present in TS but missing in UE:")
        print("\n".join(f"  - {value}" for value in missing))
    if extra:
        print("[fail] Commands present in UE node-parity block but absent from TS:")
        print("\n".join(f"  - {value}" for value in extra))
    if not order_matches and not missing and not extra:
        print("[fail] Command sets match, but order differs from TS matrix.")

    print("")
    print("[hint] Update Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h")
    print("       so the BEGIN_NODE_CLI_COMMAND_KEYS block mirrors TS exactly.")
    return 1, ts_keys, ue_keys


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare TS SDK and UE SDK files, symbols, CLI keys, and generated map inventory."
    )
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repo root.")
    parser.add_argument("--map-path", help="Path to classified/docs/design/sdk/map.md.")
    parser.add_argument("--runtime-root", help=argparse.SUPPRESS)
    parser.add_argument("--update-map", action="store_true", help="Rewrite the generated inventory section in map.md.")
    parser.add_argument("--require-map", action="store_true", help="Fail when map.md cannot be found.")
    args = parser.parse_args()

    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, args.ts_sdk_root)
    map_path = resolve_map_path(ue_root, args.map_path)

    cli_status, ts_keys, ue_keys = check_cli_command_parity(ts_root, ue_root)

    ts_files = iter_ts_files(ts_root)
    ue_files = iter_ue_files(ue_root)
    ts_folders = iter_ts_folders(ts_files, ts_root)
    ue_folders = iter_ue_folders(ue_files, ue_root)
    file_mirrors = build_file_mirrors(ts_files, ts_root, ue_files, ue_root)
    folder_mirrors = build_folder_mirrors(ts_folders, ue_folders, file_mirrors)

    ts_symbols = [
        symbol
        for path in ts_files
        for symbol in extract_ts_symbols(path, ts_root)
    ]
    ue_symbols = [
        symbol
        for path in ue_files
        for symbol in extract_ue_symbols(path, ue_root)
    ]
    symbol_mirrors = build_symbol_mirrors(ts_symbols, ue_symbols)

    section = build_generated_section(
        ts_root,
        ue_root,
        ts_folders,
        ue_folders,
        folder_mirrors,
        ts_files,
        ue_files,
        file_mirrors,
        ts_symbols,
        ue_symbols,
        symbol_mirrors,
        ts_keys,
        ue_keys,
    )

    folder_gap_count = sum(1 for mirror in folder_mirrors if mirror.status == "Gap")
    file_gap_count = sum(1 for mirror in file_mirrors if mirror.status == "Gap")
    symbol_gap_count = sum(1 for mirror in symbol_mirrors if mirror.status == "Gap")
    print(f"[info] TS source folders scanned: {len(ts_folders)}")
    print(f"[info] UE source folders scanned: {len(ue_folders)}")
    print(f"[info] TS folders without recorded UE mirror: {folder_gap_count}")
    print(f"[info] TS source files scanned: {len(ts_files)}")
    print(f"[info] UE source files scanned: {len(ue_files)}")
    print(f"[info] TS files without recorded UE mirror: {file_gap_count}")
    print(f"[info] TS exported symbols without UE mirror: {symbol_gap_count}")

    map_status = 0
    if map_path is None:
        message = "[warn] map.md not found; generated inventory freshness was not checked."
        print(message)
        map_status = 1 if args.require_map else 0
    elif not map_path.exists():
        print(f"[fail] map.md not found: {map_path}")
        map_status = 1
    else:
        original = read_text(map_path)
        updated = update_last_updated(replace_generated_section(original, section))
        if args.update_map:
            map_path.write_text(updated, encoding="utf-8", newline="\n")
            print(f"[ok] Updated generated SDK parity inventory in {map_path}")
        elif original != updated:
            print(f"[fail] Generated SDK parity inventory is stale in {map_path}")
            print("[hint] Run: python3 scripts/check-sdk-parity.py --update-map")
            map_status = 1
        else:
            print(f"[ok] Generated SDK parity inventory is current in {map_path}")

    return 1 if cli_status or map_status else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[fail] {exc}", file=sys.stderr)
        raise SystemExit(1)
