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
  FString Thought;
  FString Memory;
  FString Turn;
  FString Role;
  FString Type;
  FString Output;
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
  FString SceneAttribute;
  FString GenerateCommand;
  FString CreateCommand;
  FString IdentityUpdateCommand;
  FString RecallCommand;
  FString DecideCommand;
  FString MemoryStoreCommand;
  FString TypedMemoryStoreCommand;
  FString SceneMemoryType;
  FString RelationshipMemoryType;
  TMap<FString, FString> AttributeMemoryTypes;
  FString ValueField;
  FString ValueType;
  FString PersonaSeparator;
  FString ContextSeparator;
  FString EmptyContext;
  FString IdPattern;
  FString LineFormat;
  FString ThoughtFormat;
  FString TitleFormat;
  FString RoleAttribute;
  FString OutputLineSeparator;
  FString BlankLine;
  FString DialogueFallback;
  FString UnavailablePrefix;
  FString CommandFailed;
  FString ContextQuote;
  FString DebugHeader;
  FString DebugFooter;
  FString DebugPromptsEnvKey;
  FString DebugPromptsEnvValue;
  FTwoNpcChatTokens Tokens;
  FTwoNpcChatNumbers Numbers;
};

} // namespace MicroGame::TwoNpcChat
