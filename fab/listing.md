# ForbocAI NPC SDK

Generic NPC interaction for Unreal Engine 5.8

## Description

ForbocAI NPC SDK gives Unreal Engine teams C++ and Blueprint APIs for NPCs that
respond to players and world events without fixing a genre, setting, identity,
role, or personality in the language model.

Your game supplies NPC identity, structured persona, world context, and action
execution. The ForbocAI API owns NPC reasoning, dialogue, policy, and action
validation. The SDK wraps that API and performs local vector-memory and portable
Soul operations next to the game.

## Features

- Game-defined NPC identity, persona, and context
- Generic hosted ForbocAI NPC inference
- Persistent local SQLite vector memory
- API-validated NPC actions
- Portable Soul import, export, and verification
- C++ and Blueprint APIs
- CLI diagnostics and API health checks
- Plugin source included

## Technical Details

- Engine version: Unreal Engine 5.8
- Code modules: 1 Runtime module
- Platform: Win64
- Network: HTTPS access to `api.forboc.ai` for hosted NPC interactions
- Account: ForbocAI account and API key
- Local storage: SQLite-backed vector memory included
- Unreal modules: HTTP, JSON, JsonUtilities, Projects, and OpenSSL

## Support

- Documentation: https://docs.forboc.ai
- Support and issues: https://github.com/ForbocAI/sdk-ue-5/issues
- Account: https://account.forboc.ai
- Website: https://forboc.ai

## FAQ

### Does the plugin ship a language model?

No. NPC inference runs in the ForbocAI API. The plugin keeps game integration
and vector memory local.

### Does ForbocAI decide who an NPC is?

No. Your game supplies each NPC's identity, structured persona, state, and
current context at runtime.

### Can I use it from Blueprints?

Yes. NPC processing, memory, Soul, action validation, and configuration have
Blueprint-callable entry points.

### Is an internet connection required?

Hosted NPC interactions require network access. Local memory operations remain
inside the plugin.
