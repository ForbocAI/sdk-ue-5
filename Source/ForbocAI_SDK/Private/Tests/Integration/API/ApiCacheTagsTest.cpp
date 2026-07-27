#include "Systems/API/Endpoints/Bridge/EndpointsBridgeAdapters.h"
#include "Systems/API/Endpoints/Ghost/EndpointsGhostAdapters.h"
#include "Systems/API/Endpoints/NPC/EndpointsNPCAdapters.h"
#include "Systems/API/Endpoints/Rules/EndpointsRulesAdapters.h"
#include "Systems/API/Endpoints/Soul/EndpointsSoulAdapters.h"
#include "Systems/API/Endpoints/Diagnostics/DiagnosticsAdapters.h"
#include "Systems/Testing/API/CacheTags/CacheTagsAdapters.h"
#include "Misc/AutomationTest.h"

namespace CacheTags = Testing::API::CacheTags;

namespace {

/** User Story: As an API cache-tag test, I need type and identity compared atomically so partial tag matches cannot pass. @fn void TestTag(FAutomationTestBase &Test, const FString &Label, const rtk::FApiEndpointTag &Actual, const rtk::FApiEndpointTag &Expected) */
void TestTag(FAutomationTestBase &Test, const FString &Label,
             const rtk::FApiEndpointTag &Actual,
             const rtk::FApiEndpointTag &Expected) {
  Test.TestTrue(Label,
                Actual.Type == Expected.Type && Actual.Id == Expected.Id);
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiCacheTagsTest, CacheTags::cacheTagFixtures().Labels.Suite,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As an SDK maintainer, I need UE cache-tag adapters verified against the same authored identities as TS. @fn bool FApiCacheTagsTest::RunTest(const FString &Parameters) */
bool FApiCacheTagsTest::RunTest(const FString &Parameters) {
  const CacheTags::FCacheTagFixtures &Fixture =
      CacheTags::cacheTagFixtures();
  TestTag(*this, Fixture.Labels.Npc,
          APISlice::Endpoints::npcTagAdapter(Fixture.Ids.Npc),
          Fixture.Tags.Npc);
  TestTag(*this, Fixture.Labels.Bridge,
          APISlice::Endpoints::bridgeTagAdapter(Fixture.Ids.Bridge),
          Fixture.Tags.Bridge);
  TestTag(*this, Fixture.Labels.BridgeList,
          APISlice::Endpoints::bridgeListTagAdapter(),
          Fixture.Tags.BridgeList);
  TestTag(*this, Fixture.Labels.Ghost,
          APISlice::Endpoints::ghostTagAdapter(Fixture.Ids.Ghost),
          Fixture.Tags.Ghost);
  TestTag(*this, Fixture.Labels.GhostList,
          APISlice::Endpoints::ghostListTagAdapter(),
          Fixture.Tags.GhostList);
  TestTag(*this, Fixture.Labels.Rule,
          APISlice::Endpoints::ruleTagAdapter(Fixture.Ids.Rule),
          Fixture.Tags.Rule);
  TestTag(*this, Fixture.Labels.RuleList,
          APISlice::Endpoints::ruleListTagAdapter(),
          Fixture.Tags.RuleList);
  TestTag(*this, Fixture.Labels.Soul,
          APISlice::Endpoints::soulTagAdapter(Fixture.Ids.Soul),
          Fixture.Tags.Soul);
  TestTag(*this, Fixture.Labels.SoulList,
          APISlice::Endpoints::soulListTagAdapter(),
          Fixture.Tags.SoulList);
  TestTag(*this, Fixture.Labels.System,
          APISlice::Endpoints::diagnosticsTagAdapter(), Fixture.Tags.System);
  return true;
}
