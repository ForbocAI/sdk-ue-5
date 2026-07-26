#include "Features/API/Endpoints/NPC/Conversation/Configuration/ConversationConfigurationAdapters.h"
#include "Features/API/Endpoints/NPC/Conversation/ConversationAdapters.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Misc/AutomationTest.h"

namespace {

struct FConversationCodecStories {
  FString Acceptance;
  FString EmptyProfileList;
  FString TurnIndex;
  FString BlankDialogue;
  FString EmptyTranscript;
};

struct FConversationCodecFixture {
  FString AutomationName;
  FConversationCodecStories Stories;
  FString ResponseJson;
  int32 FirstIndex{};
  int32 InvalidTurnIndex{};
  FString InvalidDialogue;
};

/** User Story: As a UE codec test, I need all conversation examples and assertions loaded from canonical JSON. @fn const FConversationCodecFixture &conversationCodecFixture() */
const FConversationCodecFixture &conversationCodecFixture() {
  static const FConversationCodecFixture Fixture = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(
            TEXT("ForbocAI_SDK"),
            TEXT("Data/tests/api/npc-conversation.json"));
    const TSharedRef<FJsonObject> Stories =
        DataAdapters::ReadObjectField(Source, TEXT("stories"));
    const TSharedRef<FJsonObject> Indices =
        DataAdapters::ReadObjectField(Source, TEXT("indices"));
    const TSharedRef<FJsonObject> Replacements =
        DataAdapters::ReadObjectField(Source, TEXT("replacements"));
    return FConversationCodecFixture{
        DataAdapters::ReadStringField(Source, TEXT("automationName")),
        {DataAdapters::ReadStringField(Stories, TEXT("acceptance")),
         DataAdapters::ReadStringField(Stories, TEXT("emptyProfileList")),
         DataAdapters::ReadStringField(Stories, TEXT("turnIndex")),
         DataAdapters::ReadStringField(Stories, TEXT("blankDialogue")),
         DataAdapters::ReadStringField(Stories, TEXT("emptyTranscript"))},
        DataAdapters::SerializeObject(
            DataAdapters::ReadObjectField(Source, TEXT("response"))),
        DataAdapters::ReadNumberField(Indices, TEXT("first")),
        DataAdapters::ReadNumberField(Replacements, TEXT("turnIndex")),
        DataAdapters::ReadStringField(Replacements, TEXT("dialogue"))};
  }();
  return Fixture;
}

/**
 * User Story: As a UE codec test, I need an independent response object for each mutation so rejection cases cannot influence one another.
 * @fn TSharedRef<FJsonObject> freshConversationResponse( const FString &ResponseJson)
 */
TSharedRef<FJsonObject> freshConversationResponse(
    const FString &ResponseJson) {
  TSharedPtr<FJsonObject> Root;
  JsonInterop::ParseJsonObject(ResponseJson, Root);
  return Root.ToSharedRef();
}

/** User Story: As a UE codec test, I need a complete object decoded through the production boundary without test-owned response logic. @fn bool conversationResponseDecodes(const TSharedRef<FJsonObject> &Root) */
bool conversationResponseDecodes(const TSharedRef<FJsonObject> &Root) {
  FNPCConversationResponse Response;
  return APISlice::Detail::DecodeNpcConversationResponse(
      JsonInterop::StringifyObject(Root), Response);
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FNpcConversationCodecContractTest,
    conversationCodecFixture().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a UE SDK maintainer, I need the conversation codec checked against JSON-owned acceptance and rejection cases.
 * @fn bool FNpcConversationCodecContractTest::RunTest( const FString &Parameters)
 */
bool FNpcConversationCodecContractTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  using namespace APISlice::Endpoints::NPCConversationConfiguration;
  const FConversationCodecFixture &Fixture = conversationCodecFixture();
  const FConversationFields &Fields = conversationConfigurationData().Fields;

  TestTrue(*Fixture.Stories.Acceptance,
           conversationResponseDecodes(
               freshConversationResponse(Fixture.ResponseJson)));

  const TSharedRef<FJsonObject> EmptyProfile =
      freshConversationResponse(Fixture.ResponseJson);
  DataAdapters::ReadArrayField(EmptyProfile, Fields.Participants)
      [Fixture.FirstIndex]
          ->AsObject()
          ->SetArrayField(Fields.Traits,
                          TArray<TSharedPtr<FJsonValue>>());
  TestFalse(*Fixture.Stories.EmptyProfileList,
            conversationResponseDecodes(EmptyProfile));

  const TSharedRef<FJsonObject> InvalidIndex =
      freshConversationResponse(Fixture.ResponseJson);
  DataAdapters::ReadArrayField(InvalidIndex, Fields.Turns)
      [Fixture.FirstIndex]
          ->AsObject()
          ->SetNumberField(Fields.Index, Fixture.InvalidTurnIndex);
  TestFalse(*Fixture.Stories.TurnIndex,
            conversationResponseDecodes(InvalidIndex));

  const TSharedRef<FJsonObject> BlankDialogue =
      freshConversationResponse(Fixture.ResponseJson);
  DataAdapters::ReadArrayField(BlankDialogue, Fields.Turns)
      [Fixture.FirstIndex]
          ->AsObject()
          ->SetStringField(Fields.Dialogue, Fixture.InvalidDialogue);
  TestFalse(*Fixture.Stories.BlankDialogue,
            conversationResponseDecodes(BlankDialogue));

  const TSharedRef<FJsonObject> EmptyTranscript =
      freshConversationResponse(Fixture.ResponseJson);
  EmptyTranscript->SetArrayField(Fields.TranscriptLines,
                                 TArray<TSharedPtr<FJsonValue>>());
  TestFalse(*Fixture.Stories.EmptyTranscript,
            conversationResponseDecodes(EmptyTranscript));
  return true;
}
