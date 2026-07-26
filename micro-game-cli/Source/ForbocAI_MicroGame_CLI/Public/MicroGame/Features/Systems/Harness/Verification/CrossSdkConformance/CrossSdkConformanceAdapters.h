#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/BridgeGrid/BridgeGridAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/Mechanics/MechanicsAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/Rtk/RtkAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/StateDomains/StateDomainsAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/CrossSdkConformanceTypes.h"

namespace MicroGame::CrossSdkConformanceAdapters {

/** User Story: As a cross-SDK verifier, I need the complete cross-SDK conformance contract loaded once so every UE test consumes the same immutable data as TS. @fn inline const CrossSdkConformance::FCrossSdkConformanceData &CrossSdkConformanceData() */
inline const CrossSdkConformance::FCrossSdkConformanceData &CrossSdkConformanceData() {
  static const CrossSdkConformance::FCrossSdkConformanceData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("tests/cross-sdk-conformance.json"));
    CrossSdkConformance::FCrossSdkConformanceData Value;
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

} // namespace MicroGame::CrossSdkConformanceAdapters
