#pragma once

#include "Core/RTK/Prelude.hpp"

namespace rtk {
template <typename T> struct EntityState {
  TArray<FString> ids;
  TMap<FString, T> entities;
};

namespace detail {
template <typename T>
bool entityStateValueEquals(const TMap<FString, T> &LeftEntities,
                            const TMap<FString, T> &RightEntities,
                            const FString &Id) {
  const T *LeftEntity = LeftEntities.Find(Id);
  const T *RightEntity = RightEntities.Find(Id);
  return LeftEntity && RightEntity && (*LeftEntity == *RightEntity);
}

template <typename T>
bool entityStateEqualsRecursive(const EntityState<T> &Left,
                                const EntityState<T> &Right, int32 Index) {
  return Index >= Left.ids.Num()
             ? true
             : Left.ids[Index] == Right.ids[Index] &&
                   entityStateValueEquals(Left.entities, Right.entities,
                                          Left.ids[Index]) &&
                   entityStateEqualsRecursive(Left, Right, Index + 1);
}
} // namespace detail

/**
 * @brief Checks if two EntityState objects are deeply equal.
 * @signature template <typename T> bool operator==(const EntityState<T> &Left, const EntityState<T> &Right)
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
         detail::entityStateEqualsRecursive(Left, Right, 0);
}

/**
 * @brief Checks if two EntityState objects are not equal.
 * @signature template <typename T> bool operator!=(const EntityState<T> &Left, const EntityState<T> &Right)
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
template <typename T>
void addEntityIfMissing(EntityState<T> &Next, const FString &Id,
                        const T &Entity) {
  const bool bMissing = !Next.entities.Find(Id);
  bMissing && (Next.ids.Add(Id), true);
  bMissing && (Next.entities.Add(Id, Entity), true);
}

template <typename T>
void setEntity(EntityState<T> &Next, const FString &Id, const T &Entity) {
  (!Next.entities.Find(Id)) && (Next.ids.Add(Id), true);
  Next.entities.Add(Id, Entity);
}

template <typename T>
void removeEntityIfPresent(EntityState<T> &Next, const FString &Id) {
  (Next.entities.Remove(Id) > 0) && (Next.ids.Remove(Id), true);
}

template <typename T, typename PatchFn>
void updateEntityIfPresent(EntityState<T> &Next, const FString &Id,
                           PatchFn Patch) {
  const T *Existing = Next.entities.Find(Id);
  Existing && (Next.entities.Add(Id, Patch(*Existing)), true);
}

template <typename T>
void appendEntityIfPresent(TArray<T> &Result, const EntityState<T> &State,
                           const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  Entity && (Result.Add(*Entity), true);
}

template <typename T>
func::Maybe<T> findEntityById(const EntityState<T> &State, const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  return Entity ? func::just(*Entity) : func::nothing<T>();
}

template <typename T>
EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                        const TArray<T> &NewEntities,
                                        int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops,
                                       const TArray<T> &NewEntities,
                                       int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                           const TArray<T> &EntitiesToUpsert,
                                           int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds,
                                           int32 Index, EntityState<T> Next);

template <typename T>
TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index,
                                     TArray<T> Result);
} // namespace detail

} // namespace rtk
