#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Systems/Testing/API/Codec/Bridge/CodecBridgeAdapters.h"
#include "Systems/Testing/API/Codec/Process/CodecProcessAdapters.h"
#include "Systems/Testing/API/Codec/Protocol/ProtocolAdapters.h"
#include "Systems/Testing/API/Codec/Rules/CodecRulesAdapters.h"
#include "Systems/Testing/API/Codec/Soul/CodecSoulAdapters.h"

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
      ReadAgentResponseProjectionFixture(Source),
      ReadProcessTapePayloadFixture(Source),
  };
  return Fixtures;
}

} // namespace Testing::API::Codec
