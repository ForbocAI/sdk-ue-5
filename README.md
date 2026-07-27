# ForbocAI NPC SDK for Unreal Engine 5.8

Add ForbocAI dialogue, persistent semantic memory, validated actions, Ghost
sessions, and portable Soul storage to Unreal Engine games from C++ or
Blueprints.

- Documentation: <https://docs.forboc.ai/ue/welcome>
- Account and API keys: <https://account.forboc.ai>
- Example game: <https://github.com/ForbocAI/demo-ue-5>

## Requirements

| Platform | Requirements |
| --- | --- |
| Windows | Unreal Engine 5.8 and Visual Studio 2022 C++ tools |
| macOS | Unreal Engine 5.8 and Xcode 15 or newer |
| Linux | Unreal Engine 5.8 and Clang 16 or newer |

## Install

Install **ForbocAI NPC SDK** from Fab, or place this repository at:

```text
YourGame/Plugins/ForbocAI_SDK/
```

Enable `ForbocAI_SDK` in the Plugins window and add it to the game module:

```csharp
PublicDependencyModuleNames.Add("ForbocAI_SDK");
```

Regenerate project files and build the `Development Editor` target.

## C++ Quick Start

Initialize the supplied game-instance subsystem once and use its Blueprint and
C++ operations throughout the game instance.

```cpp
#include "Systems/Integration/Unreal/Blueprint/BlueprintThunks.h"
#include "Systems/Integration/Unreal/Subsystem/SubsystemThunks.h"

void UMyNpcComponent::InitializeForbocAI(
    const FString& ApiKey,
    const FString& StructuredPersonaJson)
{
    UForbocAISubsystem* SDK =
        GetWorld()->GetGameInstance()->GetSubsystem<UForbocAISubsystem>();

    SDK->Init(ApiKey);
    NpcId = UForbocAIBlueprintLibrary::createNpc(StructuredPersonaJson);
    SDK->OnMessageReceived.AddDynamic(
        this, &UMyNpcComponent::HandleNpcDialogue);
    SDK->OnNPCActionReceived.AddDynamic(
        this, &UMyNpcComponent::HandleNpcAction);
}

void UMyNpcComponent::SendNpcInput(const FString& Input)
{
    GetWorld()->GetGameInstance()
        ->GetSubsystem<UForbocAISubsystem>()
        ->ProcessNPC(NpcId, Input);
}
```

Declare the delegate handlers as `UFUNCTION()` methods. Render dialogue in your
UI and execute an action only after your game has accepted the validated result.

## Blueprint Quick Start

1. Get the `ForbocAI Subsystem` from the game instance and call `Init` once.
2. Call `Create Npc` with structured persona JSON supplied by your game data.
3. Keep the returned NPC ID on the NPC actor or component.
4. Bind `On Message Received`, `On NPC Action Received`, `On Typing Start`, and
   `On Typing End` as needed.
5. Call `Process NPC` with the NPC ID and the current player or world input.

The same public library also exposes memory, Bridge, Ghost, Soul, and config
operations for Blueprint-first projects.

## Runtime Ownership

- The ForbocAI API owns NPC inference, orchestration, decision policy, and
  output validation.
- The plugin wraps the API and executes local vector-memory and permanent Soul
  storage effects requested by the API.
- Your game owns NPC identity placement, world context, action application,
  presentation, animation, audio, and save data.

Credentials stay at runtime boundaries. Use `FORBOCAI_API_KEY` or pass the key
to `UForbocAISubsystem::Init`; do not place credentials in reflected assets,
game state, logs, or source control.

## Support

- UE integration guides: <https://docs.forboc.ai/ue/welcome>
- SDK issues: <https://github.com/ForbocAI/sdk-ue-5/issues>
- Community: <https://discord.gg/6hr2jHqnsG>

## License

Fab acquisitions use the Fab Standard License. Other distribution channels
require a separate ForbocAI license. See [LICENSE](./LICENSE).
