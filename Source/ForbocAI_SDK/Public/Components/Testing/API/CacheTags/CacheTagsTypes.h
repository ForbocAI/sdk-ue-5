#pragma once

#include "Core/rtk.hpp"

namespace Testing::API::CacheTags {

struct FCacheTagLabels {
  FString Suite;
  FString Npc;
  FString Bridge;
  FString BridgeList;
  FString Ghost;
  FString GhostList;
  FString Rule;
  FString RuleList;
  FString Soul;
  FString SoulList;
  FString System;
};

struct FCacheTagIds {
  FString Npc;
  FString Bridge;
  FString Ghost;
  FString Rule;
  FString Soul;
};

struct FCacheTagValues {
  rtk::FApiEndpointTag Npc;
  rtk::FApiEndpointTag Bridge;
  rtk::FApiEndpointTag BridgeList;
  rtk::FApiEndpointTag Ghost;
  rtk::FApiEndpointTag GhostList;
  rtk::FApiEndpointTag Rule;
  rtk::FApiEndpointTag RuleList;
  rtk::FApiEndpointTag Soul;
  rtk::FApiEndpointTag SoulList;
  rtk::FApiEndpointTag System;
};

struct FCacheTagFixtures {
  FCacheTagLabels Labels;
  FCacheTagIds Ids;
  FCacheTagValues Tags;
};

} // namespace Testing::API::CacheTags
