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

  FCommandSpec() : Group(ECommandGroup::Unknown) {}
};

} // namespace TestGame
