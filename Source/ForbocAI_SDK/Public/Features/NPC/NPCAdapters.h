#pragma once

#include "Algo/Transform.h"
#include "Core/rtk.hpp"
#include "Dom/JsonObject.h"
#include "Features/NPC/NPCTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace NPCAdapters {

inline FString npcIdSelector(const FNPCInternalState &Npc) { return Npc.Id; }

inline const rtk::EntityAdapter<FNPCInternalState> &npcAdapter() {
  static const rtk::EntityAdapter<FNPCInternalState> Adapter =
      rtk::createEntityAdapter<FNPCInternalState>(&npcIdSelector);
  return Adapter;
}

inline FAgentState updateNPCStateLocally(const FAgentState &Current,
                                         const FAgentState &Delta) {
  return (Delta.JsonData.IsEmpty() || Delta.JsonData == TEXT("{}"))
             ? Current
         : (Current.JsonData.IsEmpty() || Current.JsonData == TEXT("{}"))
             ? Delta
             : [&]() -> FAgentState {
    TSharedPtr<FJsonObject> CurrentJson = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> DeltaJson = MakeShared<FJsonObject>();
    return (!FJsonSerializer::Deserialize(
                TJsonReaderFactory<>::Create(Current.JsonData), CurrentJson) ||
            !CurrentJson.IsValid())
               ? Delta
           : (!FJsonSerializer::Deserialize(
                  TJsonReaderFactory<>::Create(Delta.JsonData), DeltaJson) ||
              !DeltaJson.IsValid())
               ? Delta
               : [&]() -> FAgentState {
      TArray<FString> Keys;
      Keys.Reserve(DeltaJson->Values.Num());
      Algo::Transform(DeltaJson->Values, Keys, [](const auto &Field) {
        return FString(Field.Key.Len(), *Field.Key);
      });
      struct MergeFields {
        static void apply(const TSharedPtr<FJsonObject> &Source,
                          const TSharedPtr<FJsonObject> &Destination,
                          const TArray<FString> &FieldNames, int32 Index) {
          Index >= FieldNames.Num()
              ? void()
              : (Destination->SetField(
                     FieldNames[Index],
                     Source->TryGetField(FieldNames[Index])),
                 apply(Source, Destination, FieldNames, Index + 1), void());
        }
      };
      MergeFields::apply(DeltaJson, CurrentJson, Keys, 0);
      FString MergedJson;
      FJsonSerializer::Serialize(CurrentJson.ToSharedRef(),
                                 TJsonWriterFactory<>::Create(&MergedJson));
      return TypeFactory::AgentState(MergedJson);
    }();
  }();
}

inline FNPCStateLogEntry makeStateLogEntry(const FAgentState &Delta,
                                           const FAgentState &State) {
  FNPCStateLogEntry Entry;
  Entry.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
  Entry.Delta = Delta;
  Entry.State = State;
  return Entry;
}

} // namespace NPCAdapters
