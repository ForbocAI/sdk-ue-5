#include "Core/fp.hpp"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Vector/MemoryVectorAdapters.h"
#include "Systems/Testing/Memory/TestingMemoryAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::Memory;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMemoryVectorTest, TestingMemoryVectorFixtures().Suite,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a semantic-memory verifier, I need the production vectorizer tested against natural and unrelated questions so recall regressions fail before release. @fn bool FMemoryVectorTest::RunTest(const FString &Parameters) */
bool FMemoryVectorTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const FMemoryVectorTestFixtures &Fixtures =
      TestingMemoryVectorFixtures();
  const TArray<float> Fact = MemoryVectorAdapters::embed(Fixtures.Fact);
  const TArray<float> RepeatedFact =
      MemoryVectorAdapters::embed(Fixtures.Fact);
  const float Norm = FMath::Sqrt(func::fold_array<float, float>(
      Fact, MemoryConfiguration::memoryData().Defaults.Similarity,
      [](float Sum, float Value) { return Sum + Value * Value; }));
  const float Relevant = MemoryVectorAdapters::cosineSimilarity(
      Fact, MemoryVectorAdapters::embed(Fixtures.NaturalQuestion));
  const float Unrelated = MemoryVectorAdapters::cosineSimilarity(
      Fact, MemoryVectorAdapters::embed(Fixtures.UnrelatedQuestion));

  TestTrue(Fixtures.Stories.Deterministic, Fact == RepeatedFact);
  TestEqual(Fixtures.Stories.Normalized, Fact.Num(),
            MemoryConfiguration::memoryData().Vector.Dimension);
  TestTrue(Fixtures.Stories.Normalized,
           FMath::IsNearlyEqual(Norm, Fixtures.UnitNorm, Fixtures.Precision));
  TestTrue(Fixtures.Stories.NaturalQuestion,
           Relevant >= Fixtures.MinimumRelevantSimilarity);
  TestTrue(Fixtures.Stories.UnrelatedQuestion,
           Unrelated < Fixtures.MaximumUnrelatedSimilarity);
  return true;
}
