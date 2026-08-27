#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Testing::TwoNpcChat {

struct FTwoNpcChatTestingLabels {
  FString Configuration;
  FString Numbers;
  FString GenerateValues;
  FString GenerateTokens;
  FString CreateToken;
  FString DecideTokens;
  FString MemoryTokens;
  FString LineTokens;
  FString Context;
};

struct FTwoNpcChatTestingData {
  FString AutomationName;
  FTwoNpcChatTestingLabels Labels;
};

} // namespace MicroGame::Testing::TwoNpcChat
