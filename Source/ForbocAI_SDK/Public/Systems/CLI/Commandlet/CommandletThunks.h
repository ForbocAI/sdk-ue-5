#pragma once

#include "Systems/CLI/CLIThunks.h"
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CommandletThunks.generated.h"

/**
 * ForbocAI Commandlet.
 * User Story: As Unreal command-line automation, I need one commandlet entry
 * point so SDK operations can run from editor and CI command invocations.
 * Usage:
 *   scripts/forbocai-ue <command> [subcommand] [args]
 *
 * Canonical commands:
 *   doctor
 *   npc create "..."
 *   npc process <id> "..."
 *   soul export <id>
 *   config set <key> <value>
 *   config get <key>
 */
UCLASS()
class UForbocAICommandlet : public UCommandlet {
  GENERATED_BODY()

public:
  /**
   * Constructs the ForbocAI commandlet with its metadata defaults.
   * User Story: As editor command execution, I need the commandlet initialized
   * with predictable defaults before parsing CLI input.
   * @fn UForbocAICommandlet()
   */
  UForbocAICommandlet();

  /**
   * Runs the requested CLI command from raw commandlet parameters.
   * User Story: As commandlet execution, I need raw params translated into a
   * command result so Unreal CLI entrypoints can drive SDK operations.
   * @fn virtual int32 Main(const FString &Params) override
   */
  virtual int32 Main(const FString &Params) override;
};
