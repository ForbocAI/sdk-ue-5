#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationTypes.h"

namespace TestGame::VerificationAdapters {

/** User Story: As a selector verifier, I need terminal and scenario interactions decoded from authored data so the mechanics test contains behavior rather than content. @fn inline Verification::FMechanicsVerificationData ReadMechanics(const TSharedRef<FJsonObject> &Root) */
inline Verification::FMechanicsVerificationData
ReadMechanics(const TSharedRef<FJsonObject> &Root) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Root, TEXT("mechanics"));
  const TSharedRef<FJsonObject> Stealth =
      DataAdapters::ReadObjectField(Object, TEXT("stealth"));
  const TSharedRef<FJsonObject> Social =
      DataAdapters::ReadObjectField(Object, TEXT("social"));
  const TSharedRef<FJsonObject> Trade =
      DataAdapters::ReadObjectField(Social, TEXT("tradeOffer"));
  const TSharedRef<FJsonObject> Ui =
      DataAdapters::ReadObjectField(Object, TEXT("ui"));
  const TSharedRef<FJsonObject> Transcript =
      DataAdapters::ReadObjectField(Object, TEXT("transcript"));
  const TSharedRef<FJsonObject> Coverage =
      DataAdapters::ReadObjectField(Object, TEXT("coverage"));
  const TSharedRef<FJsonObject> Scenario =
      DataAdapters::ReadObjectField(Object, TEXT("scenario"));
  Verification::FMechanicsVerificationData Data;
  Data.suite = DataAdapters::ReadStringField(Object, TEXT("suite"));
  Data.automationName =
      DataAdapters::ReadStringField(Object, TEXT("automationName"));
  Data.story = DataAdapters::ReadStringField(Object, TEXT("story"));
  Data.stealth.doorOpen =
      DataAdapters::ReadBooleanField(Stealth, TEXT("doorOpen"));
  Data.stealth.alertDelta =
      DataAdapters::ReadNumberField(Stealth, TEXT("alertDelta"));
  Data.social.dialogue =
      DataAdapters::ReadStringField(Social, TEXT("dialogue"));
  Data.social.tradeOffer.NpcId =
      DataAdapters::ReadStringField(Trade, TEXT("npcId"));
  Data.social.tradeOffer.Item =
      DataAdapters::ReadStringField(Trade, TEXT("item"));
  Data.social.tradeOffer.Price =
      DataAdapters::ReadNumberField(Trade, TEXT("price"));
  Data.ui.message = DataAdapters::ReadStringField(Ui, TEXT("message"));
  Data.ui.expectedMessageCount =
      DataAdapters::ReadNumberField(Ui, TEXT("expectedMessageCount"));
  Data.transcript.scenarioId =
      DataAdapters::ReadStringField(Transcript, TEXT("scenarioId"));
  Data.transcript.statusCommand =
      DataAdapters::ReadStringField(Transcript, TEXT("statusCommand"));
  Data.transcript.npcCommand =
      DataAdapters::ReadStringField(Transcript, TEXT("npcCommand"));
  Data.transcript.expectedEntryCount =
      DataAdapters::ReadNumberField(Transcript, TEXT("expectedEntryCount"));
  Data.transcript.expectedErrorCount =
      DataAdapters::ReadNumberField(Transcript, TEXT("expectedErrorCount"));
  Data.coverage.requiredGroups =
      DataAdapters::ReadStringArrayField(Coverage, TEXT("requiredGroups"));
  Data.coverage.coveredGroup =
      DataAdapters::ReadStringField(Coverage, TEXT("coveredGroup"));
  Data.coverage.expectedCoveredCount =
      DataAdapters::ReadNumberField(Coverage, TEXT("expectedCoveredCount"));
  Data.coverage.expectedMissingCount =
      DataAdapters::ReadNumberField(Coverage, TEXT("expectedMissingCount"));
  Data.scenario.expectedStepCount =
      DataAdapters::ReadNumberField(Scenario, TEXT("expectedStepCount"));
  return Data;
}

} // namespace TestGame::VerificationAdapters
