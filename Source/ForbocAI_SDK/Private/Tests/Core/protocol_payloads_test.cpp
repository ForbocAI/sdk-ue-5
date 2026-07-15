/**
 * Protocol payloads isolation tests — tests local serialization codecs
 * Payloads are exercised through their concrete serialization contracts.
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/Testing/API/Codec/APICodecAdapters.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Protocol/Requests/RequestsTypes.h"
#include "Features/API/APIAdapters.h"

using namespace rtk;
using namespace Testing::API::Codec;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeIdentifyActorPayloadTest,
    "ForbocAI.Core.Protocol.SerializeIdentifyActorPayload",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSerializeIdentifyActorPayloadTest::RunTest(const FString &Parameters) {
  const FIdentifyActorPayloadFixture &Fixture =
      CodecFixtures().IdentifyActorPayload;
  FNPCActorInfo Actor;
  Actor.NpcId = Fixture.NpcId;
  Actor.Persona = Fixture.Persona;
  Actor.Data.JsonData = Fixture.DataJson;
  
  FString Json = rtk::detail::SerializeIdentifyActorResult(Actor);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> ActorObject =
      DataAdapters::ReadObjectField(RootObject, TEXT("actor"));
  const TSharedRef<FJsonObject> DataObject =
      DataAdapters::ReadObjectField(ActorObject, TEXT("data"));

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject, TEXT("type")),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.NpcId,
            DataAdapters::ReadStringField(ActorObject, TEXT("npcId")),
            Fixture.NpcId);
  TestEqual(Fixture.Labels.Persona,
            DataAdapters::ReadStringField(ActorObject, TEXT("persona")),
            Fixture.Persona);
  TestEqual(Fixture.Labels.Health,
            DataAdapters::ReadNumberField(DataObject, TEXT("health")),
            Fixture.ExpectedHealth);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeDecisionPayloadTest,
    "ForbocAI.Core.Protocol.SerializeDecisionPayload",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSerializeDecisionPayloadTest::RunTest(const FString &Parameters) {
  const FDecisionPayloadFixture &Fixture = CodecFixtures().DecisionPayload;
  FString Json = rtk::detail::SerializeDecisionResult(
      Fixture.Goal, Fixture.ActionType, Fixture.Target);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Intent =
      DataAdapters::ReadObjectField(RootObject, TEXT("decisionIntent"));

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject, TEXT("type")),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.Goal,
            DataAdapters::ReadStringField(Intent, TEXT("goal")),
            Fixture.Goal);
  TestEqual(Fixture.Labels.ActionType,
            DataAdapters::ReadStringField(Intent, TEXT("actionType")),
            Fixture.ActionType);
  TestEqual(Fixture.Labels.Target,
            DataAdapters::ReadStringField(Intent, TEXT("target")),
            Fixture.Target);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeReasoningPayloadTest,
    "ForbocAI.Core.Protocol.SerializeReasoningPayload",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSerializeReasoningPayloadTest::RunTest(const FString &Parameters) {
  const FReasoningPayloadFixture &Fixture = CodecFixtures().ReasoningPayload;
  FString Json = rtk::detail::SerializeReasoningResult(
      Fixture.ReasoningText, Fixture.ResponseText);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Reasoning =
      DataAdapters::ReadObjectField(RootObject, TEXT("reasoningOutput"));

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject, TEXT("type")),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.ReasoningText,
            DataAdapters::ReadStringField(Reasoning, TEXT("reasoningText")),
            Fixture.ReasoningText);
  TestEqual(Fixture.Labels.ResponseText,
            DataAdapters::ReadStringField(Reasoning, TEXT("responseText")),
            Fixture.ResponseText);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEncodeProcessTapePayloadTest,
    "ForbocAI.Core.Protocol.EncodeProcessTapePayload",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FEncodeProcessTapePayloadTest::RunTest(const FString &Parameters) {
  const FProcessTapePayloadFixture &Fixture =
      CodecFixtures().ProcessTapePayload;
  FNPCProcessTape Tape;
  Tape.Observation = Fixture.Observation;
  Tape.ContextJson = Fixture.ContextJson;
  Tape.NpcState.JsonData = Fixture.NpcStateJson;
  Tape.Persona = Fixture.Persona;
  
  TSharedRef<FJsonObject> Obj = APISlice::Detail::EncodeProcessTapeObject(Tape);
  FString Json = APISlice::Detail::ToJsonString(Obj);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Context =
      DataAdapters::ReadObjectField(RootObject, TEXT("context"));
  const TSharedRef<FJsonObject> StructuredPersona =
      DataAdapters::ReadObjectField(RootObject, TEXT("structuredPersona"));
  const TArray<FString> Traits = DataAdapters::ReadStringArrayField(
      StructuredPersona, TEXT("traits"));

  TestEqual(Fixture.Labels.Observation,
            DataAdapters::ReadStringField(RootObject, TEXT("observation")),
            Fixture.Observation);
  TestTrue(Fixture.Labels.Persona, Traits == Fixture.ExpectedTraits);
  TestEqual(Fixture.Labels.ContextTime,
            DataAdapters::ReadStringField(Context, TEXT("time")),
            Fixture.ExpectedContextTime);
  
  return true;
}
