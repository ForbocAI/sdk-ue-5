#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Testing/RTK/ToolkitSurface/ToolkitSurfaceAdapters.h"
#include "Misc/AutomationTest.h"

using namespace rtk;
namespace ToolkitSurface = Testing::RTK::ToolkitSurface;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRtkActionAndUtilitySurfaceTest,
    ToolkitSurface::ToolkitSurfaceFixtures().Names.ActionAndUtility,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK consumer, I need RTK action and utility exports to
 * preserve their executable contracts rather than only sharing JS names.
 * @fn bool FRtkActionAndUtilitySurfaceTest::RunTest(const FString &Parameters)
 */
bool FRtkActionAndUtilitySurfaceTest::RunTest(const FString &Parameters) {
  const ToolkitSurface::FToolkitSurfaceFixtures &Fixture =
      ToolkitSurface::ToolkitSurfaceFixtures();
  ActionCreatorWithPreparedPayload<FString, int32> PreparedCreator{
      TEXT("surface/prepared"),
      [&Fixture](int32 Value) {
        PreparedAction<FString> Prepared{
            FString::Format(*Fixture.Prepared.Format,
                            {FStringFormatArg(Value)}),
            {},
            Fixture.Prepared.bError};
        Prepared.Meta.Add(Fixture.Prepared.MetaKey, Fixture.Prepared.MetaValue);
        return Prepared;
      }};
  const AnyAction Prepared = PreparedCreator(Fixture.Prepared.Input);
  const func::Maybe<FString> PreparedPayload =
      Prepared.getPayload<FString>();
  TestTrue(Fixture.Labels.PreparedPayload, func::is_just(PreparedPayload));
  TestEqual(Fixture.Labels.PreparedTransform, PreparedPayload.value,
            Fixture.Prepared.ExpectedValue);
  TestEqual(Fixture.Labels.PreparedMetadata,
            Prepared.Meta.FindRef(Fixture.Prepared.MetaKey),
            Fixture.Prepared.MetaValue);
  TestEqual(Fixture.Labels.PreparedError, Prepared.bError,
            Fixture.Prepared.bError);

  const ActionCreatorWithOptionalPayload<int32> OptionalCreator{
      TEXT("surface/optional")};
  const AnyAction Optional = OptionalCreator();
  TestFalse(Fixture.Labels.OptionalPayload,
            static_cast<bool>(Optional.PayloadWrapper));
  TestEqual(Fixture.Labels.OptionalType, Optional.Type, OptionalCreator.Type);

  int32 DispatchCount = Fixture.Utility.InitialDispatchCount;
  const ActionCreator<int32> BoundAction =
      createAction<int32>(TEXT("surface/bound"));
  const std::function<AnyAction(int32)> BoundCreator = bindActionCreators(
      BoundAction, [&DispatchCount](const AnyAction &ActionValue) {
        ++DispatchCount;
        return ActionValue;
      });
  TestEqual(Fixture.Labels.BoundAction,
            BoundCreator(Fixture.Utility.BoundPayload).Type, BoundAction.Type);
  TestEqual(Fixture.Labels.BoundDispatch, DispatchCount,
            Fixture.Utility.ExpectedDispatchCount);

  const Tuple<int32> Values(Fixture.Utility.InitialValues);
  const Tuple<int32> Combined =
      Values.prepend(Fixture.Utility.Head).concat(Fixture.Utility.Suffix);
  TestEqual(Fixture.Labels.TupleOrder, Combined.Values,
            Fixture.Utility.ExpectedValues);

  const int32 Original = Fixture.Utility.Original;
  const int32 Next = createNextState(
      Original, [&Fixture](int32 &DraftValue) {
        DraftValue += Fixture.Utility.DraftDelta;
      });
  TestEqual(Fixture.Labels.NextOriginal, Original, Fixture.Utility.Original);
  TestEqual(Fixture.Labels.NextResult, Next, Fixture.Utility.ExpectedNext);
  TestEqual(Fixture.Labels.NanoidSize,
            nanoid(Fixture.Utility.NanoidSize).Len(),
            Fixture.Utility.NanoidSize);
  TestTrue(Fixture.Labels.NanoidUnique,
           nanoid(Fixture.Utility.NanoidSize) !=
               nanoid(Fixture.Utility.NanoidSize));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRtkAutoBatchSurfaceTest,
    ToolkitSurface::ToolkitSurfaceFixtures().Names.AutoBatch,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a store subscriber, I need low-priority RTK actions to update
 * state immediately while coalescing view notifications into one scheduled job.
 * @fn bool FRtkAutoBatchSurfaceTest::RunTest(const FString &Parameters)
 */
bool FRtkAutoBatchSurfaceTest::RunTest(const FString &Parameters) {
  const ToolkitSurface::FToolkitSurfaceFixtures &Fixture =
      ToolkitSurface::ToolkitSurfaceFixtures();
  const ActionCreator<int32> Increment =
      createAction<int32>(TEXT("counter/increment"));
  const CaseReducer<int32> Reducer =
      [Increment](const int32 &State, const AnyAction &ActionValue) {
        const func::Maybe<int32> Amount = Increment.extract(ActionValue);
        return func::is_just(Amount) ? State + Amount.value : State;
      };

  std::vector<std::function<void()>> Notifications;
  AutoBatchOptions Options;
  Options.Schedule = AutoBatchOptions::Type::Callback;
  Options.QueueNotification =
      [&Notifications](std::function<void()> Notification) {
        Notifications.push_back(std::move(Notification));
      };
  EnhancedStore<int32> StoreValue = configureStore<int32>(
      Reducer, Fixture.AutoBatch.InitialState,
      std::vector<Middleware<int32>>(), autoBatchEnhancer(Options));

  int32 SubscriberCalls = Fixture.AutoBatch.InitialSubscriberCalls;
  StoreValue.subscribe([&SubscriberCalls]() { ++SubscriberCalls; });
  StoreValue.dispatch(prepareAutoBatched<int32>(
      Increment.Type, Fixture.AutoBatch.FirstAmount));
  StoreValue.dispatch(prepareAutoBatched<int32>(
      Increment.Type, Fixture.AutoBatch.SecondAmount));
  TestEqual(Fixture.Labels.BatchState, StoreValue.getState(),
            Fixture.AutoBatch.ExpectedBatchedState);
  TestEqual(Fixture.Labels.BatchQueue,
            static_cast<int32>(Notifications.size()),
            Fixture.AutoBatch.ExpectedFirstQueueSize);
  TestEqual(Fixture.Labels.BatchWait, SubscriberCalls,
            Fixture.AutoBatch.InitialSubscriberCalls);
  Notifications[Fixture.AutoBatch.FirstNotificationIndex]();
  TestEqual(Fixture.Labels.BatchNotify, SubscriberCalls,
            Fixture.AutoBatch.ExpectedFirstSubscriberCalls);

  StoreValue.dispatch(prepareAutoBatched<int32>(
      Increment.Type, Fixture.AutoBatch.ThirdAmount));
  StoreValue.dispatch(Increment(Fixture.AutoBatch.NormalAmount));
  TestEqual(Fixture.Labels.NormalState, StoreValue.getState(),
            Fixture.AutoBatch.ExpectedFinalState);
  TestEqual(Fixture.Labels.NormalNotify, SubscriberCalls,
            Fixture.AutoBatch.ExpectedFinalSubscriberCalls);
  Notifications[Fixture.AutoBatch.SecondNotificationIndex]();
  TestEqual(Fixture.Labels.StaleNotify, SubscriberCalls,
            Fixture.AutoBatch.ExpectedFinalSubscriberCalls);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRtkMiddlewareSurfaceTest,
    ToolkitSurface::ToolkitSurfaceFixtures().Names.Middleware,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a store owner, I need dynamic and invariant middleware to
 * execute against the live root store after configuration.
 * @fn bool FRtkMiddlewareSurfaceTest::RunTest(const FString &Parameters)
 */
bool FRtkMiddlewareSurfaceTest::RunTest(const FString &Parameters) {
  const ToolkitSurface::FToolkitSurfaceFixtures &Fixture =
      ToolkitSurface::ToolkitSurfaceFixtures();
  const ActionCreatorWithoutPayload Ticked =
      createAction(TEXT("counter/ticked"));
  const CaseReducer<int32> Reducer =
      [Ticked, &Fixture](const int32 &State, const AnyAction &ActionValue) {
        return Ticked.match(ActionValue)
                   ? State + Fixture.Middleware.ExpectedDynamicCalls
                   : State;
      };
  DynamicMiddlewareInstance<int32> Dynamic = createDynamicMiddleware<int32>();
  ImmutableStateInvariantMiddlewareOptions<int32> ImmutableOptions;
  int32 ImmutableChecks = Fixture.Middleware.InitialImmutableChecks;
  ImmutableOptions.IsImmutable = [&ImmutableChecks](const int32 &) {
    ++ImmutableChecks;
    return true;
  };

  std::vector<Middleware<int32>> Middlewares;
  Middlewares.push_back(Dynamic.middleware());
  Middlewares.push_back(
      createImmutableStateInvariantMiddleware<int32>(ImmutableOptions));
  Middlewares.push_back(createSerializableStateInvariantMiddleware<int32>());
  EnhancedStore<int32> StoreValue = configureStore<int32>(
      Reducer, Fixture.Middleware.InitialState, Middlewares);

  int32 DynamicCalls = Fixture.Middleware.InitialDynamicCalls;
  StoreValue.dispatch(Ticked());
  TestEqual(Fixture.Labels.DynamicEmpty, DynamicCalls,
            Fixture.Middleware.InitialDynamicCalls);

  Dynamic.addMiddleware(
      [&DynamicCalls](const MiddlewareApi<int32> &) {
        return [&DynamicCalls](Dispatcher NextDispatch) {
          return [&DynamicCalls,
                  NextDispatch](const AnyAction &ActionValue) {
            ++DynamicCalls;
            return NextDispatch(ActionValue);
          };
        };
      });
  StoreValue.dispatch(Ticked());
  TestEqual(Fixture.Labels.DynamicLive, DynamicCalls,
            Fixture.Middleware.ExpectedDynamicCalls);
  TestEqual(Fixture.Labels.InvariantCount, ImmutableChecks,
            Fixture.Middleware.ExpectedImmutableChecks);
  TestEqual(Fixture.Labels.ReducerState, StoreValue.getState(),
            Fixture.Middleware.ExpectedFinalState);

  TestTrue(Fixture.Labels.EmptyActionPath,
           func::is_just(findNonSerializableValue(AnyAction())));
  TestFalse(Fixture.Labels.TypedAction,
            func::is_just(findNonSerializableValue(Ticked())));
  return true;
}
