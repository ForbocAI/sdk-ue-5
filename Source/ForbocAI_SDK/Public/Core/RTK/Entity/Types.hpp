#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Prelude.hpp"

namespace rtk {
template <typename T> struct EntityState {
  TArray<FString> ids;
  TMap<FString, T> entities;
};

namespace detail {
/** User Story: As a core rtk entity consumer, I need to invoke entity state value equals through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> bool entityStateValueEquals(const TMap<FString, T> &LeftEntities, const TMap<FString, T> &RightEntities, const FString &Id) */
template <typename T>
bool entityStateValueEquals(const TMap<FString, T> &LeftEntities,
                            const TMap<FString, T> &RightEntities,
                            const FString &Id) {
  const T *LeftEntity = LeftEntities.Find(Id);
  const T *RightEntity = RightEntities.Find(Id);
  return LeftEntity && RightEntity && (*LeftEntity == *RightEntity);
}

/** User Story: As a core rtk entity consumer, I need to invoke entity state equals recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> bool entityStateEqualsRecursive(const EntityState<T> &Left, const EntityState<T> &Right, int32 Index) */
template <typename T>
bool entityStateEqualsRecursive(const EntityState<T> &Left,
                                const EntityState<T> &Right, int32 Index) {
  return Index >= Left.ids.Num()
             ? true
             : Left.ids[Index] == Right.ids[Index] &&
                   entityStateValueEquals(Left.entities, Right.entities,
                                          Left.ids[Index]) &&
                   entityStateEqualsRecursive(Left, Right, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
}
} // namespace detail

/**
 * @fn template <typename T> bool operator==(const EntityState<T> &Left, const EntityState<T> &Right)
 * @brief Checks if two EntityState objects are deeply equal.
 * @param Left The first state to compare.
 * @param Right The second state to compare.
 * @return true if both states have the same entities and ordering; false otherwise.
 *
 * User Story: As a functional reducer, I need deep equality checks to avoid unnecessary state updates when payload data matches existing state.
 */
template <typename T>
bool operator==(const EntityState<T> &Left, const EntityState<T> &Right) {
  return Left.ids.Num() == Right.ids.Num() &&
         Left.entities.Num() == Right.entities.Num() &&
         detail::entityStateEqualsRecursive(Left, Right, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
}

/**
 * @fn template <typename T> bool operator!=(const EntityState<T> &Left, const EntityState<T> &Right)
 * @brief Checks if two EntityState objects are not equal.
 * @param Left The first state to compare.
 * @param Right The second state to compare.
 * @return true if the states differ; false otherwise.
 *
 * User Story: As a functional reducer, I need inequality checks to trigger state changes when updating collections.
 */
template <typename T>
bool operator!=(const EntityState<T> &Left, const EntityState<T> &Right) {
  return !(Left == Right);
}

template <typename T> struct EntitySelectors {
  std::function<TArray<T>(const EntityState<T> &)> selectAll;
  std::function<func::Maybe<T>(const EntityState<T> &, const FString &)>
      selectById;
  std::function<TArray<FString>(const EntityState<T> &)> selectIds;
  std::function<int32_t(const EntityState<T> &)> selectTotal;
};

template <typename T> struct EntityAdapter;

namespace detail {
/** User Story: As a core rtk entity consumer, I need to invoke add entity if missing through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> void addEntityIfMissing(EntityState<T> &Next, const FString &Id, const T &Entity) */
template <typename T>
void addEntityIfMissing(EntityState<T> &Next, const FString &Id,
                        const T &Entity) {
  const bool bMissing = !Next.entities.Find(Id);
  bMissing && (Next.ids.Add(Id), true);
  bMissing && (Next.entities.Add(Id, Entity), true);
}

/** User Story: As a core rtk entity consumer, I need to invoke set entity through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> void setEntity(EntityState<T> &Next, const FString &Id, const T &Entity) */
template <typename T>
void setEntity(EntityState<T> &Next, const FString &Id, const T &Entity) {
  (!Next.entities.Find(Id)) && (Next.ids.Add(Id), true);
  Next.entities.Add(Id, Entity);
}

/** User Story: As a core rtk entity consumer, I need to invoke remove entity if present through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> void removeEntityIfPresent(EntityState<T> &Next, const FString &Id) */
template <typename T>
void removeEntityIfPresent(EntityState<T> &Next, const FString &Id) {
  (Next.entities.Remove(Id) > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) && (Next.ids.Remove(Id), true);
}

/** User Story: As a core rtk entity consumer, I need to invoke update entity if present through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T, typename PatchFn> void updateEntityIfPresent(EntityState<T> &Next, const FString &Id, PatchFn Patch) */
template <typename T, typename PatchFn>
void updateEntityIfPresent(EntityState<T> &Next, const FString &Id,
                           PatchFn Patch) {
  const T *Existing = Next.entities.Find(Id);
  Existing && (Next.entities.Add(Id, Patch(*Existing)), true);
}

/** User Story: As a core rtk entity consumer, I need to invoke append entity if present through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> void appendEntityIfPresent(TArray<T> &Result, const EntityState<T> &State, const FString &Id) */
template <typename T>
void appendEntityIfPresent(TArray<T> &Result, const EntityState<T> &State,
                           const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  Entity && (Result.Add(*Entity), true);
}

/** User Story: As a core rtk entity consumer, I need to invoke find entity by id through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> func::Maybe<T> findEntityById(const EntityState<T> &State, const FString &Id) */
template <typename T>
func::Maybe<T> findEntityById(const EntityState<T> &State, const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  return Entity ? func::just(*Entity) : func::nothing<T>();
}

/** User Story: As a core rtk entity consumer, I need to invoke add many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &NewEntities, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                        const TArray<T> &NewEntities,
                                        int32 Index, EntityState<T> Next);

/** User Story: As a core rtk entity consumer, I need to invoke set all entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &NewEntities, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops,
                                       const TArray<T> &NewEntities,
                                       int32 Index, EntityState<T> Next);

/** User Story: As a core rtk entity consumer, I need to invoke upsert many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &EntitiesToUpsert, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                           const TArray<T> &EntitiesToUpsert,
                                           int32 Index, EntityState<T> Next);

/** User Story: As a core rtk entity consumer, I need to invoke remove many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds,
                                           int32 Index, EntityState<T> Next);

/** User Story: As a core rtk entity consumer, I need to invoke select all entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index, TArray<T> Result) */
template <typename T>
TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index,
                                     TArray<T> Result);
} // namespace detail

} // namespace rtk
