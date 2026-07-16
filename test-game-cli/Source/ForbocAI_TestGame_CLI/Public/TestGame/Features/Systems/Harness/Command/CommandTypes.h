#pragma once

#include "CoreMinimal.h"

namespace TestGame {

enum class ECommandGroup : uint8 {
  Unknown,
  Status,
  NpcLifecycle,
  NpcProcessChat,
  listMemory,
  recallMemory,
  storeMemory,
  clearMemory,
  MemoryExport,
  getBridgeRules,
  BridgeValidate,
  loadBridgePreset,
  SoulExport,
  SoulImport,
  SoulList,
  SoulChat,
  GhostLifecycle
};

enum class EOutputAssertionKind : uint8 { Unknown, IncludesAlias };

struct FOutputAssertion {
  EOutputAssertionKind Kind;
  FString Alias;

  /** User Story: As a systems harness command consumer, I need output assertions initialized to an invalid kind so malformed API contracts cannot pass silently. @fn FOutputAssertion() */
  FOutputAssertion() : Kind(EOutputAssertionKind::Unknown) {}
};

/** User Story: As a systems harness command consumer, I need to invoke required groups through a stable signature so the systems harness command workflow remains explicit and composable. @fn inline const TArray<ECommandGroup> &RequiredGroups() */
inline const TArray<ECommandGroup> &RequiredGroups() {
  static const TArray<ECommandGroup> Groups = {
      ECommandGroup::Status,         ECommandGroup::NpcLifecycle,
      ECommandGroup::NpcProcessChat, ECommandGroup::listMemory,
      ECommandGroup::recallMemory,   ECommandGroup::storeMemory,
      ECommandGroup::clearMemory,    ECommandGroup::MemoryExport,
      ECommandGroup::getBridgeRules, ECommandGroup::BridgeValidate,
      ECommandGroup::loadBridgePreset, ECommandGroup::SoulExport,
      ECommandGroup::SoulImport,     ECommandGroup::SoulList,
      ECommandGroup::SoulChat,       ECommandGroup::GhostLifecycle};
  return Groups;
}

struct FCommandSpec {
  ECommandGroup Group;
  FString Command;
  TArray<FString> ExpectedRoutes;
  TArray<FOutputAssertion> OutputAssertions;

  /** User Story: As a systems harness command consumer, I need to invoke fcommand spec through a stable signature so the systems harness command workflow remains explicit and composable. @fn FCommandSpec() */
  FCommandSpec() : Group(ECommandGroup::Unknown) {}
};

} // namespace TestGame
