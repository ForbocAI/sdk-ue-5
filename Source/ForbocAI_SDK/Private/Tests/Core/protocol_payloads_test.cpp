/**
 * Protocol payloads isolation tests — tests local serialization codecs
 * Payloads are exercised through their concrete serialization contracts.
 */

#include "CoreMinimal.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Misc/AutomationTest.h"
#include "Systems/API/Serialization/APISerializationAdapters.h"
#include "Systems/Testing/API/Codec/APICodecAdapters.h"
#include "Systems/Protocol/ProtocolThunks.h"
#include "Components/Protocol/Requests/RequestsTypes.h"
#include "Systems/API/APIAdapters.h"

using namespace rtk;
using namespace Testing::API::Codec;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeIdentifyActorPayloadTest,
    FORBOCAI_SDK_AUTHORED_STRINGVB67052AFCE5C,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FSerializeIdentifyActorPayloadTest::RunTest(const FString &Parameters)
 */
bool FSerializeIdentifyActorPayloadTest::RunTest(const FString &Parameters) {
  const FIdentifyActorPayloadFixture &Fixture =
      CodecFixtures().IdentifyActorPayload;
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  const auto &AgentData =
      JsonInterop::AgentConfiguration::agentContractData();
  FNPCActorInfo Actor;
  Actor.NpcId = Fixture.NpcId;
  Actor.Persona = Fixture.Persona;
  Actor.bHasStructuredPersona = true;
  Actor.Data.JsonData = Fixture.DataJson;
  
  FString Json = rtk::detail::SerializeIdentifyActorResult(Actor);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> ActorObject =
      DataAdapters::ReadObjectField(RootObject, ProcessData.Tape.Actor);
  const TSharedRef<FJsonObject> DataObject =
      DataAdapters::ReadObjectField(ActorObject, ProcessData.Actor.Data);
  const TSharedRef<FJsonObject> PersonaObject = DataAdapters::ReadObjectField(
      ActorObject, ProcessData.Actor.StructuredPersona);
  const TArray<FString> Traits = DataAdapters::ReadStringArrayField(
      PersonaObject, AgentData.Persona.Traits);

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject,
                                          ProcessData.Instruction.Type),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.NpcId,
            DataAdapters::ReadStringField(ActorObject, ProcessData.Actor.Id),
            Fixture.NpcId);
  TestTrue(Fixture.Labels.Persona,
           Traits == TArray<FString>{Fixture.Persona});
  TestEqual(Fixture.Labels.Data, DataAdapters::SerializeObject(DataObject),
            Fixture.DataJson);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeDecisionPayloadTest,
    FORBOCAI_SDK_AUTHORED_STRINGV10387C1784E8,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FSerializeDecisionPayloadTest::RunTest(const FString &Parameters)
 */
bool FSerializeDecisionPayloadTest::RunTest(const FString &Parameters) {
  const FDecisionPayloadFixture &Fixture = CodecFixtures().DecisionPayload;
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  FString Json = rtk::detail::SerializeDecisionResult(
      Fixture.Goal, Fixture.ActionType, Fixture.Target);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Intent =
      DataAdapters::ReadObjectField(RootObject,
                                    ProcessData.Tape.DecisionIntent);

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject,
                                          ProcessData.Instruction.Type),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.Goal,
            DataAdapters::ReadStringField(Intent,
                                          ProcessData.DecisionIntent.Goal),
            Fixture.Goal);
  TestEqual(Fixture.Labels.ActionType,
            DataAdapters::ReadStringField(
                Intent, ProcessData.DecisionIntent.ActionType),
            Fixture.ActionType);
  TestEqual(Fixture.Labels.Target,
            DataAdapters::ReadStringField(Intent,
                                          ProcessData.DecisionIntent.Target),
            Fixture.Target);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeReasoningPayloadTest,
    FORBOCAI_SDK_AUTHORED_STRINGV646CDFE05999,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FSerializeReasoningPayloadTest::RunTest(const FString &Parameters)
 */
bool FSerializeReasoningPayloadTest::RunTest(const FString &Parameters) {
  const FReasoningPayloadFixture &Fixture = CodecFixtures().ReasoningPayload;
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  FString Json = rtk::detail::SerializeReasoningResult(
      Fixture.ReasoningText, Fixture.ResponseText);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Reasoning =
      DataAdapters::ReadObjectField(RootObject,
                                    ProcessData.Tape.ReasoningOutput);

  TestEqual(Fixture.Labels.Type,
            DataAdapters::ReadStringField(RootObject,
                                          ProcessData.Instruction.Type),
            Fixture.ExpectedType);
  TestEqual(Fixture.Labels.ReasoningText,
            DataAdapters::ReadStringField(
                Reasoning, ProcessData.ReasoningOutput.ReasoningText),
            Fixture.ReasoningText);
  TestEqual(Fixture.Labels.ResponseText,
            DataAdapters::ReadStringField(
                Reasoning, ProcessData.ReasoningOutput.ResponseText),
            Fixture.ResponseText);
  
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEncodeProcessTapePayloadTest,
    FORBOCAI_SDK_AUTHORED_STRINGV425E87BC6BAA,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FEncodeProcessTapePayloadTest::RunTest(const FString &Parameters)
 */
bool FEncodeProcessTapePayloadTest::RunTest(const FString &Parameters) {
  const FProcessTapePayloadFixture &Fixture =
      CodecFixtures().ProcessTapePayload;
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  const auto &AgentData =
      JsonInterop::AgentConfiguration::agentContractData();
  FNPCProcessTape Tape;
  Tape.Observation = Fixture.Observation;
  Tape.ContextJson = Fixture.ContextJson;
  Tape.NpcState.JsonData = Fixture.NpcStateJson;
  Tape.Persona = Fixture.Persona;
  Tape.bHasStructuredPersona = true;
  
  TSharedRef<FJsonObject> Obj = APISlice::Detail::EncodeProcessTapeObject(Tape);
  FString Json = APISlice::Detail::ToJsonString(Obj);
  TSharedPtr<FJsonObject> Root;
  TestTrue(Fixture.Labels.Payload,
           JsonInterop::ParseJsonObject(Json, Root));
  const TSharedRef<FJsonObject> RootObject = Root.ToSharedRef();
  const TSharedRef<FJsonObject> Context =
      DataAdapters::ReadObjectField(RootObject, ProcessData.Tape.Context);
  const TSharedRef<FJsonObject> StructuredPersona =
      DataAdapters::ReadObjectField(RootObject,
                                    ProcessData.Tape.StructuredPersona);
  const TArray<FString> Traits = DataAdapters::ReadStringArrayField(
      StructuredPersona, AgentData.Persona.Traits);

  TestEqual(Fixture.Labels.Observation,
            DataAdapters::ReadStringField(RootObject,
                                          ProcessData.Tape.Observation),
            Fixture.Observation);
  TestTrue(Fixture.Labels.Persona, Traits == Fixture.ExpectedTraits);
  TestEqual(Fixture.Labels.ContextTime,
            DataAdapters::ReadStringField(Context, TEXT("time")),
            Fixture.ExpectedContextTime);
  
  return true;
}
