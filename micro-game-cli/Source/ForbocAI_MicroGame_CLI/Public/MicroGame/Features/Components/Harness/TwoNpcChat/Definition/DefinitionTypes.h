#pragma once

#include "CoreMinimal.h"

namespace MicroGame::TwoNpcChat {

struct FTwoNpcChatTokens {
  FString Attribute;
  FString Context;
  FString Persona;
  FString Id;
  FString Line;
  FString Name;
  FString Dialogue;
  FString Memory;
};

struct FTwoNpcChatNumbers {
  int32 InitialIndex{};
  int32 NextIndex{};
};

struct FTwoNpcChatConfig {
  FString Group;
  FString Header;
  FString Footer;
  int32 TurnCount{};
  int32 ParticipantCount{};
  FString NameAttribute;
  TArray<FString> PersonaAttributes;
  FString SeedAttribute;
  FString GenerateCommand;
  FString CreateCommand;
  FString DecideCommand;
  FString MemoryStoreCommand;
  FString ValueField;
  FString PersonaSeparator;
  FString ContextSeparator;
  FString EmptyContext;
  FString IdPattern;
  FString LineFormat;
  FString DialogueFallback;
  FString ContextQuote;
  FTwoNpcChatTokens Tokens;
  FTwoNpcChatNumbers Numbers;
};

} // namespace MicroGame::TwoNpcChat
