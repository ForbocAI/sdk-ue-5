#include "Core/fp.hpp"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFPArrayRangeTest, "ForbocAI.Core.FP.Array.Range",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK feature maintainer, I need reusable FP array ranges to
 * preserve order and clamp bounds so feature domains do not reimplement loops.
 * @fn bool FFPArrayRangeTest::RunTest(const FString &Parameters)
 */
bool FFPArrayRangeTest::RunTest(const FString &Parameters) {
  const TArray<int32> Values{2, 4, 6, 8};

  TestTrue(TEXT("take preserves the requested prefix"),
           func::take_array(Values, 2) == TArray<int32>({2, 4}));
  TestTrue(TEXT("drop preserves the requested suffix"),
           func::drop_array(Values, 2) == TArray<int32>({6, 8}));
  TestTrue(TEXT("slice clamps an oversized count"),
           func::slice_array(Values, 1, 99) == TArray<int32>({4, 6, 8}));
  TestTrue(TEXT("slice clamps a start past the end"),
           func::slice_array(Values, 99, 1).IsEmpty());
  return true;
}
