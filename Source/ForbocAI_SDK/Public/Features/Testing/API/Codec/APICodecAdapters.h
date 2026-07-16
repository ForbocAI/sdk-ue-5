#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/Bridge/BridgeAdapters.h"
#include "Features/Testing/API/Codec/Process/ProcessAdapters.h"
#include "Features/Testing/API/Codec/Protocol/ProtocolAdapters.h"
#include "Features/Testing/API/Codec/Rules/RulesAdapters.h"
#include "Features/Testing/API/Codec/Soul/SoulAdapters.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need one immutable fixture catalog composed from domain readers so every codec test shares the same authored contract. @fn inline const FCodecFixtures &CodecFixtures() */
inline const FCodecFixtures &CodecFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/api/codec.json"));
  static const FCodecFixtures Fixtures = {
      ReadSoulVerifyFixture(Source),
      ReadBridgeRulesFixture(Source),
      ReadRulesetFixture(Source),
      ReadNullableProtocolFixture(Source),
      ReadBridgeValidationFixture(Source),
      ReadActionContractFixture(Source),
      ReadIdentifyActorPayloadFixture(Source),
      ReadDecisionPayloadFixture(Source),
      ReadReasoningPayloadFixture(Source),
      ReadProcessTapePayloadFixture(Source),
  };
  return Fixtures;
}

} // namespace Testing::API::Codec
