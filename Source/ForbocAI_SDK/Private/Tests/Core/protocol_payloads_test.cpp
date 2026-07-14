/**
 * Protocol payloads isolation tests — tests local serialization codecs
 * Payloads are exercised through their concrete serialization contracts.
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Core/JsonInterop.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Protocol/Requests/RequestsTypes.h"
#include "Features/API/APIAdapters.h"

using namespace rtk;

namespace {

FString StringField(const TSharedPtr<FJsonObject> &Object,
                    const TCHAR *FieldName) {
  FString Value;
  Object.IsValid() ? (Object->TryGetStringField(FieldName, Value), void())
                   : void();
  return Value;
}

TSharedPtr<FJsonObject> ObjectField(const TSharedPtr<FJsonObject> &Object,
                                    const TCHAR *FieldName) {
  const TSharedPtr<FJsonObject> *Value = nullptr;
  return Object.IsValid() && Object->TryGetObjectField(FieldName, Value) &&
                 Value != nullptr
             ? *Value
             : TSharedPtr<FJsonObject>();
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSerializeIdentifyActorPayloadTest,
    "ForbocAI.Core.Protocol.SerializeIdentifyActorPayload",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSerializeIdentifyActorPayloadTest::RunTest(const FString &Parameters) {
  FNPCActorInfo Actor;
  Actor.NpcId = TEXT("npc_test_1");
  Actor.Persona = TEXT("Tester");
  Actor.Data.JsonData = FString(TEXT("{") TEXT("\"health\": 100}"));
  
  FString Json = rtk::detail::SerializeIdentifyActorResult(Actor);
  TSharedPtr<FJsonObject> Root;
  TestTrue("Payload parses", JsonInterop::ParseJsonObject(Json, Root));
  const TSharedPtr<FJsonObject> ActorObject = ObjectField(Root, TEXT("actor"));
  const TSharedPtr<FJsonObject> DataObject =
      ObjectField(ActorObject, TEXT("data"));
  double Health = 0.0;

  TestEqual("type", StringField(Root, TEXT("type")),
            FString(TEXT("IdentifyActorResult")));
  TestEqual("npcId", StringField(ActorObject, TEXT("npcId")),
            FString(TEXT("npc_test_1")));
  TestEqual("persona", StringField(ActorObject, TEXT("persona")),
            FString(TEXT("Tester")));
  TestTrue("data health", DataObject.IsValid() &&
                           DataObject->TryGetNumberField(TEXT("health"),
                                                         Health));
  TestEqual("health value", static_cast<int32>(Health), 100);
  
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
  FString Json =
      rtk::detail::SerializeDecisionResult(TEXT("Respond"), TEXT("SPEAK"),
                                           TEXT("Player"));
  TSharedPtr<FJsonObject> Root;
  TestTrue("Payload parses", JsonInterop::ParseJsonObject(Json, Root));
  const TSharedPtr<FJsonObject> Intent =
      ObjectField(Root, TEXT("decisionIntent"));

  TestEqual("type", StringField(Root, TEXT("type")),
            FString(TEXT("Decision")));
  TestEqual("goal", StringField(Intent, TEXT("goal")),
            FString(TEXT("Respond")));
  TestEqual("actionType", StringField(Intent, TEXT("actionType")),
            FString(TEXT("SPEAK")));
  TestEqual("target", StringField(Intent, TEXT("target")),
            FString(TEXT("Player")));
  
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
  FString Json = rtk::detail::SerializeReasoningResult(
      TEXT("Thinking..."), TEXT("Hello there"));
  TSharedPtr<FJsonObject> Root;
  TestTrue("Payload parses", JsonInterop::ParseJsonObject(Json, Root));
  const TSharedPtr<FJsonObject> Reasoning =
      ObjectField(Root, TEXT("reasoningOutput"));

  TestEqual("type", StringField(Root, TEXT("type")),
            FString(TEXT("Reasoning")));
  TestEqual("reasoningText",
            StringField(Reasoning, TEXT("reasoningText")),
            FString(TEXT("Thinking...")));
  TestEqual("responseText",
            StringField(Reasoning, TEXT("responseText")),
            FString(TEXT("Hello there")));
  
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
  FNPCProcessTape Tape;
  Tape.Observation = TEXT("Saw player");
  Tape.ContextJson = FString(TEXT("{") TEXT("\"time\":\"day\"}"));
  Tape.NpcState.JsonData = FString(TEXT("{") TEXT("}"));
  Tape.Persona = TEXT("Guard");
  
  TSharedRef<FJsonObject> Obj = APISlice::Detail::EncodeProcessTapeObject(Tape);
  FString Json = APISlice::Detail::ToJsonString(Obj);
  TSharedPtr<FJsonObject> Root;
  TestTrue("Payload parses", JsonInterop::ParseJsonObject(Json, Root));
  const TSharedPtr<FJsonObject> Context =
      ObjectField(Root, TEXT("context"));

  TestEqual("observation", StringField(Root, TEXT("observation")),
            FString(TEXT("Saw player")));
  TestEqual("persona", StringField(Root, TEXT("structuredPersona")),
            FString(TEXT("Guard")));
  TestEqual("context time", StringField(Context, TEXT("time")),
            FString(TEXT("day")));
  
  return true;
}
