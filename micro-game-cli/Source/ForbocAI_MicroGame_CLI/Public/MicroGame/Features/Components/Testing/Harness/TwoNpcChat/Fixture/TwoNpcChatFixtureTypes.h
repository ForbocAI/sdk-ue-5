#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Testing::TwoNpcChat {

struct FTwoNpcChatTestingLabels {
  FString Configuration;
  FString Numbers;
  FString GenerateValues;
  FString GenerateTokens;
  FString CreateToken;
  FString IdentityTokens;
  FString RecallToken;
  FString DecideTokens;
  FString MemoryTokens;
  FString TypedMemoryTokens;
  FString LineTokens;
  FString ThoughtTokens;
  FString DisplayNameTokens;
  FString TypedMemories;
  FString Context;
};

struct FTwoNpcChatTestingData {
  FString AutomationName;
  FTwoNpcChatTestingLabels Labels;
};

} // namespace MicroGame::Testing::TwoNpcChat
