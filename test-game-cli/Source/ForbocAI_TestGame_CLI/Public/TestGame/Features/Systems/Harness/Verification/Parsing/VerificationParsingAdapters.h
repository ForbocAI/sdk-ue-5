#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationTypes.h"

namespace TestGame::VerificationAdapters {

/** User Story: As a verification-data consumer, I need positions decoded through one typed boundary so every test domain uses the same coordinates. @fn inline FPosition ReadPosition(const TSharedRef<FJsonObject> &Object) */
inline FPosition ReadPosition(const TSharedRef<FJsonObject> &Object) {
  return {DataAdapters::ReadNumberField(Object, TEXT("x")),
          DataAdapters::ReadNumberField(Object, TEXT("y"))};
}

/** User Story: As a verification-data consumer, I need NPC state decoded through one typed boundary so reducer tests do not author entities in C++. @fn inline FGameNPC ReadNpc(const TSharedRef<FJsonObject> &Object) */
inline FGameNPC ReadNpc(const TSharedRef<FJsonObject> &Object) {
  const TSharedRef<FJsonObject> Position =
      DataAdapters::ReadObjectField(Object, TEXT("position"));
  FGameNPC Npc;
  Npc.Id = DataAdapters::ReadStringField(Object, TEXT("id"));
  Npc.Name = DataAdapters::ReadStringField(Object, TEXT("name"));
  Npc.Faction = DataAdapters::ReadStringField(Object, TEXT("faction"));
  Npc.Hp = DataAdapters::ReadNumberField(Object, TEXT("hp"));
  Npc.Suspicion = DataAdapters::ReadNumberField(Object, TEXT("suspicion"));
  Npc.Inventory =
      DataAdapters::ReadStringArrayField(Object, TEXT("inventory"));
  Npc.KnownSecrets =
      DataAdapters::ReadStringArrayField(Object, TEXT("knownSecrets"));
  Npc.Position = ReadPosition(Position);
  return Npc;
}

/** User Story: As a verification-data consumer, I need memory records decoded through one typed boundary so entity-adapter tests operate on authored records. @fn inline FMemoryRecord ReadMemory(const TSharedRef<FJsonObject> &Object) */
inline FMemoryRecord ReadMemory(const TSharedRef<FJsonObject> &Object) {
  FMemoryRecord Memory;
  Memory.Id = DataAdapters::ReadStringField(Object, TEXT("id"));
  Memory.NpcId = DataAdapters::ReadStringField(Object, TEXT("npcId"));
  Memory.Text = DataAdapters::ReadStringField(Object, TEXT("text"));
  Memory.Importance =
      DataAdapters::ReadFloatField(Object, TEXT("importance"));
  return Memory;
}

} // namespace TestGame::VerificationAdapters
