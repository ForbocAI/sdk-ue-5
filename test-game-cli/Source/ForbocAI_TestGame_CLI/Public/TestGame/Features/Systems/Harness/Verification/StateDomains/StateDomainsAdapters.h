#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/Parsing/VerificationParsingAdapters.h"

namespace TestGame::VerificationAdapters {

/** User Story: As a normalized-state verifier, I need NPC, memory, inventory, and Soul cases decoded atomically so tests only exercise store transitions. @fn inline Verification::FStateDomainsVerificationData ReadStateDomains(const TSharedRef<FJsonObject> &Root) */
inline Verification::FStateDomainsVerificationData
ReadStateDomains(const TSharedRef<FJsonObject> &Root) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Root, TEXT("stateDomains"));
  const TSharedRef<FJsonObject> Automation =
      DataAdapters::ReadObjectField(Object, TEXT("automationNames"));
  const TSharedRef<FJsonObject> Stories =
      DataAdapters::ReadObjectField(Object, TEXT("stories"));
  const TSharedRef<FJsonObject> Npc =
      DataAdapters::ReadObjectField(Object, TEXT("npc"));
  const TSharedRef<FJsonObject> Patch =
      DataAdapters::ReadObjectField(Npc, TEXT("patch"));
  const TSharedRef<FJsonObject> Verdict =
      DataAdapters::ReadObjectField(Npc, TEXT("verdict"));
  const TSharedRef<FJsonObject> Store =
      DataAdapters::ReadObjectField(Object, TEXT("store"));
  const TSharedRef<FJsonObject> Memory =
      DataAdapters::ReadObjectField(Store, TEXT("memory"));
  const TSharedRef<FJsonObject> Inventory =
      DataAdapters::ReadObjectField(Store, TEXT("inventory"));
  const TSharedRef<FJsonObject> Soul =
      DataAdapters::ReadObjectField(Store, TEXT("soul"));
  Verification::FStateDomainsVerificationData Data;
  Data.suite = DataAdapters::ReadStringField(Object, TEXT("suite"));
  Data.automationNames.npc =
      DataAdapters::ReadStringField(Automation, TEXT("npc"));
  Data.automationNames.store =
      DataAdapters::ReadStringField(Automation, TEXT("store"));
  Data.stories.npc = DataAdapters::ReadStringField(Stories, TEXT("npc"));
  Data.stories.store = DataAdapters::ReadStringField(Stories, TEXT("store"));
  Data.npc.initial =
      ReadNpc(DataAdapters::ReadObjectField(Npc, TEXT("initial")));
  Data.npc.patch.name = DataAdapters::ReadStringField(Patch, TEXT("name"));
  Data.npc.patch.hp = DataAdapters::ReadNumberField(Patch, TEXT("hp"));
  Data.npc.patch.suspicion =
      DataAdapters::ReadNumberField(Patch, TEXT("suspicion"));
  Data.npc.patch.inventory =
      DataAdapters::ReadStringArrayField(Patch, TEXT("inventory"));
  Data.npc.patch.position =
      ReadPosition(DataAdapters::ReadObjectField(Patch, TEXT("position")));
  Data.npc.verdict.actionType =
      DataAdapters::ReadStringField(Verdict, TEXT("actionType"));
  Data.npc.verdict.targetPosition = ReadPosition(
      DataAdapters::ReadObjectField(Verdict, TEXT("targetPosition")));
  Data.npc.verdict.suspicion =
      DataAdapters::ReadNumberField(Verdict, TEXT("suspicion"));
  Data.npc.expectedEntityCount =
      DataAdapters::ReadNumberField(Npc, TEXT("expectedEntityCount"));
  Data.store.memory.records =
      func::map_array<TSharedPtr<FJsonValue>, FMemoryRecord>(
          DataAdapters::ReadObjectArrayField(Memory, TEXT("records")),
          [](const TSharedPtr<FJsonValue> &Value) {
            return ReadMemory(Value->AsObject().ToSharedRef());
          });
  Data.store.memory.lookupId =
      DataAdapters::ReadStringField(Memory, TEXT("lookupId"));
  Data.store.memory.npcId =
      DataAdapters::ReadStringField(Memory, TEXT("npcId"));
  Data.store.memory.expectedTotal =
      DataAdapters::ReadNumberField(Memory, TEXT("expectedTotal"));
  Data.store.memory.expectedNpcCount =
      DataAdapters::ReadNumberField(Memory, TEXT("expectedNpcCount"));
  Data.store.inventory.ownerId =
      DataAdapters::ReadStringField(Inventory, TEXT("ownerId"));
  Data.store.inventory.items =
      DataAdapters::ReadStringArrayField(Inventory, TEXT("items"));
  Data.store.soul.npcId =
      DataAdapters::ReadStringField(Soul, TEXT("npcId"));
  Data.store.soul.transactionId =
      DataAdapters::ReadStringField(Soul, TEXT("transactionId"));
  Data.store.soul.expectedImportedCount =
      DataAdapters::ReadNumberField(Soul, TEXT("expectedImportedCount"));
  return Data;
}

} // namespace TestGame::VerificationAdapters
