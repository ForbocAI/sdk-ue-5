#include "CoreMinimal.h"
#include "Features/API/Endpoints/NPC/Process/NPCProcessAdapters.h"
#include "Features/Testing/API/Process/ProcessAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::API::Process;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FNPCProcessCompleteResponsesTest,
    "ForbocAI.Core.API.NPCProcessCompleteResponses",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As an SDK consumer, I need complete process responses decoded without dropping nested API state so the protocol loop receives the exact authored contract. @fn bool FNPCProcessCompleteResponsesTest::RunTest(const FString &Parameters) */
bool FNPCProcessCompleteResponsesTest::RunTest(const FString &Parameters) {
  const FProcessCodecFixture &Fixture = ProcessCodecFixture();
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  const auto &AgentData =
      JsonInterop::AgentConfiguration::agentContractData();
  const auto &MemoryData =
      JsonInterop::MemoryConfiguration::memoryContractData();

  FNPCProcessResponse QueryResponse;
  TestTrue(Fixture.Labels.QueryVector,
           APISlice::Detail::DecodeNpcProcessResponse(
               Fixture.QueryVectorResponseJson, QueryResponse));
  TSharedPtr<FJsonObject> QueryExpectedRoot;
  TestTrue(Fixture.Labels.QueryVector,
           JsonInterop::ParseJsonObject(Fixture.QueryVectorExpectedJson,
                                        QueryExpectedRoot));
  const TSharedRef<FJsonObject> QueryExpectedInstruction =
      DataAdapters::ReadObjectField(QueryExpectedRoot.ToSharedRef(),
                                    ProcessData.Response.Instruction);
  const TSharedRef<FJsonObject> QueryExpectedTape =
      DataAdapters::ReadObjectField(QueryExpectedRoot.ToSharedRef(),
                                    ProcessData.Response.Tape);
  const TSharedRef<FJsonObject> QueryExpectedConstraints =
      DataAdapters::ReadObjectField(QueryExpectedTape,
                                    ProcessData.Tape.Constraints);
  const TSharedRef<FJsonObject> QueryExpectedPersona =
      DataAdapters::ReadObjectField(QueryExpectedTape,
                                    ProcessData.Tape.StructuredPersona);
  const TSharedRef<FJsonObject> QueryPersona =
      JsonInterop::StructuredPersonaToObject(QueryResponse.Tape.Persona);

  TestTrue(Fixture.Labels.QueryVector,
           QueryResponse.Instruction.Type ==
               ENPCInstructionType::QueryVector);
  TestEqual(Fixture.Labels.QueryVector, QueryResponse.Instruction.Query,
            DataAdapters::ReadStringField(QueryExpectedInstruction,
                                          ProcessData.Instruction.Query));
  TestEqual(Fixture.Labels.QueryVector, QueryResponse.Instruction.Limit,
            DataAdapters::ReadNumberField(QueryExpectedInstruction,
                                          ProcessData.Instruction.Limit));
  TestTrue(Fixture.Labels.QueryVector,
           FMath::IsNearlyEqual(
               QueryResponse.Instruction.Threshold,
               DataAdapters::ReadFloatField(QueryExpectedInstruction,
                                            ProcessData.Instruction.Threshold)));
  TestEqual(Fixture.Labels.QueryVector, QueryResponse.Tape.Observation,
            DataAdapters::ReadStringField(QueryExpectedTape,
                                          ProcessData.Tape.Observation));
  TestTrue(Fixture.Labels.QueryVector,
           QueryResponse.Tape.bHasStructuredPersona);
  TestTrue(
      Fixture.Labels.QueryVector,
      DataAdapters::ReadStringArrayField(QueryPersona, AgentData.Persona.Traits) ==
          DataAdapters::ReadStringArrayField(QueryExpectedPersona,
                                             AgentData.Persona.Traits));
  TestTrue(Fixture.Labels.QueryVector, QueryResponse.Tape.bHasPrompt);
  TestEqual(Fixture.Labels.QueryVector, QueryResponse.Tape.Prompt,
            DataAdapters::ReadStringField(QueryExpectedTape,
                                          ProcessData.Tape.Prompt));
  TestTrue(Fixture.Labels.QueryVector, QueryResponse.Tape.bHasConstraints);
  TestEqual(Fixture.Labels.QueryVector,
            QueryResponse.Tape.Constraints.MaxTokens,
            DataAdapters::ReadNumberField(
                QueryExpectedConstraints,
                ProcessData.PromptConstraints.MaxTokens));
  TestTrue(Fixture.Labels.QueryVector,
           FMath::IsNearlyEqual(
               QueryResponse.Tape.Constraints.Temperature,
               DataAdapters::ReadFloatField(
                   QueryExpectedConstraints,
                   ProcessData.PromptConstraints.Temperature)));
  TestTrue(Fixture.Labels.QueryVector,
           QueryResponse.Tape.Constraints.Stop ==
               DataAdapters::ReadStringArrayField(
                   QueryExpectedConstraints,
                   ProcessData.PromptConstraints.Stop));
  TestTrue(Fixture.Labels.QueryVector,
           FMath::IsNearlyEqual(
               QueryResponse.Tape.Constraints.RepeatPenalty,
               DataAdapters::ReadFloatField(
                   QueryExpectedConstraints,
                   ProcessData.PromptConstraints.RepeatPenalty)));
  TestEqual(Fixture.Labels.QueryVector, QueryResponse.Tape.Constraints.Seed,
            DataAdapters::ReadNumberField(QueryExpectedConstraints,
                                          ProcessData.PromptConstraints.Seed));
  TestTrue(Fixture.Labels.QueryVector,
           QueryResponse.Tape.bHasVectorQueried &&
               QueryResponse.Tape.bVectorQueried);

  FNPCProcessResponse FinalizeResponse;
  TestTrue(Fixture.Labels.Finalize,
           APISlice::Detail::DecodeNpcProcessResponse(
               Fixture.FinalizeResponseJson, FinalizeResponse));
  TSharedPtr<FJsonObject> FinalizeExpectedRoot;
  TestTrue(Fixture.Labels.Finalize,
           JsonInterop::ParseJsonObject(Fixture.FinalizeExpectedJson,
                                        FinalizeExpectedRoot));
  const TSharedRef<FJsonObject> FinalizeExpectedInstruction =
      DataAdapters::ReadObjectField(FinalizeExpectedRoot.ToSharedRef(),
                                    ProcessData.Response.Instruction);
  const TSharedRef<FJsonObject> FinalizeExpectedTape =
      DataAdapters::ReadObjectField(FinalizeExpectedRoot.ToSharedRef(),
                                    ProcessData.Response.Tape);
  const TArray<TSharedPtr<FJsonValue>> FinalizeExpectedMemories =
      DataAdapters::ReadArrayField(FinalizeExpectedInstruction,
                                   ProcessData.Finalize.MemoryStore);
  const TSharedRef<FJsonObject> FinalizeExpectedMemory =
      FinalizeExpectedMemories.Last()->AsObject().ToSharedRef();
  const TSharedRef<FJsonObject> FinalizeExpectedAction =
      DataAdapters::ReadObjectField(FinalizeExpectedInstruction,
                                    ProcessData.Finalize.Action);
  const TSharedRef<FJsonObject> FinalizeExpectedAudit =
      DataAdapters::ReadObjectField(FinalizeExpectedInstruction,
                                    ProcessData.Finalize.RuleAudit);
  const TArray<TSharedPtr<FJsonValue>> FinalizeExpectedRules =
      DataAdapters::ReadArrayField(FinalizeExpectedAudit,
                                   ProcessData.RuleAudit.AppliedRules);
  const TSharedRef<FJsonObject> FinalizeExpectedRule =
      FinalizeExpectedRules.Last()->AsObject().ToSharedRef();

  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Instruction.Type == ENPCInstructionType::Finalize);
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Instruction.bValid,
            DataAdapters::ReadBooleanField(FinalizeExpectedInstruction,
                                           ProcessData.Finalize.Valid));
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Instruction.Signature,
            DataAdapters::ReadStringField(FinalizeExpectedInstruction,
                                          ProcessData.Finalize.Signature));
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.storeMemory.Num(),
            FinalizeExpectedMemories.Num());
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.storeMemory.Last().Text,
            DataAdapters::ReadStringField(FinalizeExpectedMemory,
                                          MemoryData.StoreInstruction.Text));
  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Instruction.bHasAction);
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.Action.Type,
            DataAdapters::ReadStringField(FinalizeExpectedAction,
                                          AgentData.Action.Type));
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.Action.Target,
            DataAdapters::ReadStringField(FinalizeExpectedAction,
                                          AgentData.Action.Target));
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Instruction.Dialogue,
            DataAdapters::ReadStringField(FinalizeExpectedInstruction,
                                          ProcessData.Finalize.Dialogue));
  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Instruction.bHasRuleAudit);
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.RuleAudit.AppliedRules.Num(),
            FinalizeExpectedRules.Num());
  TestEqual(Fixture.Labels.Finalize,
            FinalizeResponse.Instruction.RuleAudit.AppliedRules.Last().RuleId,
            DataAdapters::ReadStringField(FinalizeExpectedRule,
                                          ProcessData.RuleVerdict.RuleId));
  TestTrue(Fixture.Labels.Finalize, FinalizeResponse.Tape.bHasActor);
  const TSharedRef<FJsonObject> FinalizeExpectedActor =
      DataAdapters::ReadObjectField(FinalizeExpectedTape,
                                    ProcessData.Tape.Actor);
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Tape.Actor.NpcId,
            DataAdapters::ReadStringField(FinalizeExpectedActor,
                                          ProcessData.Actor.Id));
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Tape.Memories.Num(),
            DataAdapters::ReadArrayField(FinalizeExpectedTape,
                                         ProcessData.Tape.Memories)
                .Num());
  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Tape.bDecisionCompleted);
  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Tape.bReasoningCompleted);
  TestTrue(Fixture.Labels.Finalize,
           FinalizeResponse.Tape.bHasGeneratedOutput);
  TestEqual(Fixture.Labels.Finalize, FinalizeResponse.Tape.GeneratedOutput,
            DataAdapters::ReadStringField(FinalizeExpectedTape,
                                          ProcessData.Tape.GeneratedOutput));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FNPCProcessRejectsMalformedResponsesTest,
    "ForbocAI.Core.API.NPCProcessRejectsMalformedResponses",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK consumer, I need malformed process responses rejected atomically so no partial API data can overwrite the current protocol tape.
 * @fn bool FNPCProcessRejectsMalformedResponsesTest::RunTest( const FString &Parameters)
 */
