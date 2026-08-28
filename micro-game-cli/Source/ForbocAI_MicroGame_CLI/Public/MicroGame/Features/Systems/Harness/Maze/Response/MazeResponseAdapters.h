#pragma once

#include "Core/JsonInterop.h"
#include "Internationalization/Regex.h"
#include "Systems/API/Endpoints/NPC/Process/Configuration/ProcessConfigurationAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze actor generation, I need the generated value read from the authored structured response field. @fn inline FString ReadMazeGeneratedValue(const FString &Output) */
inline FString ReadMazeGeneratedValue(const FString &Output) {
  TSharedPtr<FJsonObject> Root;
  return JsonInterop::ParseJsonObject(Output, Root) && Root.IsValid() &&
                 Root->HasTypedField<EJson::String>(MazeConfig().ValueField)
             ? Root->GetStringField(MazeConfig().ValueField)
             : MazeConfig().EmptyContext;
}

/** User Story: As Maze actor creation, I need the created identity extracted through the authored id pattern. @fn inline FString ReadMazeActorId(const FString &Output) */
inline FString ReadMazeActorId(const FString &Output) {
  FRegexMatcher Matcher(FRegexPattern(MazeConfig().IdPattern), Output);
  return Matcher.FindNext()
             ? Matcher.GetCaptureGroup(MazeConfig().Numbers.Zero)
             : MazeConfig().EmptyContext;
}

/** User Story: As Maze diagnostics, I need one API-owned analytical line selected by its SDK-authored CLI prefix. @fn inline FString ReadAnalysisLine(const FString &Output, const FString &Prefix) */
inline FString ReadAnalysisLine(const FString &Output,
                                const FString &Prefix) {
  TArray<FString> Lines;
  Output.ParseIntoArray(Lines, *MazeConfig().OutputLineSeparator, false);
  const FString *Found = Lines.FindByPredicate(
      [&Prefix](const FString &Line) { return Line.StartsWith(Prefix); });
  return Found == nullptr ? MazeConfig().DialogueFallback
                          : Found->Mid(Prefix.Len());
}

/** User Story: As Maze movement, I need spoken dialogue selected from the first thin CLI response line. @fn inline FString ReadMazeDialogue(const FString &Output) */
inline FString ReadMazeDialogue(const FString &Output) {
  TArray<FString> Lines;
  Output.ParseIntoArray(Lines, *MazeConfig().OutputLineSeparator, false);
  const FString Spoken = Lines.IsValidIndex(MazeConfig().Numbers.Zero)
                             ? Lines[MazeConfig().Numbers.Zero]
                                   .TrimStartAndEnd()
                             : MazeConfig().EmptyContext;
  return Spoken.IsEmpty() ? MazeConfig().DialogueFallback : Spoken;
}

/** User Story: As thought diagnostics, I need the API-owned decision intent retained as its canonical serialized JSON. @fn inline FString ReadMazeThought(const FString &Output) */
inline FString ReadMazeThought(const FString &Output) {
  return ReadAnalysisLine(
      Output, ForbocAI::CLI::NPC::readCliNpcState()
                  .Messages.ThoughtResultPrefix);
}

/** User Story: As reasoning diagnostics, I need the API-owned public rationale selected from its canonical result object without reinterpretation. @fn inline FString ReadMazeReason(const FString &Output) */
inline FString ReadMazeReason(const FString &Output) {
  const FString Serialized = ReadAnalysisLine(
      Output, ForbocAI::CLI::NPC::readCliNpcState()
                  .Messages.ReasoningResultPrefix);
  TSharedPtr<FJsonObject> Object;
  const FString Field =
      APISlice::NPCProcessConfiguration::processContractData()
          .ReasoningOutput.ReasoningText;
  return JsonInterop::ParseJsonObject(Serialized, Object) &&
                 Object.IsValid() &&
                 Object->HasTypedField<EJson::String>(Field)
             ? Object->GetStringField(Field)
             : MazeConfig().DialogueFallback;
}

} // namespace MicroGame::Maze
