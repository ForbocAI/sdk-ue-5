#include "Features/API/APIApi.h"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/Testing/API/Codec/APICodecAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecSoulVerifyAliasTest, "ForbocAI.Core.API.SoulVerifyAliases",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiCodecSoulVerifyAliasTest::RunTest(const FString &Parameters) {
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
    FApiCodecBridgeRulesAliasTest, "ForbocAI.Core.API.BridgeRuleAliases",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiCodecBridgeRulesAliasTest::RunTest(const FString &Parameters) {
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
    FApiCodecRulesetAliasTest, "ForbocAI.Core.API.RulesetAliases",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiCodecRulesetAliasTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FRulesetFixture &Fixture =
      Testing::API::Codec::CodecFixtures().Ruleset;

  FDirectiveRuleSet Ruleset;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeDirectiveRuleSetResponse(
               Fixture.ResponseJson, Ruleset));
  TestEqual(*Fixture.Labels.Id, Ruleset.Id, Fixture.ExpectedId);
  TestEqual(*Fixture.Labels.RulesetId, Ruleset.RulesetId,
            Fixture.ExpectedRulesetId);
  TestEqual(*Fixture.Labels.RuleCount, Ruleset.RulesetRules.Num(),
            Fixture.ExpectedRuleCount);
  if (Ruleset.RulesetRules.IsValidIndex(Fixture.FirstRuleIndex)) {
    const auto &Rule = Ruleset.RulesetRules[Fixture.FirstRuleIndex];
    TestEqual(*Fixture.Labels.RuleName, Rule.RuleName,
              Fixture.ExpectedRuleName);
    TestEqual(*Fixture.Labels.ActionCount, Rule.RuleActionTypes.Num(),
              Fixture.ExpectedActionCount);
    if (Rule.RuleActionTypes.IsValidIndex(Fixture.FirstActionIndex)) {
      TestEqual(*Fixture.Labels.Action,
                Rule.RuleActionTypes[Fixture.FirstActionIndex],
                Fixture.ExpectedAction);
    }
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
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecBridgeValidationWrapperTest,
    "ForbocAI.Core.API.BridgeValidationWrapper",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiCodecBridgeValidationWrapperTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FBridgeValidationFixture &Fixture =
      Testing::API::Codec::CodecFixtures().BridgeValidation;

  FValidationResult Result;
  TestTrue(*Fixture.Labels.Decode,
           APISlice::Detail::DecodeValidationResult(Fixture.ResponseJson,
                                                    Result));
  TestEqual(*Fixture.Labels.Valid, Result.bValid, Fixture.bExpectedValid);
  TestEqual(*Fixture.Labels.Reason, Result.Reason, Fixture.ExpectedReason);
  return true;
}

/**
 * Response normalization: gaType→type, actionReason→reason, actionTarget→target
 * (Haskell API may return aliased field names; JsonInterop::ActionFromObject normalizes)
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCodecActionFromObjectGaTypeTest,
    "ForbocAI.Core.API.ActionFromObjectGaTypeAliases",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiCodecActionFromObjectGaTypeTest::RunTest(const FString &Parameters) {
  const Testing::API::Codec::FActionAliasesFixture &Fixture =
      Testing::API::Codec::CodecFixtures().ActionAliases;

  const FAgentAction Action = JsonInterop::ActionFromObject(Fixture.Input);

  TestEqual(*Fixture.Labels.Type, Action.Type, Fixture.ExpectedType);
  TestEqual(*Fixture.Labels.Target, Action.Target, Fixture.ExpectedTarget);
  TestEqual(*Fixture.Labels.Reason, Action.Reason, Fixture.ExpectedReason);
  return true;
}
