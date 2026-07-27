#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Entity/Types.hpp"

namespace rtk {
template <typename T> struct EntityAdapter {
  std::function<FString(const T &)> selectId;

  /**
   * Returns an empty entity-state container.
   * User Story: As entity-backed slices, I need a canonical empty entity state
   * so adapters can initialize predictable reducer storage.
   * @fn EntityState<T> getInitialState() const
   */
  EntityState<T> getInitialState() const { return EntityState<T>{{}, {}}; }

  /**
   * Adds a single entity when its id is not already present.
   * User Story: As entity-backed slices, I need single-entity insertion so new
   * records can be added without mutating existing adapter state.
   * @fn EntityState<T> addOne(const EntityState<T> &state, const T &entity) const
   */
  EntityState<T> addOne(const EntityState<T> &state, const T &entity) const {
    EntityState<T> next = state;
    FString id = selectId(entity);
    detail::addEntityIfMissing(next, id, entity);
    return next;
  }

  /**
   * Adds each missing entity from a batch without replacing existing entries.
   * User Story: As entity-backed slices, I need batch insertion so collections
   * can be seeded while preserving existing records.
   * @fn EntityState<T> addMany(const EntityState<T> &state, const TArray<T> &newEntities) const
   */
  EntityState<T> addMany(const EntityState<T> &state,
                         const TArray<T> &newEntities) const {
    return detail::addManyEntitiesRecursive(*this, newEntities, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, state);
  }

  /**
   * Inserts or replaces a single entity by id.
   * User Story: As entity-backed slices, I need single-entity replacement so
   * reducers can upsert records deterministically.
   * @fn EntityState<T> setOne(const EntityState<T> &state, const T &entity) const
   */
  EntityState<T> setOne(const EntityState<T> &state, const T &entity) const {
    EntityState<T> next = state;
    FString id = selectId(entity);
    detail::setEntity(next, id, entity);
    return next;
  }

  /**
   * Replaces the full entity set with the provided collection.
   * User Story: As entity-backed slices, I need whole-collection replacement so
   * reducers can resync adapter state from remote payloads.
   * @fn EntityState<T> setAll(const EntityState<T> &state, const TArray<T> &newEntities) const
   */
  EntityState<T> setAll(const EntityState<T> &state,
                        const TArray<T> &newEntities) const {
    return detail::setAllEntitiesRecursive(*this, newEntities, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
                                           EntityState<T>());
  }

  /**
   * Upserts a single entity by delegating to setOne.
   * User Story: As entity-backed slices, I need a semantic upsert helper so
   * reducers can express intent without duplicating adapter logic.
   * @fn EntityState<T> upsertOne(const EntityState<T> &state, const T &entity) const
   */
  EntityState<T> upsertOne(const EntityState<T> &state, const T &entity) const {
    return setOne(state, entity);
  }

  /**
   * Upserts a batch of entities by id.
   * User Story: As entity-backed slices, I need batch upsert so synced payloads
   * can merge into adapter state efficiently.
   * @fn EntityState<T> upsertMany(const EntityState<T> &state, const TArray<T> &entitiesToUpsert) const
   */
  EntityState<T> upsertMany(const EntityState<T> &state,
                            const TArray<T> &entitiesToUpsert) const {
    return detail::upsertManyEntitiesRecursive(*this, entitiesToUpsert, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
                                               state);
  }

  /**
   * Removes a single entity and its id when present.
   * User Story: As entity-backed slices, I need record removal so deleted items
   * disappear from both entity maps and id orderings.
   * @fn EntityState<T> removeOne(const EntityState<T> &state, const FString &id) const
   */
  EntityState<T> removeOne(const EntityState<T> &state,
                           const FString &id) const {
    EntityState<T> next = state;
    detail::removeEntityIfPresent(next, id);
    return next;
  }

  /**
   * Removes all entities whose ids appear in the supplied list.
   * User Story: As entity-backed slices, I need batch removal so reducers can
   * clear multiple records in one pure operation.
   * @fn EntityState<T> removeMany(const EntityState<T> &state, const TArray<FString> &removeIds) const
   */
  EntityState<T> removeMany(const EntityState<T> &state,
                            const TArray<FString> &removeIds) const {
    return detail::removeManyEntitiesRecursive(removeIds, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, state);
  }

  /**
   * Clears every entity from the adapter state.
   * User Story: As entity-backed slices, I need a reset helper so adapters can
   * return to a clean initial state predictably.
   * @fn EntityState<T> removeAll(const EntityState<T> &) const
   */
  EntityState<T> removeAll(const EntityState<T> &) const {
    return getInitialState();
  }

  /**
   * Replaces one entity with the result of a patch function.
   * User Story: As entity-backed slices, I need targeted patching so one record
   * can be updated without rebuilding the full collection manually.
   * @fn EntityState<T> updateOne(const EntityState<T> &state, const FString &id, std::function<T(const T &)> patch) const
   */
  EntityState<T> updateOne(const EntityState<T> &state, const FString &id,
                           std::function<T(const T &)> patch) const {
    EntityState<T> next = state;
    detail::updateEntityIfPresent(next, id, patch);
    return next;
  }

  /**
   * Builds selector helpers for the current adapter shape.
   * User Story: As entity-backed slices, I need selector helpers so callers can
   * read ids, entities, and totals without hand-rolled lookup code.
   * @fn EntitySelectors<T> getSelectors() const
   */
  EntitySelectors<T> getSelectors() const {
    const auto SelectAll = [](const EntityState<T> &state) -> TArray<T> {
      return detail::selectAllEntitiesRecursive(state, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, TArray<T>());
    };

    const auto SelectById =
        [](const EntityState<T> &state, const FString &id) -> func::Maybe<T> {
      return detail::findEntityById(state, id);
    };

    const auto SelectIds = [](const EntityState<T> &state) -> TArray<FString> {
      return state.ids;
    };

    const auto SelectTotal = [](const EntityState<T> &state) -> int32_t {
      return state.ids.Num();
    };

    return EntitySelectors<T>{
        SelectAll, SelectById, SelectIds, SelectTotal};
  }
};

namespace detail {
/** User Story: As a core rtk entity consumer, I need to invoke add many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &NewEntities, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                        const TArray<T> &NewEntities,
                                        int32 Index, EntityState<T> Next) {
  return Index >= NewEntities.Num()
             ? Next
             : (addEntityIfMissing(Next, Ops.selectId(NewEntities[Index]),
                                   NewEntities[Index]),
                addManyEntitiesRecursive(Ops, NewEntities, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                         std::move(Next)));
}

/** User Story: As a core rtk entity consumer, I need to invoke set all entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &NewEntities, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops,
                                       const TArray<T> &NewEntities,
                                       int32 Index, EntityState<T> Next) {
  return Index >= NewEntities.Num()
             ? Next
             : (Next.ids.Add(Ops.selectId(NewEntities[Index])),
                Next.entities.Add(Ops.selectId(NewEntities[Index]),
                                  NewEntities[Index]),
                setAllEntitiesRecursive(Ops, NewEntities, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                        std::move(Next)));
}

/** User Story: As a core rtk entity consumer, I need to invoke upsert many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops, const TArray<T> &EntitiesToUpsert, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                           const TArray<T> &EntitiesToUpsert,
                                           int32 Index, EntityState<T> Next) {
  return Index >= EntitiesToUpsert.Num()
             ? Next
             : upsertManyEntitiesRecursive(Ops, EntitiesToUpsert, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                           Ops.setOne(Next,
                                                      EntitiesToUpsert[Index]));
}

/** User Story: As a core rtk entity consumer, I need to invoke remove many entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds, int32 Index, EntityState<T> Next) */
template <typename T>
EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds,
                                           int32 Index, EntityState<T> Next) {
  return Index >= RemoveIds.Num()
             ? Next
             : (removeEntityIfPresent(Next, RemoveIds[Index]),
                removeManyEntitiesRecursive(RemoveIds, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                            std::move(Next)));
}

/** User Story: As a core rtk entity consumer, I need to invoke select all entities recursive through a stable signature so the core rtk entity workflow remains explicit and composable. @fn template <typename T> TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index, TArray<T> Result) */
template <typename T>
TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index,
                                     TArray<T> Result) {
  return Index >= State.ids.Num()
             ? Result
             : (appendEntityIfPresent(Result, State, State.ids[Index]),
                selectAllEntitiesRecursive(State, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                           std::move(Result)));
}
} // namespace detail

/**
 * @fn template <typename T> EntityAdapter<T> createEntityAdapter(std::function<FString(const T &)> selectId)
 * @brief Creates entity-adapter operations from an id selector.
 * @param selectId A function to extract the string ID from an entity.
 * @return EntityAdapter<T> The adapter with CRUD operation helpers.
 *
 * User Story: As slice authors, I need adapter factories so entity state
 * management can be generated from one id-selection rule.
 */
template <typename T>
EntityAdapter<T>
createEntityAdapter(std::function<FString(const T &)> selectId) {
  return EntityAdapter<T>{std::move(selectId)};
}

/**
 * Phase 4: Async Thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

} // namespace rtk
