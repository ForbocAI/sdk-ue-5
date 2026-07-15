#pragma once

#include "Core/fp.hpp"
#include "Features/NPC/NPCTypes.h"

namespace Testing::NPC {

enum class ENPCTestActionKind : uint8 {
  InfoReceived,
  ActiveChanged,
  StateReplaced,
  StateUpdated,
  HistoryAppended,
  HistoryReceived,
  Blocked,
  BlockCleared,
  Removed,
  Inspect,
  Count
};

struct FNPCTestAction {
  ENPCTestActionKind Kind;
  func::Maybe<FString> Id;
  func::Maybe<FString> Persona;
  func::Maybe<FString> StateJson;
  func::Maybe<FString> DeltaJson;
  func::Maybe<FString> Role;
  func::Maybe<FString> Content;
  func::Maybe<FString> Reason;
  TArray<FNPCHistoryEntry> History;
};

struct FNPCTestExpected {
  func::Maybe<FString> ActiveId;
  func::Maybe<int32> NpcCount;
  func::Maybe<bool> HasNpc;
  func::Maybe<FString> SelectedPersona;
  func::Maybe<FString> SelectedStateJson;
  func::Maybe<int32> HistoryCount;
  func::Maybe<bool> Blocked;
  func::Maybe<FString> BlockReason;
  func::Maybe<int32> StateLogCount;
  func::Maybe<bool> ActiveExists;
  func::Maybe<int32> IdCount;
  func::Maybe<int32> EntityCount;
};

struct FNPCTestStep {
  FNPCTestAction Action;
  FNPCTestExpected Expected;
};

struct FNPCTestScenario {
  FString Name;
  TArray<FNPCTestStep> Steps;
};

struct FNPCTestLabels {
  FString Suite;
  FString CaseName;
  FString RequiredField;
  FString ScenarioPresent;
  FString ActiveId;
  FString NpcCount;
  FString HasNpc;
  FString SelectedPersona;
  FString SelectedStateJson;
  FString HistoryCount;
  FString Blocked;
  FString BlockReason;
  FString StateLogCount;
  FString ActiveExists;
  FString IdCount;
  FString EntityCount;
};

struct FNPCTestFixtures {
  FNPCTestLabels Labels;
  TArray<FNPCTestScenario> Scenarios;
};

} // namespace Testing::NPC
