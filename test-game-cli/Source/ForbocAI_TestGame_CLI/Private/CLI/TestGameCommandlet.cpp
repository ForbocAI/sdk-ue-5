#include "TestGame/CLI/TestGameCommandlet.h"

#include "TestGame/Features/Systems/CLI/CLIThunks.h"
#include "TestGame/TestGameStore.h"
#include "TestGame/Views/Terminal/TerminalView.h"

/** User Story: As a test-game CLI host, I need an Unreal commandlet configured for unattended console execution so the same feature flow runs on development and CI machines. @fn UForbocAITestGameCommandlet::UForbocAITestGameCommandlet() */
UForbocAITestGameCommandlet::UForbocAITestGameCommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/** User Story: As a test-game CLI host, I need commandlet entry delegated to the CLI feature so this integration layer only wires Unreal presentation. @fn int32 UForbocAITestGameCommandlet::Main(const FString &Params) */
int32 UForbocAITestGameCommandlet::Main(const FString &Params) {
  TestGame::FTestGameStore Store = TestGame::createTestGameStore();
  return TestGame::CLI::runCli(
      Params, Store,
      [](const TestGame::FTerminalProgressViewModel &ViewModel) {
        TestGame::PresentProgress(ViewModel);
      });
}
