#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Decision/DecisionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Generation/GenerationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Transcript/Layout/LayoutAdapters.h"
#include "MicroGame/Features/Systems/Harness/MazeGhost/Definition/MazeGhostDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Vocabulary/VerificationVocabularyAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Testing/Harness/MazeGhost/Fixture/MazeGhostFixtureAdapters.h"

namespace {

using MicroGame::Maze::FMazeCommands;

/** User Story: As a systems harness maze consumer, I need to invoke command values through a stable signature so the systems harness maze workflow remains explicit and composable. @fn TArray<FString> CommandValues(const FMazeCommands &Commands) */
TArray<FString> CommandValues(const FMazeCommands &Commands) {
  return {Commands.Generate, Commands.Create, Commands.IdentityUpdate,
          Commands.Recall, Commands.Decide, Commands.MemoryStore,
          Commands.TypedMemoryStore};
}

/** User Story: As a systems harness maze consumer, I need to invoke uses only ghost commands through a stable signature so the systems harness maze workflow remains explicit and composable. @fn bool UsesOnlyGhostCommands(const TArray<FString> &Commands) */
bool UsesOnlyGhostCommands(const TArray<FString> &Commands) {
  const auto &Data =
      MicroGame::Testing::MazeGhost::MazeGhostTestingData();
  return func::all_array<FString>(
      Commands, [&Data](const FString &Command) {
        return Command.StartsWith(Data.CommandPrefix) &&
               !Command.Contains(Data.ForbiddenFragment,
                                 ESearchCase::IgnoreCase);
      });
}

/** User Story: As a systems harness maze consumer, I need to invoke has reciprocal passages through a stable signature so the systems harness maze workflow remains explicit and composable. @fn bool HasReciprocalPassages(const MicroGame::Maze::FMaze &Maze) */
bool HasReciprocalPassages(const MicroGame::Maze::FMaze &Maze) {
  const auto &Data = MicroGame::Maze::MazeConfig();
  return func::all_array<FString>(
      Maze.CellOrder, [&Data, &Maze](const FString &Cell) {
        const TMap<FString, FString> &Passages =
            Maze.Cells.FindChecked(Cell);
        return func::all_array<FString>(
            Data.Directions,
            [&Data, &Maze, &Cell,
             &Passages](const FString &Direction) {
              const FString *Neighbor = Passages.Find(Direction);
              return Neighbor == nullptr ||
                     Maze.Cells.FindChecked(*Neighbor).FindChecked(
                         Data.Opposites.FindChecked(Direction)) == Cell;
            });
      });
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMazeGhostSharedBenchmarkTest,
    MicroGame::Testing::MazeGhost::MazeGhostTestingData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a cross-host maintainer, I need Maze and Maze Ghost proven to share one deterministic topology and differ only through injected Ghost transport commands. @fn bool FMazeGhostSharedBenchmarkTest::RunTest(const FString &Parameters) */
bool FMazeGhostSharedBenchmarkTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  using namespace MicroGame;
  const auto &Fixture = Testing::MazeGhost::MazeGhostTestingData();
  const auto &Labels = Fixture.Labels;
  const Maze::FMazeConfig &MazeData = Maze::MazeConfig();
  const MazeGhost::FMazeGhostConfig &GhostData =
      MazeGhost::MazeGhostConfig();
  const TArray<FString> MazeCommands = CommandValues(MazeData.Commands);
  const TArray<FString> GhostCommands = CommandValues(GhostData.Commands);
  const TArray<FString> GhostLifecycle = {
      GhostData.RunCommand, GhostData.StopCommand, GhostData.ResultsCommand};

  TestEqual(*Labels.OperationShape, MazeCommands.Num(),
            Fixture.ExpectedOperationCount);
  TestEqual(*Labels.OperationShape, GhostCommands.Num(),
            MazeCommands.Num());
  TestEqual(*Labels.OperationShape, GhostLifecycle.Num(),
            Fixture.ExpectedLifecycleCommandCount);
  TestTrue(*Labels.GhostOnly,
           UsesOnlyGhostCommands(GhostCommands) &&
               UsesOnlyGhostCommands(GhostLifecycle));

  const Maze::FMaze First = Maze::GenerateMaze(Fixture.Seed);
  const Maze::FMaze Second = Maze::GenerateMaze(Fixture.Seed);
  TestEqual(*Labels.Deterministic,
            FString::Join(Maze::FormatMazeLayout(First),
                          *MazeData.OutputLineSeparator),
            FString::Join(Maze::FormatMazeLayout(Second),
                          *MazeData.OutputLineSeparator));
  TestTrue(*Labels.Topology,
           First.Cells.Contains(First.Start) &&
               First.Cells.Contains(First.Exit) &&
               First.DeadEnds.Num() == MazeData.DeadEndCount);
  TestTrue(*Labels.Reciprocal, HasReciprocalPassages(First));
  TestEqual(*Labels.Distance, First.Distances.FindChecked(First.Start),
            First.Optimal);
  TestTrue(*Labels.Ward,
           !Maze::MeasureWardOutcome(First, First.Start, {})
                .WardDirections.IsEmpty());

  const FString Usage = FString::Join(
      TerminalAdapters::TerminalData().usage, *Fixture.UsageSeparator);
  const TArray<FString> RuntimeModes =
      VerificationVocabularyAdapters::GameRuntimeData().modes.all.Array();
  TestTrue(*Labels.ModeVocabulary,
           func::all_array<FString>(
               RuntimeModes,
               [&Usage](const FString &Mode) {
                 return Usage.Contains(Mode);
               }));
  return true;
}
