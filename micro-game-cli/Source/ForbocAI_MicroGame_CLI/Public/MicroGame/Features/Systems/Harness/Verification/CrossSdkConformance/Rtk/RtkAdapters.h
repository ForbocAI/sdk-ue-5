#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/Parsing/CrossSdkConformanceParsingAdapters.h"

namespace MicroGame::CrossSdkConformanceAdapters {

/** User Story: As an RTK verifier, I need player and bridge cases decoded from the same cross-SDK conformance contract used by TS so store behavior remains comparable. @fn inline CrossSdkConformance::FRtkVerificationData ReadRtk(const TSharedRef<FJsonObject> &Root) */
inline CrossSdkConformance::FRtkVerificationData
ReadRtk(const TSharedRef<FJsonObject> &Root) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Root, TEXT("rtk"));
  const TSharedRef<FJsonObject> Automation =
      DataAdapters::ReadObjectField(Object, TEXT("automationNames"));
  const TSharedRef<FJsonObject> Stories =
      DataAdapters::ReadObjectField(Object, TEXT("stories"));
  const TSharedRef<FJsonObject> Player =
      DataAdapters::ReadObjectField(Object, TEXT("player"));
  const TSharedRef<FJsonObject> PlayerPatch =
      DataAdapters::ReadObjectField(Player, TEXT("patch"));
  const TSharedRef<FJsonObject> Bridge =
      DataAdapters::ReadObjectField(Object, TEXT("bridge"));
  const TSharedRef<FJsonObject> BridgePatch =
      DataAdapters::ReadObjectField(Bridge, TEXT("patch"));
  const TSharedRef<FJsonObject> Presets =
      DataAdapters::ReadObjectField(Bridge, TEXT("presets"));
  const TSharedRef<FJsonObject> Social =
      DataAdapters::ReadObjectField(Presets, TEXT("social"));
  const TSharedRef<FJsonObject> Custom =
      DataAdapters::ReadObjectField(Presets, TEXT("custom"));
  CrossSdkConformance::FRtkVerificationData Data;
  Data.suite = DataAdapters::ReadStringField(Object, TEXT("suite"));
  Data.automationNames.player =
      DataAdapters::ReadStringField(Automation, TEXT("player"));
  Data.automationNames.bridge =
      DataAdapters::ReadStringField(Automation, TEXT("bridge"));
  Data.stories.player =
      DataAdapters::ReadStringField(Stories, TEXT("player"));
  Data.stories.bridge =
      DataAdapters::ReadStringField(Stories, TEXT("bridge"));
  Data.player.patch.hp =
      DataAdapters::ReadNumberField(PlayerPatch, TEXT("hp"));
  Data.player.patch.hidden =
      DataAdapters::ReadBooleanField(PlayerPatch, TEXT("hidden"));
  Data.player.patch.position = ReadPosition(
      DataAdapters::ReadObjectField(PlayerPatch, TEXT("position")));
  Data.player.patch.inventory =
      DataAdapters::ReadStringArrayField(PlayerPatch, TEXT("inventory"));
  Data.player.expectedName =
      DataAdapters::ReadStringField(Player, TEXT("expectedName"));
  Data.bridge.patch.maxJumpForce =
      DataAdapters::ReadNumberField(BridgePatch, TEXT("maxJumpForce"));
  Data.bridge.patch.activePreset =
      DataAdapters::ReadStringField(BridgePatch, TEXT("activePreset"));
  Data.bridge.expectedInitialDistance =
      DataAdapters::ReadNumberField(Bridge, TEXT("expectedInitialDistance"));
  Data.bridge.presets.social = {
      DataAdapters::ReadStringField(Social, TEXT("name")),
      DataAdapters::ReadNumberField(Social, TEXT("expectedDistance"))};
  Data.bridge.presets.custom = {
      DataAdapters::ReadStringField(Custom, TEXT("name")),
      DataAdapters::ReadNumberField(Custom, TEXT("expectedDistance"))};
  return Data;
}

} // namespace MicroGame::CrossSdkConformanceAdapters
