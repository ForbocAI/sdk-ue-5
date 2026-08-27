#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/TwoNpcChatAdapters.h"
#include "MicroGame/Features/Systems/Testing/Harness/TwoNpcChat/TwoNpcChatAdapters.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTwoNpcChatAuthoredCompositionTest,
    MicroGame::Testing::TwoNpcChat::TwoNpcChatTestingData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a micro-game maintainer, I need two-NPC command composition executed against its production JSON so a stale path or bypassed token cannot pass compilation alone.
 * @fn bool FTwoNpcChatAuthoredCompositionTest::RunTest( const FString &Parameters)
 */
bool FTwoNpcChatAuthoredCompositionTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  using namespace MicroGame::TwoNpcChat;
  const FTwoNpcChatConfig &Config = TwoNpcChatConfig();
  const auto &Labels =
      MicroGame::Testing::TwoNpcChat::TwoNpcChatTestingData().Labels;

  TestTrue(*Labels.Configuration,
           !Config.Group.IsEmpty() && !Config.Header.IsEmpty() &&
               !Config.Footer.IsEmpty());
  TestTrue(*Labels.Numbers,
           Config.Numbers.NextIndex > Config.Numbers.InitialIndex);

  const FString Generate =
      FormatGenerate(Config, Config.NameAttribute, Config.SeedAttribute);
  TestTrue(*Labels.GenerateValues,
           Generate.Contains(Config.NameAttribute) &&
               Generate.Contains(Config.SeedAttribute));
  TestFalse(*Labels.GenerateTokens,
            Generate.Contains(Config.Tokens.Attribute) ||
                Generate.Contains(Config.Tokens.Context));

  const FString Create = FormatCreate(Config, Config.NameAttribute);
  TestFalse(*Labels.CreateToken, Create.Contains(Config.Tokens.Persona));
  const FString Decide =
      FormatDecide(Config, Config.NameAttribute, Config.SeedAttribute);
  TestFalse(*Labels.DecideTokens,
            Decide.Contains(Config.Tokens.Id) ||
                Decide.Contains(Config.Tokens.Line));
  const FString Memory =
      FormatMemoryStore(Config, Config.NameAttribute, Config.SeedAttribute);
  TestFalse(*Labels.MemoryTokens,
            Memory.Contains(Config.Tokens.Id) ||
                Memory.Contains(Config.Tokens.Memory));
  const FString Line =
      FormatLine(Config, Config.NameAttribute, Config.SeedAttribute);
  TestFalse(*Labels.LineTokens,
            Line.Contains(Config.Tokens.Name) ||
                Line.Contains(Config.Tokens.Dialogue));
  TestEqual(*Labels.Context,
            ExtendContext(Config, Config.NameAttribute,
                          Config.SeedAttribute),
            Config.NameAttribute + Config.ContextSeparator +
                Config.SeedAttribute);
  return true;
}
