#pragma once

#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "MicroGame/Features/Components/Harness/Maze/Run/MazeRunTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze evidence persistence, I need an environment override or authored project-relative path selected at the file effect boundary. @fn inline FString SelectMazeLogPath(const FMazeLogTarget &Target) */
inline FString SelectMazeLogPath(const FMazeLogTarget &Target) {
  const FString Override =
      FPlatformMisc::GetEnvironmentVariable(*Target.EnvironmentKey);
  return Override.IsEmpty()
             ? FPaths::ConvertRelativePathToFull(
                   FPaths::Combine(FPaths::ProjectDir(), Target.DefaultPath))
             : FPaths::ConvertRelativePathToFull(Override);
}

/** User Story: As durable Maze diagnostics, I need the complete transcript written with authored line separation and UTF-8 encoding. @fn inline bool WriteMazeTranscript(const FMazeLogTarget &Target, const TArray<FString> &Lines) */
inline bool WriteMazeTranscript(const FMazeLogTarget &Target,
                                const TArray<FString> &Lines) {
  const FString Path = SelectMazeLogPath(Target);
  FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(
      *FPaths::GetPath(Path));
  return FFileHelper::SaveStringToFile(
      FString::Join(Lines, *MazeConfig().OutputLineSeparator) +
          MazeConfig().OutputLineSeparator,
      *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

/** User Story: As an exclusive Maze CLI mode, I need transcript lines emitted without adding presentation policy to the runner. @fn inline void LogMazeTranscript(const TArray<FString> &Lines) */
inline void LogMazeTranscript(const TArray<FString> &Lines) {
  func::for_each_array<FString>(Lines, [](const FString &Line) {
    GLog != nullptr ? GLog->Log(*Line) : void();
  });
}

} // namespace MicroGame::Maze
