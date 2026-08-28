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
    FProjectApiAnalysisResponseTest,
    FORBOCAI_SDK_AUTHORED_STRINGVA7F828161E65,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a protocol consumer, I need API-owned thought, reasoning,
 * prompt, dialogue, and action results projected without SDK reinterpretation.
 * @fn bool FProjectApiAnalysisResponseTest::RunTest(const FString &Parameters)
 */
bool FProjectApiAnalysisResponseTest::RunTest(const FString &Parameters) {
  const FAgentResponseProjectionFixture &Fixture =
      CodecFixtures().AgentResponseProjection;
  FNPCInstruction Instruction;
  Instruction.Dialogue = Fixture.Dialogue;
  Instruction.bHasAction = true;
  Instruction.Action.Type = Fixture.ActionType;
  FNPCProcessTape Tape;
  Tape.bHasDecisionIntent = true;
  Tape.DecisionIntent.Goal = Fixture.Goal;
  Tape.DecisionIntent.ActionType = Fixture.DecisionActionType;
  Tape.DecisionIntent.Target = Fixture.Target;
  Tape.bHasReasoningOutput = true;
  Tape.ReasoningOutput.ReasoningText = Fixture.ReasoningText;
  Tape.ReasoningOutput.ResponseText = Fixture.ResponseText;
  Tape.bHasPrompt = true;
  Tape.Prompt = Fixture.Prompt;

  const FAgentResponse Response =
      rtk::detail::BuildAgentResponse(Instruction, Tape);
  TestEqual(Fixture.Labels.Dialogue, Response.Dialogue, Fixture.Dialogue);
  TestTrue(Fixture.Labels.Action,
           Response.bHasAction && Response.Action.Type == Fixture.ActionType);
  TestTrue(Fixture.Labels.Thought,
           Response.bHasThoughtResult &&
               Response.ThoughtResult.Goal == Fixture.Goal &&
               Response.ThoughtResult.ActionType ==
                   Fixture.DecisionActionType);
  TestTrue(Fixture.Labels.Reasoning,
           Response.bHasReasoningResult &&
               Response.ReasoningResult.ReasoningText ==
                   Fixture.ReasoningText &&
               Response.ReasoningResult.ResponseText == Fixture.ResponseText);
  TestTrue(Fixture.Labels.Prompt,
           Response.bHasPrompt && Response.Prompt == Fixture.Prompt);
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
  Tape.ThoughtProfile = Fixture.ThoughtProfile;
  Tape.bHasThoughtProfile = true;
  Tape.LegalActions = Fixture.LegalActions;
  Tape.bHasLegalActions = true;
  Tape.VisitedActions = Fixture.VisitedActions;
  Tape.bHasVisitedActions = true;
  Tape.AvoidActions = Fixture.AvoidActions;
  Tape.bHasAvoidActions = true;
  
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
            DataAdapters::ReadStringField(Context, Fixture.ContextTimeField),
            Fixture.ExpectedContextTime);
  TestEqual(Fixture.Labels.ThoughtProfile,
            DataAdapters::ReadStringField(
                RootObject, ProcessData.Tape.ThoughtProfile),
            Fixture.ExpectedThoughtProfile);
  TestEqual(Fixture.Labels.LegalActions,
            DataAdapters::ReadStringArrayField(
                RootObject, ProcessData.Tape.LegalActions).Num(),
            Fixture.ExpectedLegalActionCount);
  TestEqual(Fixture.Labels.VisitedActions,
            DataAdapters::ReadStringArrayField(
                RootObject, ProcessData.Tape.VisitedActions).Num(),
            Fixture.ExpectedVisitedActionCount);
  TestEqual(Fixture.Labels.AvoidActions,
            DataAdapters::ReadStringArrayField(
                RootObject, ProcessData.Tape.AvoidActions).Num(),
            Fixture.ExpectedAvoidActionCount);
  
  return true;
}
