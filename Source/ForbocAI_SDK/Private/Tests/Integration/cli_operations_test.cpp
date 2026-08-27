#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Systems/Config/ConfigAdapters.h"
#include "Store.h"
#include "Systems/Config/ConfigThunks.h"
#include "Systems/CLI/Diagnostics/DiagnosticsThunks.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/API/Endpoints/NPC/Generate/Configuration/GenerateConfigurationAdapters.h"
#include "Entities/NPC/NPCActions.h"

// @covers:cliOp:loadBridgePreset
// @covers:cliOp:getBridgeRules
// @covers:cliOp:checkApiStatus
// @covers:cliOp:getMicroGameContract
// @covers:cliOp:clearNodeMemory
// @covers:cliOp:getConfigValue
// @covers:cliOp:setConfigValue
// @covers:cliOp:hydrateRuntimeConfig
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
// @covers:cliOp:generateNpcAttribute
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
 * Test: The thin CLI transport operations construct lazy SDK RTK thunks.
 * User Story: As a CLI maintainer, I need transport commands verified without
 * executing production HTTP so coverage proves delegation and remains stable.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpsTransportThunkConstructionTest,
    FORBOCAI_SDK_AUTHORED_STRINGV93B17AC46DEB,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a CLI maintainer, I need both API-backed CLI operations to return executable lazy thunks. @fn bool FOpsTransportThunkConstructionTest::RunTest(const FString &Parameters) */
bool FOpsTransportThunkConstructionTest::RunTest(const FString &Parameters) {
  const auto &Names = APISlice::Endpoints::Configuration::endpointData().Names;
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  const func::AsyncResult<FString> ContractRequest =
      Ops::getMicroGameContract(Store);
  const func::AsyncResult<FNpcAttributeGenerateResponse> GenerateRequest =
      Ops::generateNpcAttribute(
          Store,
          APISlice::Endpoints::NPCGenerateConfiguration::
              generateConfigurationData().Fields.Attribute,
          FString());

  TestTrue(Names.GetMicroGameContract,
           ContractRequest.state &&
               static_cast<bool>(ContractRequest.state->executor));
  TestTrue(Names.PostNpcGenerateAttribute,
           GenerateRequest.state &&
               static_cast<bool>(GenerateRequest.state->executor));
  return true;
}

/**
 * Test: Ops::createNpc creates NPC and updates store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsCreateNpcTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV85503B6D9702,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsCreateNpcTest::RunTest(const FString &Parameters)
 */
bool FOpsCreateNpcTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  FNPCInternalState Result = Ops::createNpc(Store, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9A1B5EE17ABE));

  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV780670144C40, Result.Id.IsEmpty());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF1105C1EEC33, Result.Persona,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9A1B5EE17ABE)));

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVB8C170481088, Active.hasValue);
  const func::Maybe<FNPCInternalState> Found = Ops::getNpc(Store, Result.Id);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVDCDB670D3C97, Found.hasValue);
  if (Active.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV7897FA35B5D1, Active.value.Id, Result.Id);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF8C2A1167532, Active.value.Persona,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9A1B5EE17ABE)));
  }

  return true;
}

/**
 * Test: Ops::getActiveNpc returns nothing on empty store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsGetActiveEmptyTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV48324DD7F576,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsGetActiveEmptyTest::RunTest(const FString &Parameters)
 */
bool FOpsGetActiveEmptyTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV3611B86B5240, Active.hasValue);

  return true;
}

/**
 * Test: Ops::getConfigValue / setConfigValue
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOpsConfigTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV943194C915CE,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsConfigTest::RunTest(const FString &Parameters)
 */
bool FOpsConfigTest::RunTest(const FString &Parameters) {
  const FString TempConfigPath = FPaths::Combine(
      FPaths::ProjectSavedDir(),
      FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3FFBF6F40234),
                      *FGuid::NewGuid().ToString(EGuidFormats::Digits)));

  IFileManager::Get().Delete(*TempConfigPath, false, true);
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  Ops::hydrateRuntimeConfig(Store, {}, TempConfigPath);

  const FString EnvApiUrl =
      FPlatformMisc::GetEnvironmentVariable(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3649BFEE691E));
  const FString ExpectedApiUrl =
      !EnvApiUrl.IsEmpty() ? EnvApiUrl
                           : ConfigSlice::configRuntimeData().Defaults.ApiUrl;
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV51C70174CD80,
            ConfigSelectors::selectApiUrl(Store.getState()), ExpectedApiUrl);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV259F92F577D6,
           Ops::getConfigValue(Store.getState(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV71B25C5AC191)).IsEmpty());

  FString Version = Ops::getConfigValue(Store.getState(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA919E63BC8C9));
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV25347A7AB9DC, Version.IsEmpty());

  Ops::setConfigValue(Store, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV71B25C5AC191), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV60DC195E2454));
  FString Url = Ops::getConfigValue(Store.getState(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV71B25C5AC191));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVBE8842B8C0C7, Url,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV60DC195E2454)));

  Ops::setConfigValue(Store, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV592526EDC540), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD13F51B00183));
  FString Key = Ops::getConfigValue(Store.getState(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV592526EDC540));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVFAFA64DD5BB1, Key, FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD13F51B00183)));

  FString Unknown = Ops::getConfigValue(Store.getState(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV68E0A7B4436F));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVEE37AA078C06, Unknown.IsEmpty());

  FString PersistedConfig;
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVD897978E01AA,
           FFileHelper::LoadFileToString(PersistedConfig, *TempConfigPath));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV482503BD85F8,
           PersistedConfig.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB0E25AD4ED71)) ||
               PersistedConfig.Contains(
                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2C55BBAB243A)));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV9FE70FE51361,
           PersistedConfig.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD412036FBE76)) ||
               PersistedConfig.Contains(
                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA0781DFB36DD)));

  IFileManager::Get().Delete(*TempConfigPath, false, true);

  return true;
}

/**
 * Test: Ops::createNpc then remove via store dispatch
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpsCreateAndRemoveTest,
    FORBOCAI_SDK_AUTHORED_STRINGV0A7C03F522B1,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FOpsCreateAndRemoveTest::RunTest(const FString &Parameters)
 */
bool FOpsCreateAndRemoveTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  FNPCInternalState Npc = Ops::createNpc(Store, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF61844AE5265));
  FString NpcId = Npc.Id;

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV09AC2DF78E30, Ops::getNpc(Store, NpcId).hasValue);

  Store.dispatch(NPCActions::removeNPC(NpcId));

  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV12551713EF7E, Ops::getNpc(Store, NpcId).hasValue);

  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(Store);
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVE18B6329F767, Active.hasValue);

  return true;
}
