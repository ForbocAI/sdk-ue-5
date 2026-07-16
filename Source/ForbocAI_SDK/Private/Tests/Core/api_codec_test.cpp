#include "Features/API/APIApi.h"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/Testing/API/Codec/APICodecAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecSoulVerifyContractTest, "ForbocAI.Core.API.SoulVerifyContract",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecSoulVerifyContractTest::RunTest(const FString &Parameters)
 */
bool FApiCodecSoulVerifyContractTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FSoulVerifyFixture &Fixture =
      Testing::API::Codec::CodecFixtures().SoulVerify;

  FSoulVerifyResult Result;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeSoulVerifyResponse(Fixture.ResponseJson,
                                                      Result));
  TestEqual(*Fixture.Labels.Valid, Result.bValid, Fixture.bExpectedValid);
  TestEqual(*Fixture.Labels.Reason, Result.Reason, Fixture.ExpectedReason);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecBridgeRulesContractTest, "ForbocAI.Core.API.BridgeRulesContract",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecBridgeRulesContractTest::RunTest(const FString &Parameters)
 */
bool FApiCodecBridgeRulesContractTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FBridgeRulesFixture &Fixture =
      Testing::API::Codec::CodecFixtures().BridgeRules;

  TArray<FBridgeRule> Rules;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeBridgeRulesResponse(Fixture.ResponseJson,
                                                       Rules));
  TestEqual(*Fixture.Labels.Count, Rules.Num(), Fixture.ExpectedCount);
  if (Rules.IsValidIndex(Fixture.FirstIndex)) {
    const FBridgeRule &Rule = Rules[Fixture.FirstIndex];
    TestEqual(*Fixture.Labels.Name, Rule.RuleName, Fixture.ExpectedName);
    TestEqual(*Fixture.Labels.Description, Rule.RuleDescription,
              Fixture.ExpectedDescription);
    TestEqual(*Fixture.Labels.ActionCount, Rule.RuleActionTypes.Num(),
              Fixture.ExpectedActionCount);
  }
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecRulesetContractTest, "ForbocAI.Core.API.RulesetContract",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecRulesetContractTest::RunTest(const FString &Parameters)
 */
bool FApiCodecRulesetContractTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FRulesetFixture &Fixture =
      Testing::API::Codec::CodecFixtures().Ruleset;

  FDirectiveRuleSet Ruleset;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeDirectiveRuleSetResponse(
               Fixture.ResponseJson, Ruleset));
  TestEqual(*Fixture.Labels.RulesetId, Ruleset.RulesetId,
            Fixture.ExpectedRulesetId);
  TestEqual(*Fixture.Labels.RuleCount, Ruleset.RulesetRules.Num(),
            Fixture.ExpectedRuleCount);
  TestEqual(*Fixture.Labels.Template, Ruleset.Template,
            Fixture.ExpectedTemplate);
  if (Ruleset.RulesetRules.IsValidIndex(Fixture.FirstRuleIndex)) {
    const FDirectiveRule &Rule = Ruleset.RulesetRules[Fixture.FirstRuleIndex];
    TestEqual(*Fixture.Labels.RuleId, Rule.RuleId, Fixture.ExpectedRuleId);
    TestEqual(*Fixture.Labels.RuleName, Rule.Name,
              Fixture.ExpectedRuleName);
    TestEqual(*Fixture.Labels.ConditionCount, Rule.Conditions.Num(),
              Fixture.ExpectedConditionCount);
    if (Rule.Conditions.IsValidIndex(Fixture.FirstConditionIndex)) {
      const FDirectiveRuleCondition &Condition =
          Rule.Conditions[Fixture.FirstConditionIndex];
      TestEqual(*Fixture.Labels.ConditionKey, Condition.Key,
                Fixture.ExpectedConditionKey);
      TestEqual(*Fixture.Labels.ConditionValue, Condition.Value,
                Fixture.ExpectedConditionValue);
    }
    TestEqual(*Fixture.Labels.Action, Rule.Action, Fixture.ExpectedAction);
    TestEqual(*Fixture.Labels.Reason, Rule.Reason, Fixture.ExpectedReason);
    TestEqual(*Fixture.Labels.Target, Rule.Target, Fixture.ExpectedTarget);
    TestEqual(*Fixture.Labels.Priority, Rule.Priority,
              Fixture.ExpectedPriority);
    TestEqual(*Fixture.Labels.ObservationPattern, Rule.ObservationPattern,
              Fixture.ExpectedObservationPattern);
    TestEqual(*Fixture.Labels.PromptSuffix, Rule.PromptSuffix,
              Fixture.ExpectedPromptSuffix);
  }
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecNullableProtocolFieldsTest,
    "ForbocAI.Core.API.NullableProtocolFields",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecNullableProtocolFieldsTest::RunTest(const FString &Parameters)
 */
bool FApiCodecNullableProtocolFieldsTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FNullableProtocolFixture &Fixture =
      Testing::API::Codec::CodecFixtures().NullableProtocol;

  FNPCProcessResponse Response;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeNpcProcessResponse(Fixture.ResponseJson,
                                                      Response));
  TestTrue(*Fixture.Labels.Signature,
           Response.Instruction.Signature.IsEmpty());
  TestTrue(*Fixture.Labels.Persona, Response.Tape.Persona.IsEmpty());
  TestTrue(*Fixture.Labels.RulesetId,
           Response.Tape.RulesetId.IsEmpty());
  FNPCProcessResponse MalformedStoreResponse;
  TestFalse(*Fixture.Labels.RejectsMalformedStore,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.MalformedStoreResponseJson,
                MalformedStoreResponse));
  FNPCProcessResponse MalformedRecallResponse;
  TestFalse(*Fixture.Labels.RejectsMalformedRecall,
            APISlice::Detail::DecodeNpcProcessResponse(
                Fixture.MalformedRecallResponseJson,
                MalformedRecallResponse));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecBridgeValidationContractTest,
    "ForbocAI.Core.API.BridgeValidationContract",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecBridgeValidationContractTest::RunTest(const FString &Parameters)
 */
bool FApiCodecBridgeValidationContractTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FBridgeValidationFixture &Fixture =
      Testing::API::Codec::CodecFixtures().BridgeValidation;

  FValidationResult Result;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeValidationResult(Fixture.ResponseJson,
                                                    Result));
  TestEqual(*Fixture.Labels.Valid, Result.bValid, Fixture.bExpectedValid);
  TestEqual(*Fixture.Labels.Reason, Result.Reason, Fixture.ExpectedReason);
  FValidationResult InvalidResult;
  TestFalse(*Fixture.Labels.RejectsInvalidAction,
            APISlice::Detail::DecodeValidationResult(
                Fixture.InvalidActionResponseJson, InvalidResult));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecActionContractTest, "ForbocAI.Core.API.ActionContract",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FApiCodecActionContractTest::RunTest(const FString &Parameters)
 */
bool FApiCodecActionContractTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FActionContractFixture &Fixture =
      Testing::API::Codec::CodecFixtures().ActionContract;

  const func::Maybe<FAgentAction> Decoded =
      JsonInterop::DecodeActionObject(Fixture.Input);
  const bool bDecoded = func::match(
      Decoded,
      [&](const FAgentAction &Action) {
        TestEqual(*Fixture.Labels.Type, Action.Type, Fixture.ExpectedType);
        TestEqual(*Fixture.Labels.Target, Action.Target,
                  Fixture.ExpectedTarget);
        TestEqual(*Fixture.Labels.Reason, Action.Reason,
                  Fixture.ExpectedReason);
        return true;
      },
      []() { return false; });
  TestTrue(*Fixture.Labels.Decode, bDecoded);
  TestTrue(*Fixture.Labels.RejectsInternalNames,
           func::is_nothing(
               JsonInterop::DecodeActionObject(Fixture.InternalInput)));
  return bDecoded;
}
