#pragma once

namespace ecs {


typedef FString EntityKey;
typedef FString ComponentType;
typedef FString Tag;
typedef FString ResourceName;
typedef FString EventType;
typedef FString DomainPathKey;
typedef FString SystemName;

struct FEntityId {
  int64 Index;
  int32 Generation;
};

/**
 * @fn inline FEntityId createEntityId(int64 Index, int32 Generation)
 * @brief Creates a generation-counted entity id.
 *
 * User Story: As an ECS caller, I need stable id values so recycled entity slots
 * do not accidentally resolve stale handles.
 */
inline FEntityId createEntityId(int64 Index, int32 Generation) {
  FEntityId Id;
  Id.Index = Index;
  Id.Generation = Generation;
  return Id;
}

/**
 * @fn inline EntityKey createEntityKey(const FEntityId &Id)
 * @brief Converts an entity id into the shared string key format.
 *
 * User Story: As component storage, I need a deterministic key for TMap-based
 * component tables.
 */
inline EntityKey createEntityKey(const FEntityId &Id) {
  return LexToString(Id.Index) + FString::Chr(TCHAR(':')) +
         LexToString(Id.Generation);
}

/** User Story: As a core ecs types consumer, I need to compare values for equality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator==(const FEntityId &Left, const FEntityId &Right) */
inline bool operator==(const FEntityId &Left, const FEntityId &Right) {
  return Left.Index == Right.Index && Left.Generation == Right.Generation;
}

/** User Story: As a core ecs types consumer, I need to compare values for inequality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator!=(const FEntityId &Left, const FEntityId &Right) */
inline bool operator!=(const FEntityId &Left, const FEntityId &Right) {
  return !(Left == Right);
}

struct FAllocator {
  int64 NextIndex;
  TArray<FEntityId> Freed;
  TMap<int64, int32> Generations;
};

struct FAllocatedEntity {
  FAllocator Allocator;
  FEntityId Entity;
};

/**
 * @fn inline FAllocator createEntityAllocator()
 * @brief Creates an empty entity allocator.
 *
 * User Story: As a world author, I need isolated allocator state for each world
 * so tests, levels, and runtime sessions do not share entity id counters.
 */
inline FAllocator createEntityAllocator() {
  FAllocator Allocator;
  Allocator.NextIndex = int64{};
  return Allocator;
}

/** User Story: As a core ecs types consumer, I need to compare values for equality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator==(const FAllocator &Left, const FAllocator &Right) */
inline bool operator==(const FAllocator &Left, const FAllocator &Right) {
  return Left.NextIndex == Right.NextIndex && Left.Freed == Right.Freed &&
         Left.Generations.OrderIndependentCompareEqual(Right.Generations);
}

/** User Story: As a core ecs types consumer, I need to compare values for inequality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator!=(const FAllocator &Left, const FAllocator &Right) */
inline bool operator!=(const FAllocator &Left, const FAllocator &Right) {
  return !(Left == Right);
}

/** User Story: As a core ecs types consumer, I need to compare values for equality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator==(const FAllocatedEntity &Left, const FAllocatedEntity &Right) */
inline bool operator==(const FAllocatedEntity &Left,
                       const FAllocatedEntity &Right) {
  return Left.Allocator == Right.Allocator && Left.Entity == Right.Entity;
}

/** User Story: As a core ecs types consumer, I need to compare values for inequality through a stable signature so the core ecs types workflow remains explicit and composable. @fn inline bool operator!=(const FAllocatedEntity &Left, const FAllocatedEntity &Right) */
inline bool operator!=(const FAllocatedEntity &Left,
                       const FAllocatedEntity &Right) {
  return !(Left == Right);
}

/**
 * @fn inline func::Maybe<FEntityId> findReusableEntityId(const FAllocator &Allocator)
 * @brief Reads the last reusable entity id from the allocator.
 *
 * User Story: As allocation code, I need the reusable-id branch represented as
 * Maybe so fresh allocation composes without imperative checks.
 */
inline func::Maybe<FEntityId> findReusableEntityId(const FAllocator &Allocator) {
  return !Allocator.Freed.IsEmpty() ? func::just(Allocator.Freed.Last())
                                    : func::nothing<FEntityId>();
}

/**
 * @fn inline FAllocatedEntity allocateRecycledEntity(FAllocator Allocator, const FEntityId &Entity)
 * @brief Allocates a previously freed entity id.
 *
 * User Story: As allocator code, I need recycled allocation to be a small
 * unary-friendly value transform used by the Maybe match branch.
 */
inline FAllocatedEntity allocateRecycledEntity(FAllocator Allocator,
                                               const FEntityId &Entity) {
  FAllocatedEntity Result;
  Allocator.Freed.Pop();
  Result.Entity = Entity;
  Result.Allocator = Allocator;
  return Result;
}

/**
 * @fn inline FAllocatedEntity allocateFreshEntity(FAllocator Allocator)
 * @brief Allocates a fresh entity id and records its generation.
 *
 * User Story: As allocator code, I need the fresh-id branch isolated so entity
 * allocation composes from reusable branch functions.
 */
inline FAllocatedEntity allocateFreshEntity(FAllocator Allocator) {
  FAllocatedEntity Result;
  Result.Entity = createEntityId(Allocator.NextIndex, int32{});
  Allocator.Generations.Add(Allocator.NextIndex, int32{});
  ++Allocator.NextIndex;
  Result.Allocator = Allocator;
  return Result;
}

/**
 * @fn inline FAllocatedEntity allocateEntity(FAllocator Allocator)
 * @brief Allocates a fresh or recycled entity id.
 *
 * User Story: As a spawn mechanic, I need entity allocation to return new
 * allocator state plus the allocated id so state transitions stay value based.
 */
inline FAllocatedEntity allocateEntity(FAllocator Allocator) {
  return func::match(
      findReusableEntityId(Allocator),
      [&Allocator](const FEntityId &Entity) {
        return allocateRecycledEntity(Allocator, Entity);
      },
      [&Allocator]() { return allocateFreshEntity(Allocator); });
}

/**
 * @fn inline bool entityGenerationMatches(const FAllocator &Allocator, const FEntityId &Id)
 * @brief Checks whether an entity id matches its allocator generation.
 *
 * User Story: As allocator code, I need stale-handle checks to share one Maybe
 * lookup predicate across free and alive queries.
 */
inline bool entityGenerationMatches(const FAllocator &Allocator,
                                    const FEntityId &Id) {
  return func::match(
      func::find_map_value<int64, int32>(Allocator.Generations, Id.Index),
      [&Id](const int32 &Current) { return Current == Id.Generation; },
      []() { return false; });
}

/**
 * @fn inline FAllocator freeMatchedEntity(FAllocator Allocator, const FEntityId &Id)
 * @brief Frees an entity id after its generation is known to match.
 *
 * User Story: As despawn code, I need generation incrementing isolated from the
 * Maybe predicate that decides whether an id can be released.
 */
inline FAllocator freeMatchedEntity(FAllocator Allocator, const FEntityId &Id) {
  int32 NextGeneration = Id.Generation;
  ++NextGeneration;
  Allocator.Generations.Add(Id.Index, NextGeneration);
  Allocator.Freed.Add(createEntityId(Id.Index, NextGeneration));
  return Allocator;
}

/**
 * @fn inline FAllocator freeEntityId(FAllocator Allocator, const FEntityId &Id)
 * @brief Frees an entity id and increments its generation for safe reuse.
 *
 * User Story: As a despawn mechanic, I need old handles to stop resolving after
 * their slot has been released.
 */
inline FAllocator freeEntityId(FAllocator Allocator, const FEntityId &Id) {
  return entityGenerationMatches(Allocator, Id)
             ? freeMatchedEntity(Allocator, Id)
             : Allocator;
}

/**
 * @fn inline bool isEntityAlive(const FAllocator &Allocator, const FEntityId &Id)
 * @brief Checks whether an entity id is still alive in the allocator.
 *
 * User Story: As a query system, I need to reject stale handles before reading
 * component data.
 */
inline bool isEntityAlive(const FAllocator &Allocator, const FEntityId &Id) {
  return entityGenerationMatches(Allocator, Id);
}

} // namespace ecs
