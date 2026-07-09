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
 * @brief Creates a generation-counted entity id.
 * @signature inline FEntityId createEntityId(int64 Index, int32 Generation)
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
 * @brief Converts an entity id into the shared string key format.
 * @signature inline EntityKey createEntityKey(const FEntityId &Id)
 *
 * User Story: As component storage, I need a deterministic key for TMap-based
 * component tables.
 */
inline EntityKey createEntityKey(const FEntityId &Id) {
  return LexToString(Id.Index) + FString::Chr(TCHAR(':')) +
         LexToString(Id.Generation);
}

inline bool operator==(const FEntityId &Left, const FEntityId &Right) {
  return Left.Index == Right.Index && Left.Generation == Right.Generation;
}

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
 * @brief Creates an empty entity allocator.
 * @signature inline FAllocator createEntityAllocator()
 *
 * User Story: As a world author, I need isolated allocator state for each world
 * so tests, levels, and runtime sessions do not share entity id counters.
 */
inline FAllocator createEntityAllocator() {
  FAllocator Allocator;
  Allocator.NextIndex = int64{};
  return Allocator;
}

inline bool operator==(const FAllocator &Left, const FAllocator &Right) {
  return Left.NextIndex == Right.NextIndex && Left.Freed == Right.Freed &&
         Left.Generations.OrderIndependentCompareEqual(Right.Generations);
}

inline bool operator!=(const FAllocator &Left, const FAllocator &Right) {
  return !(Left == Right);
}

inline bool operator==(const FAllocatedEntity &Left,
                       const FAllocatedEntity &Right) {
  return Left.Allocator == Right.Allocator && Left.Entity == Right.Entity;
}

inline bool operator!=(const FAllocatedEntity &Left,
                       const FAllocatedEntity &Right) {
  return !(Left == Right);
}

/**
 * @brief Reads the last reusable entity id from the allocator.
 * @signature inline func::Maybe<FEntityId> findReusableEntityId(const FAllocator &Allocator)
 *
 * User Story: As allocation code, I need the reusable-id branch represented as
 * Maybe so fresh allocation composes without imperative checks.
 */
inline func::Maybe<FEntityId> findReusableEntityId(const FAllocator &Allocator) {
  return !Allocator.Freed.IsEmpty() ? func::just(Allocator.Freed.Last())
                                    : func::nothing<FEntityId>();
}

/**
 * @brief Allocates a previously freed entity id.
 * @signature inline FAllocatedEntity allocateRecycledEntity(FAllocator Allocator, const FEntityId &Entity)
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
 * @brief Allocates a fresh entity id and records its generation.
 * @signature inline FAllocatedEntity allocateFreshEntity(FAllocator Allocator)
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
 * @brief Allocates a fresh or recycled entity id.
 * @signature inline FAllocatedEntity allocateEntity(FAllocator Allocator)
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
 * @brief Checks whether an entity id matches its allocator generation.
 * @signature inline bool entityGenerationMatches(const FAllocator &Allocator, const FEntityId &Id)
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
 * @brief Frees an entity id after its generation is known to match.
 * @signature inline FAllocator freeMatchedEntity(FAllocator Allocator, const FEntityId &Id)
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
 * @brief Frees an entity id and increments its generation for safe reuse.
 * @signature inline FAllocator freeEntityId(FAllocator Allocator, const FEntityId &Id)
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
 * @brief Checks whether an entity id is still alive in the allocator.
 * @signature inline bool isEntityAlive(const FAllocator &Allocator, const FEntityId &Id)
 *
 * User Story: As a query system, I need to reject stale handles before reading
 * component data.
 */
inline bool isEntityAlive(const FAllocator &Allocator, const FEntityId &Id) {
  return entityGenerationMatches(Allocator, Id);
}

} // namespace ecs