bool FNPCProcessRejectsMalformedResponsesTest::RunTest(
    const FString &Parameters) {
  const FProcessCodecFixture &Fixture = ProcessCodecFixture();
  TSharedPtr<FJsonObject> ExpectedRoot;
  JsonInterop::ParseJsonObject(Fixture.QueryVectorExpectedJson, ExpectedRoot);
  const auto &ProcessData =
      APISlice::NPCProcessConfiguration::processContractData();
  const FString Sentinel = DataAdapters::ReadStringField(
      DataAdapters::ReadObjectField(ExpectedRoot.ToSharedRef(),
                                    ProcessData.Response.Tape),
      ProcessData.Tape.Observation);
  FNPCProcessResponse Response;

  Response.Tape.Observation = Sentinel;
  TestFalse(Fixture.Labels.RejectsAction,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.MalformedActionResponseJson, Response));
  TestEqual(Fixture.Labels.RejectsAction, Response.Tape.Observation, Sentinel);
  Response.Tape.Observation = Sentinel;
  TestFalse(Fixture.Labels.RejectsMemory,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.MalformedMemoryResponseJson, Response));
  TestEqual(Fixture.Labels.RejectsMemory, Response.Tape.Observation, Sentinel);
  Response.Tape.Observation = Sentinel;
  TestFalse(Fixture.Labels.RejectsActor,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.MalformedActorResponseJson, Response));
  TestEqual(Fixture.Labels.RejectsActor, Response.Tape.Observation, Sentinel);
  Response.Tape.Observation = Sentinel;
  TestFalse(Fixture.Labels.RejectsInstruction,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.UnknownInstructionResponseJson, Response));
  TestEqual(Fixture.Labels.RejectsInstruction, Response.Tape.Observation,
            Sentinel);
  return true;
}
