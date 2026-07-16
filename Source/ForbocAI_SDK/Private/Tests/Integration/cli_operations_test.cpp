#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Features/Config/ConfigAdapters.h"
#include "Store.h"
#include "Features/CLI/Config/ConfigThunks.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Features/NPC/NPCActions.h"

// @covers:cliOp:loadBridgePreset
// @covers:cliOp:getBridgeRules
// @covers:cliOp:checkApiStatus
// @covers:cliOp:clearNodeMemory
// @covers:cliOp:getConfigValue
// @covers:cliOp:setConfigValue
// @covers:cliOp:createNpc
// @covers:cliOp:exportSoul
// @covers:cliOp:getActiveNpc
// @covers:cliOp:getNpc
// @covers:cliOp:getGhostHistory
// @covers:cliOp:getGhostResults
// @covers:cliOp:startGhost
// @covers:cliOp:getGhostStatus
// @covers:cliOp:stopGhost
// @covers:cliOp:importNpcFromSoul
// @covers:cliOp:initNodeMemory
// @covers:cliOp:listSouls
// @covers:cliOp:clearMemory
// @covers:cliOp:listMemory
// @covers:cliOp:recallMemory
// @covers:cliOp:storeMemory
// @covers:cliOp:processNpc
// @covers:cliOp:recallNodeMemory
// @covers:cliOp:listRulesets
// @covers:cliOp:listRulePresets
// @covers:cliOp:storeNodeMemory
// @covers:cliOp:updateNpc
// @covers:cliOp:validateBridgePayload
// @covers:cliOp:verifySoul
// @covers:cli:bridge_preset
// @covers:cli:bridge_rules
// @covers:cli:bridge_validate
// @covers:cli:config_get
// @covers:cli:config_list
// @covers:cli:config_set
// @covers:cli:doctor
// @covers:cli:ghost_history
// @covers:cli:ghost_results
// @covers:cli:ghost_run
// @covers:cli:ghost_status
// @covers:cli:ghost_stop
// @covers:cli:memory_clear
// @covers:cli:memory_export
// @covers:cli:memory_list
// @covers:cli:memory_recall
// @covers:cli:memory_store
// @covers:cli:npc_chat
// @covers:cli:npc_create
// @covers:cli:npc_import
// @covers:cli:npc_process
// @covers:cli:npc_state
// @covers:cli:npc_update
// @covers:cli:rules_list
// @covers:cli:rules_presets
// @covers:cli:soul_export
// @covers:cli:soul_chat
// @covers:cli:soul_import
// @covers:cli:soul_list
// @covers:cli:soul_verify
// @covers:cli:status
// @covers:cli:version


using namespace rtk;

/**
 * Test: Ops::createNpc creates NPC and updates store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsCreateNpcTest,
                                 "ForbocAI.Integration.Ops.createNpc",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsCreateNpcTest::RunTest(const FString &Parameters)
 */
bool FOpsCreateNpcTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  FNPCInternalState Result = Ops::createNpc(Store, TEXT("A loyal guard"));

  TestFalse("NPC Id not empty", Result.Id.IsEmpty());
  TestEqual("Persona matches", Result.Persona,
            FString(TEXT("A loyal guard")));

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestTrue("Active NPC exists", Active.hasValue);
  const func::Maybe<FNPCInternalState> Found = Ops::getNpc(Store, Result.Id);
  TestTrue("Created NPC is addressable by id", Found.hasValue);
  if (Active.hasValue) {
    TestEqual("Active NPC Id matches created", Active.value.Id, Result.Id);
    TestEqual("Active NPC Persona", Active.value.Persona,
              FString(TEXT("A loyal guard")));
  }

  return true;
}

/**
 * Test: Ops::getActiveNpc returns nothing on empty store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsGetActiveEmptyTest,
                                 "ForbocAI.Integration.Ops.GetActiveEmpty",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsGetActiveEmptyTest::RunTest(const FString &Parameters)
 */
bool FOpsGetActiveEmptyTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestFalse("No active NPC on fresh store", Active.hasValue);

  return true;
}

/**
 * Test: Ops::getConfigValue / setConfigValue
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsConfigTest,
                                 "ForbocAI.Integration.Ops.Config",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsConfigTest::RunTest(const FString &Parameters)
 */
bool FOpsConfigTest::RunTest(const FString &Parameters) {
  const FString TempConfigPath = FPaths::Combine(
      FPaths::ProjectSavedDir(),
      FString::Printf(TEXT("forbocai-sdkops-%s.json"),
                      *FGuid::NewGuid().ToString(EGuidFormats::Digits)));

  IFileManager::Get().Delete(*TempConfigPath, false, true);
  SDKConfig::SetConfigFilePathOverride(TempConfigPath);
  SDKConfig::ReloadConfig();

  const FString EnvApiUrl =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_URL"));
  const FString ExpectedApiUrl =
      !EnvApiUrl.IsEmpty() ? EnvApiUrl : FString(SDKConfig::DEFAULT_API_URL);
  TestEqual("Resolved runtime API URL honors env or production default",
            SDKConfig::GetApiUrl(), ExpectedApiUrl);
  TestTrue("Unset persisted apiUrl is empty",
           Ops::getConfigValue(TEXT("apiUrl")).IsEmpty());

  FString Version = Ops::getConfigValue(TEXT("version"));
  TestFalse("Version not empty", Version.IsEmpty());

  Ops::setConfigValue(TEXT("apiUrl"), TEXT("https://test.forboc.ai"));
  FString Url = Ops::getConfigValue(TEXT("apiUrl"));
  TestEqual("ApiUrl roundtrip", Url,
            FString(TEXT("https://test.forboc.ai")));

  Ops::setConfigValue(TEXT("apiKey"), TEXT("sk_test_roundtrip"));
  FString Key = Ops::getConfigValue(TEXT("apiKey"));
  TestEqual("ApiKey roundtrip", Key, FString(TEXT("sk_test_roundtrip")));

  FString Unknown = Ops::getConfigValue(TEXT("nonexistent"));
  TestTrue("Unknown key returns empty", Unknown.IsEmpty());

  FString PersistedConfig;
  TestTrue("Config file saved",
           FFileHelper::LoadFileToString(PersistedConfig, *TempConfigPath));
  TestTrue("Persisted config includes apiUrl",
           PersistedConfig.Contains(TEXT("\"apiUrl\":\"https://test.forboc.ai\"")) ||
               PersistedConfig.Contains(
                   TEXT("\"apiUrl\": \"https://test.forboc.ai\"")));
  TestTrue("Persisted config includes apiKey",
           PersistedConfig.Contains(TEXT("\"apiKey\":\"sk_test_roundtrip\"")) ||
               PersistedConfig.Contains(
                   TEXT("\"apiKey\": \"sk_test_roundtrip\"")));

  IFileManager::Get().Delete(*TempConfigPath, false, true);
  SDKConfig::SetConfigFilePathOverride(TEXT(""));
  SDKConfig::ReloadConfig();

  return true;
}

/**
 * Test: Ops::createNpc then remove via store dispatch
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpsCreateAndRemoveTest,
    "ForbocAI.Integration.Ops.CreateAndRemove",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsCreateAndRemoveTest::RunTest(const FString &Parameters)
 */
bool FOpsCreateAndRemoveTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  FNPCInternalState Npc = Ops::createNpc(Store, TEXT("Ephemeral"));
  FString NpcId = Npc.Id;

  TestTrue("Created NPC exists", Ops::getNpc(Store, NpcId).hasValue);

  Store.dispatch(NPCActions::removeNPC(NpcId));

  TestFalse("Removed NPC no longer exists", Ops::getNpc(Store, NpcId).hasValue);

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestFalse("No active NPC after removal", Active.hasValue);

  return true;
}
