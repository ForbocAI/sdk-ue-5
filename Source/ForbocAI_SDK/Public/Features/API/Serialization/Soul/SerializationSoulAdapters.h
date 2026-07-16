#pragma once

#include "Features/API/Serialization/Agent/AgentAdapters.h"
#include "Features/API/Serialization/Memory/SerializationMemoryAdapters.h"
#include "Features/API/Serialization/Soul/Configuration/SerializationSoulConfigurationAdapters.h"

namespace JsonInterop {

/**
 * User Story: As a Soul serializer, I need memory values projected through the
 * canonical memory codec so vector data is never lost during export.
 * @fn inline TArray<TSharedPtr<FJsonValue>> SoulMemoryValues(const TArray<FMemoryItem> &Memories)
 */
inline TArray<TSharedPtr<FJsonValue>>
SoulMemoryValues(const TArray<FMemoryItem> &Memories) {
  return func::map_array<FMemoryItem, TSharedPtr<FJsonValue>>(
      Memories, [](const FMemoryItem &Memory) {
        return MakeShared<FJsonValueObject>(MemoryItemToObject(Memory));
      });
}

/**
 * User Story: As a Soul persistence boundary, I need the complete portable
 * domain value encoded without API-owned fields.
 * @fn inline TSharedRef<FJsonObject> SoulToObject(const FSoul &Soul)
 */
inline TSharedRef<FJsonObject> SoulToObject(const FSoul &Soul) {
  const SoulConfiguration::FSoulSerializationData &Data =
      SoulConfiguration::soulSerializationData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(Data.Soul.Id, Soul.Id),
          Object->SetStringField(Data.Soul.Version, Soul.Version),
          Object->SetStringField(Data.Soul.Name, Soul.Name),
          Object->SetObjectField(
              Data.Soul.StructuredPersona,
              StructuredPersonaToObject(Soul.StructuredPersona)),
          Object->SetArrayField(Data.Soul.Memories,
                                SoulMemoryValues(Soul.Memories)),
          Object->SetObjectField(Data.Soul.State, StateToObject(Soul.State)),
          Object);
}

/**
 * User Story: As a Soul importer, I need required payload fields checked before
 * normalization so malformed permanent bytes fail closed.
 * @fn inline bool HasRequiredSoulFields(const TSharedPtr<FJsonObject> &Object)
 */
inline bool HasRequiredSoulFields(const TSharedPtr<FJsonObject> &Object) {
  const SoulConfiguration::FSoulSerializationData &Data =
      SoulConfiguration::soulSerializationData();
  return Object.IsValid() &&
         Object->HasTypedField<EJson::String>(Data.Soul.Id) &&
         Object->HasTypedField<EJson::String>(Data.Soul.Version) &&
         Object->HasTypedField<EJson::String>(Data.Soul.Name) &&
         Object->HasTypedField<EJson::Object>(Data.Soul.StructuredPersona) &&
         Object->HasTypedField<EJson::Array>(Data.Soul.Memories) &&
         Object->HasTypedField<EJson::Object>(Data.Soul.State);
}

/**
 * User Story: As a Soul importer, I need one pure constructor after every
 * nested payload has passed strict FP traversal.
 * @fn inline FSoul BuildDecodedSoul(const TSharedPtr<FJsonObject> &Object, const FString &StructuredPersona, const TArray<FMemoryItem> &Memories)
 */
inline FSoul BuildDecodedSoul(const TSharedPtr<FJsonObject> &Object,
                              const FString &StructuredPersona,
                              const TArray<FMemoryItem> &Memories) {
  const SoulConfiguration::FSoulSerializationData &Data =
      SoulConfiguration::soulSerializationData();
  FSoul Soul;
  Soul.Id = Object->GetStringField(Data.Soul.Id);
  Soul.Version = Object->GetStringField(Data.Soul.Version);
  Soul.Name = Object->GetStringField(Data.Soul.Name);
  Soul.StructuredPersona = StructuredPersona;
  Soul.Memories = Memories;
  Soul.State = StateFromField(Object, Data.Soul.State);
  return Soul;
}

/**
 * User Story: As a Soul consumer, I need strict nested FP traversal so one
 * malformed persona or memory rejects the complete payload.
 * @fn inline func::Maybe<FSoul> DecodeSoulObject(const TSharedPtr<FJsonObject> &Object)
 */
inline func::Maybe<FSoul>
DecodeSoulObject(const TSharedPtr<FJsonObject> &Object) {
  const SoulConfiguration::FSoulSerializationData &Data =
      SoulConfiguration::soulSerializationData();
  const TArray<TSharedPtr<FJsonValue>> *Memories = nullptr;
  const bool bValid =
      HasRequiredSoulFields(Object) &&
      Object->GetStringField(Data.Soul.Version) == Data.Version &&
      Object->TryGetArrayField(Data.Soul.Memories, Memories) && Memories;
  return !bValid
             ? func::nothing<FSoul>()
             : [&]() {
  const func::Maybe<FString> Persona = DecodeStructuredPersonaObject(
      Object->GetObjectField(Data.Soul.StructuredPersona));
  const func::Maybe<TArray<FMemoryItem>> DecodedMemories =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FMemoryItem>(
          *Memories, DecodeMemoryItemValue);
  return func::is_nothing(Persona) || func::is_nothing(DecodedMemories)
             ? func::nothing<FSoul>()
             : func::just(BuildDecodedSoul(Object, Persona.value,
                                           DecodedMemories.value));
               }();
}

} // namespace JsonInterop
