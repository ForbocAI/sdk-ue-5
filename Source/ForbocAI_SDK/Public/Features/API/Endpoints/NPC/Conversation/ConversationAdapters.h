#pragma once

#include "Core/JsonInterop.h"
#include "Features/API/Endpoints/NPC/Conversation/Configuration/ConversationConfigurationAdapters.h"
#include "Features/API/Serialization/Agent/AgentAdapters.h"
#include "Features/NPC/Conversation/ConversationTypes.h"

namespace APISlice::Detail {

namespace NPCConversationCodec {

/** User Story: As an NPC conversation decoder, I need required text rejected when absent or below the authored minimum length. @fn inline bool HasRequiredString(const TSharedPtr<FJsonObject> &Object, const FString &Field, int32 MinimumTextLength) */
inline bool HasRequiredString(const TSharedPtr<FJsonObject> &Object,
                              const FString &Field,
                              int32 MinimumTextLength) {
  return Object.IsValid() && Object->HasTypedField<EJson::String>(Field) &&
         Object->GetStringField(Field).TrimStartAndEnd().Len() >=
             MinimumTextLength;
}

/**
 * User Story: As an NPC conversation decoder, I need string arrays traversed atomically.
 * @fn inline func::Maybe<TArray<FString>> DecodeStringArray( const TSharedPtr<FJsonObject> &Object, const FString &Field, int32 MinimumCount, int32 MinimumTextLength)
 */
inline func::Maybe<TArray<FString>> DecodeStringArray(
    const TSharedPtr<FJsonObject> &Object, const FString &Field,
    int32 MinimumCount, int32 MinimumTextLength) {
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  return !Object.IsValid() || !Object->TryGetArrayField(Field, Values) ||
                 !Values || Values->Num() < MinimumCount
             ? func::nothing<TArray<FString>>()
             : func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
                   *Values, [MinimumTextLength](
                                const TSharedPtr<FJsonValue> &Value) {
                     return Value.IsValid() &&
                                    Value->Type == EJson::String &&
                                    Value->AsString()
                                            .TrimStartAndEnd()
                                            .Len() >= MinimumTextLength
                                ? func::just(Value->AsString())
                                : func::nothing<FString>();
                   });
}

/**
 * User Story: As an NPC conversation decoder, I need generated participant fields validated before entering SDK state.
 * @fn inline func::Maybe<FNPCConversationParticipant> DecodeParticipant( const TSharedPtr<FJsonValue> &Value, const APISlice::Endpoints::NPCConversationConfiguration:: FConversationLimits &Limits)
 */
inline func::Maybe<FNPCConversationParticipant> DecodeParticipant(
    const TSharedPtr<FJsonValue> &Value,
    const APISlice::Endpoints::NPCConversationConfiguration::
        FConversationLimits &Limits) {
  using namespace APISlice::Endpoints::NPCConversationConfiguration;
  const FConversationFields &Fields = conversationConfigurationData().Fields;
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object
          ? Value->AsObject()
          : TSharedPtr<FJsonObject>();
  const func::Maybe<TArray<FString>> Traits =
      DecodeStringArray(Object, Fields.Traits,
                        Limits.MinimumStringArrayCount,
                        Limits.MinimumTextLength);
  const func::Maybe<TArray<FString>> Goals =
      DecodeStringArray(Object, Fields.Goals,
                        Limits.MinimumStringArrayCount,
                        Limits.MinimumTextLength);
  const func::Maybe<TArray<FString>> Relationships =
      DecodeStringArray(Object, Fields.Relationships,
                        Limits.MinimumStringArrayCount,
                        Limits.MinimumTextLength);
  const func::Maybe<TArray<FString>> SpeakingStyle =
      DecodeStringArray(Object, Fields.SpeakingStyle,
                        Limits.MinimumStringArrayCount,
                        Limits.MinimumTextLength);
  const func::Maybe<TArray<FString>> Constraints =
      DecodeStringArray(Object, Fields.Constraints,
                        Limits.MinimumStringArrayCount,
                        Limits.MinimumTextLength);
  const bool bValid =
      HasRequiredString(Object, Fields.Slot, Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.NpcId, Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.Name, Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.Role, Limits.MinimumTextLength) &&
      Traits.hasValue &&
      Goals.hasValue && Relationships.hasValue &&
      HasRequiredString(Object, Fields.Location, Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.Mood, Limits.MinimumTextLength) &&
      SpeakingStyle.hasValue &&
      Constraints.hasValue;
  return !bValid
             ? func::nothing<FNPCConversationParticipant>()
             : [&]() {
                 FNPCConversationParticipant Participant;
                 Participant.Slot = Object->GetStringField(Fields.Slot);
                 Participant.NpcId = Object->GetStringField(Fields.NpcId);
                 Participant.Name = Object->GetStringField(Fields.Name);
                 Participant.Role = Object->GetStringField(Fields.Role);
                 Participant.Traits = Traits.value;
                 Participant.Goals = Goals.value;
                 Participant.Relationships = Relationships.value;
                 Participant.Location =
                     Object->GetStringField(Fields.Location);
                 Participant.Mood = Object->GetStringField(Fields.Mood);
                 Participant.SpeakingStyle = SpeakingStyle.value;
                 Participant.Constraints = Constraints.value;
                 return func::just(Participant);
               }();
}

/** User Story: As an NPC conversation decoder, I need each API-authored turn validated as one value. @fn inline func::Maybe<FNPCConversationTurn> DecodeTurn(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FNPCConversationTurn>
DecodeTurn(const TSharedPtr<FJsonValue> &Value) {
  using namespace APISlice::Endpoints::NPCConversationConfiguration;
  const FConversationConfigurationData &Data =
      conversationConfigurationData();
  const FConversationFields &Fields = Data.Fields;
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object
          ? Value->AsObject()
          : TSharedPtr<FJsonObject>();
  double RawIndex{};
  const TSharedPtr<FJsonValue> ActionValue =
      Object.IsValid() ? Object->TryGetField(Fields.Action) : nullptr;
  const bool bHasAction = ActionValue.IsValid() &&
                          ActionValue->Type != EJson::Null;
  const func::Maybe<FAgentAction> Action =
      !bHasAction || ActionValue->Type != EJson::Object
          ? func::nothing<FAgentAction>()
          : JsonInterop::DecodeActionObject(ActionValue->AsObject());
  const bool bValid =
      Object.IsValid() && Object->TryGetNumberField(Fields.Index, RawIndex) &&
      FMath::IsFinite(RawIndex) &&
      FMath::IsNearlyEqual(RawIndex, FMath::RoundToDouble(RawIndex)) &&
      RawIndex >= Data.Limits.MinimumTurnIndex &&
      HasRequiredString(Object, Fields.SpeakerSlot,
                        Data.Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.SpeakerName,
                        Data.Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.ListenerSlot,
                        Data.Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.ListenerName,
                        Data.Limits.MinimumTextLength) &&
      HasRequiredString(Object, Fields.Dialogue,
                        Data.Limits.MinimumTextLength) &&
      Object->HasTypedField<EJson::Boolean>(Fields.Valid) &&
      (!bHasAction || Action.hasValue);
  return !bValid
             ? func::nothing<FNPCConversationTurn>()
             : [&]() {
                 FNPCConversationTurn Turn;
                 Turn.Index = static_cast<int32>(RawIndex);
                 Turn.SpeakerSlot =
                     Object->GetStringField(Fields.SpeakerSlot);
                 Turn.SpeakerName =
                     Object->GetStringField(Fields.SpeakerName);
                 Turn.ListenerSlot =
                     Object->GetStringField(Fields.ListenerSlot);
                 Turn.ListenerName =
                     Object->GetStringField(Fields.ListenerName);
                 Turn.Dialogue = Object->GetStringField(Fields.Dialogue);
                 Turn.bHasAction = bHasAction;
                 bHasAction ? (Turn.Action = Action.value, void()) : void();
                 Turn.bValid = Object->GetBoolField(Fields.Valid);
                 return func::just(Turn);
               }();
}

} // namespace NPCConversationCodec

/**
 * User Story: As a thin UE SDK consumer, I need the API conversation response decoded without recreating its logic.
 * @fn inline bool DecodeNpcConversationResponse( const FString &Json, FNPCConversationResponse &Response)
 */
inline bool DecodeNpcConversationResponse(
    const FString &Json, FNPCConversationResponse &Response) {
  using namespace APISlice::Endpoints::NPCConversationConfiguration;
  const FConversationConfigurationData &Data =
      conversationConfigurationData();
  const FConversationFields &Fields = Data.Fields;
  TSharedPtr<FJsonObject> Root;
  const TArray<TSharedPtr<FJsonValue>> *RawParticipants = nullptr;
  const TArray<TSharedPtr<FJsonValue>> *RawTurns = nullptr;
  const bool bShape = JsonInterop::ParseJsonObject(Json, Root) &&
                      Root.IsValid() &&
                      Root->TryGetArrayField(Fields.Participants,
                                             RawParticipants) &&
                      RawParticipants &&
                      Root->TryGetArrayField(Fields.Turns, RawTurns) &&
                      RawTurns;
  const func::Maybe<FNPCConversationResponse> Decoded =
      !bShape
          ? func::nothing<FNPCConversationResponse>()
          : [&]() {
              const func::Maybe<TArray<FNPCConversationParticipant>>
                  Participants = func::traverse_maybe_array<
                      TSharedPtr<FJsonValue>, FNPCConversationParticipant>(
                      *RawParticipants,
                      [&Data](const TSharedPtr<FJsonValue> &Value) {
                        return NPCConversationCodec::DecodeParticipant(
                            Value, Data.Limits);
                      });
              const func::Maybe<TArray<FNPCConversationTurn>> Turns =
                  func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                             FNPCConversationTurn>(
                      *RawTurns, NPCConversationCodec::DecodeTurn);
              const func::Maybe<TArray<FString>> TranscriptLines =
                  NPCConversationCodec::DecodeStringArray(
                      Root, Fields.TranscriptLines,
                      Data.Limits.MinimumTranscriptLineCount,
                      Data.Limits.MinimumTextLength);
              const bool bValid =
                  NPCConversationCodec::HasRequiredString(
                      Root, Fields.SchemaVersion,
                      Data.Limits.MinimumTextLength) &&
                  NPCConversationCodec::HasRequiredString(
                      Root, Fields.ConversationId,
                      Data.Limits.MinimumTextLength) &&
                  Root->HasTypedField<EJson::Boolean>(Fields.Valid) &&
                  Participants.hasValue &&
                  Participants.value.Num() == Data.Limits.ParticipantCount &&
                  NPCConversationCodec::HasRequiredString(
                      Root, Fields.Topic, Data.Limits.MinimumTextLength) &&
                  NPCConversationCodec::HasRequiredString(
                      Root, Fields.Opening, Data.Limits.MinimumTextLength) &&
                  Turns.hasValue &&
                  Turns.value.Num() >= Data.Limits.MinimumTurnCount &&
                  TranscriptLines.hasValue;
              return !bValid
                         ? func::nothing<FNPCConversationResponse>()
                         : [&]() {
                             FNPCConversationResponse Value;
                             Value.SchemaVersion = Root->GetStringField(
                                 Fields.SchemaVersion);
                             Value.ConversationId = Root->GetStringField(
                                 Fields.ConversationId);
                             Value.bValid = Root->GetBoolField(Fields.Valid);
                             Value.Participants = Participants.value;
                             Value.Topic = Root->GetStringField(Fields.Topic);
                             Value.Opening =
                                 Root->GetStringField(Fields.Opening);
                             Value.Turns = Turns.value;
                             Value.TranscriptLines = TranscriptLines.value;
                             Value.RawJson = JsonInterop::StringifyObject(Root);
                             return func::just(MoveTemp(Value));
                           }();
            }();
  return func::match(
      Decoded,
      [&Response](const FNPCConversationResponse &Value) {
        Response = Value;
        return true;
      },
      []() { return false; });
}

} // namespace APISlice::Detail
