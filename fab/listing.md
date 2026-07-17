# ForbocAI NPC SDK

Persistent, rules-aware NPC intelligence for Unreal Engine 5.8

## Description

ForbocAI NPC SDK gives Unreal Engine teams a production-oriented C++ and Blueprint surface for game-defined NPCs that can respond to players and world events without baking a genre, setting, role, or personality into the model.

Your game owns identity and context. The SDK owns local vector memory, immutable runtime state, rules-aware action validation, portable soul data, and the integration boundary to the hosted ForbocAI NPC model.

The plugin includes source, Blueprint-callable APIs, a thin CLI for diagnostics, and authored configuration. The public SDK repository maintains a full end-to-end quality harness that exercises the CLI, SDK, production API, and model together.

## Features

- Game-defined NPC identity and context
- Genre-agnostic hosted NPC inference
- Persistent local SQLite vector memory
- Rules-aware action validation bridge
- Portable soul import and export
- C++ and Blueprint-callable APIs
- RTK-style single-store state ownership
- Functional immutable operations
- CLI diagnostics and API health checks
- Source included

## Technical Details

- Engine version: Unreal Engine 5.8
- Code modules: 1 (Runtime)
- Tested platforms: Win64
- Network: HTTPS access to api.forboc.ai is required for NPC inference
- Account: A ForbocAI account and API key are required for hosted inference
- Local storage: SQLite-backed vector memory is included in the plugin
- Dependencies: Uses Unreal Engine HTTP, JSON, JsonUtilities, Projects, and OpenSSL modules

## Pricing

- License: Fab Standard License
- Personal: $49.99
- Professional: $149.99

## Support

- Documentation: https://docs.forboc.ai
- Support and issues: https://github.com/ForbocAI/sdk-ue-5/issues
- Account: https://account.forboc.ai
- Website: https://forboc.ai

## FAQ

### Does the plugin ship a language model?

No. ForbocAI NPC inference is hosted by the ForbocAI API. The SDK keeps game integration and vector memory local.

### Does ForbocAI decide who an NPC is?

No. Your game supplies the NPC identity, state, rules, memories, and current context for each interaction.

### Can I use it from Blueprints?

Yes. Runtime status, NPC processing, memory, soul, bridge, and configuration operations have Blueprint-callable entry points.

### Is an internet connection required?

Yes for hosted NPC inference. Local memory and immutable state operations remain in the plugin.

### Can I evaluate hosted inference before choosing a paid plan?

Yes. New ForbocAI accounts begin on the free Initiate tier. Higher-volume plans are managed through the ForbocAI account portal.
