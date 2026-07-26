#include "MicroGame/CLI/MicroGameCommandlet.h"

#include "MicroGame/Features/Systems/CLI/CLIThunks.h"
#include "MicroGame/MicroGameStore.h"
#include "MicroGame/Views/Terminal/TerminalView.h"

/** User Story: As a micro-game CLI host, I need an Unreal commandlet configured for unattended console execution so the same feature flow runs on development and CI machines. @fn UForbocAIMicroGameCommandlet::UForbocAIMicroGameCommandlet() */
UForbocAIMicroGameCommandlet::UForbocAIMicroGameCommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/** User Story: As a micro-game CLI host, I need commandlet entry delegated to the CLI feature so this integration layer only wires Unreal presentation. @fn int32 UForbocAIMicroGameCommandlet::Main(const FString &Params) */
int32 UForbocAIMicroGameCommandlet::Main(const FString &Params) {
  MicroGame::FMicroGameStore Store = MicroGame::createMicroGameStore();
  return MicroGame::CLI::runCli(
      Params, Store,
      [](const MicroGame::FTerminalProgressViewModel &ViewModel) {
        MicroGame::PresentProgress(ViewModel);
      });
}
