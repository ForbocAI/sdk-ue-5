#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/BridgeGrid/BridgeGridAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/Mechanics/MechanicsAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/Rtk/RtkAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/StateDomains/StateDomainsAdapters.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationTypes.h"

namespace TestGame::VerificationAdapters {

/** User Story: As a cross-SDK verifier, I need the complete architecture test contract loaded once so every UE test consumes the same immutable data as TS. @fn inline const Verification::FArchitectureVerificationData &ArchitectureTestData() */
inline const Verification::FArchitectureVerificationData &ArchitectureTestData() {
  static const Verification::FArchitectureVerificationData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("tests/architecture.json"));
    Verification::FArchitectureVerificationData Value;
    Value.bridgeGrid = ReadBridgeGrid(Source.Root);
    const TSharedRef<FJsonObject> Coverage =
        DataAdapters::ReadObjectField(Source, TEXT("coverage"));
    Value.coverage.suite =
        DataAdapters::ReadStringField(Coverage, TEXT("suite"));
    Value.coverage.story =
        DataAdapters::ReadStringField(Coverage, TEXT("story"));
    Value.rtk = ReadRtk(Source.Root);
    Value.stateDomains = ReadStateDomains(Source.Root);
    Value.mechanics = ReadMechanics(Source.Root);
    return Value;
  }();
  return Data;
}

} // namespace TestGame::VerificationAdapters
