#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Maze {

struct FMazeCommands {
  FString Generate;
  FString Create;
  FString IdentityUpdate;
  FString Recall;
  FString Decide;
  FString MemoryStore;
  FString TypedMemoryStore;
};

struct FMazeTokens {
  FString Turn;
  FString Cell;
  FString Query;
  FString Legal;
  FString Visited;
  FString Avoid;
  FString Narrowed;
  FString Thought;
  FString Reason;
  FString Optimal;
  FString Choices;
  FString Direction;
  FString Neighbor;
  FString From;
  FString To;
  FString Note;
  FString Dialogue;
  FString Turns;
  FString DeadEnds;
  FString Edges;
  FString Options;
  FString Moves;
  FString Id;
  FString Line;
  FString Attribute;
  FString Context;
  FString Persona;
  FString Memory;
  FString Type;
  FString Role;
  FString Prefix;
  FString Index;
  FString Depth;
  FString Name;
  FString Seed;
  FString Solved;
  FString Output;
};

struct FMazeNumbers {
  int32 Zero{};
  int32 One{};
  int32 MainPathEndpointCount{};
  uint32 PrngIncrement{};
  int32 PrngFirstShift{};
  int32 PrngSecondShift{};
  uint32 PrngMultiplier{};
  int32 PrngThirdShift{};
  double Uint32Divisor{};
};

struct FMazeConfig {
  FString Group;
  FString Header;
  FString Footer;
  int32 TurnLimit{};
  int32 DeadEndCount{};
  int32 DeadEndDepth{};
  int32 MinRealChoice{};
  int32 MainPathMin{};
  int32 MainPathMax{};
  FString NameAttribute;
  FString RoleAttribute;
  TArray<FString> PersonaAttributes;
  FString SceneAttribute;
  FString SceneMemoryType;
  FString GoalMemoryType;
  TArray<FString> Directions;
  TMap<FString, FString> Opposites;
  FString StartId;
  FString ExitId;
  FString CellPrefix;
  FString DeadEndPrefix;
  FString CellIdFormat;
  FString DeadEndIdFormat;
  FString SceneText;
  FString TurnPrompt;
  FMazeCommands Commands;
  FString ContextQuote;
  FString EmptyContext;
  FString ContextSeparator;
  FString PersonaSeparator;
  FString DisplayNameFormat;
  FString ValueField;
  FString IdPattern;
  FString OptionSeparator;
  FString DisplayOptionSeparator;
  FString DeadEndNote;
  FString QueryFormat;
  FString WardFormat;
  FString ReasonFormat;
  FString ThoughtFormat;
  FString DecisionQualityFormat;
  FString NoRealChoiceNote;
  FString MovedFormat;
  FString BlockedFormat;
  FString UnparsedFormat;
  FString SolvedFormat;
  FString FailedFormat;
  FString CommandFailed;
  FString GeneratedValueMissing;
  FString CreatedIdMissing;
  FString LogWriteFailed;
  FString LayoutHeader;
  FString LayoutEntry;
  FString EdgeFormat;
  FString RunHeader;
  FString ExplorerFormat;
  FString RunSummaryFormat;
  FString BlankLine;
  FString DebugHeader;
  FString DebugFooter;
  FString DialogueFallback;
  FString LogFileEnvKey;
  FString DefaultLogPath;
  FString OutputLineSeparator;
  FString OutputEncoding;
  FMazeTokens Tokens;
  FMazeNumbers Numbers;
};

} // namespace MicroGame::Maze
