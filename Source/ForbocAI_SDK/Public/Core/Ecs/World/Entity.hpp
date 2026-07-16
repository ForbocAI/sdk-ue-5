#pragma once

namespace ecs {

/** User Story: As a core ecs world consumer, I need to invoke set component through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld setComponent(const FSetComponentRequest &Request) */
inline FWorld setComponent(const FSetComponentRequest &Request) {
  FWorld World = Request.World;
  World.Components.FindOrAdd(Request.Type).Add(Request.Entity, Request.Value);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/** User Story: As a core ecs world consumer, I need to invoke get component through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline func::Maybe<FComponentValue> getComponent(const FGetComponentRequest &Request) */
inline func::Maybe<FComponentValue>
getComponent(const FGetComponentRequest &Request) {
  return func::mbind(
      func::find_map_value_ptr<ComponentType, ComponentTable>(
          Request.World.Components, Request.Type),
      [&Request](const ComponentTable *Table) {
        return func::find_map_value<EntityKey, FComponentValue>(
            *Table, Request.Entity);
      });
}

/** User Story: As a core ecs world consumer, I need to invoke has component through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline bool hasComponent(const FHasComponentRequest &Request) */
inline bool hasComponent(const FHasComponentRequest &Request) {
  return func::is_just(
      getComponent({Request.World, Request.Entity, Request.Type}));
}

/** User Story: As a core ecs world consumer, I need to invoke remove component from table through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline std::function<FWorld(const ComponentTable &)> removeComponentFromTable(const FRemoveComponentRequest &Request) */
inline std::function<FWorld(const ComponentTable &)>
removeComponentFromTable(const FRemoveComponentRequest &Request) {
  return [&Request](const ComponentTable &Table) {
    FWorld Next = Request.World;
    ComponentTable UpdatedTable = Table;
    UpdatedTable.Remove(Request.Entity);
    Next.Components.Add(Request.Type, UpdatedTable);
    return Next;
  };
}

/** User Story: As a core ecs world consumer, I need to invoke remove component through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld removeComponent(const FRemoveComponentRequest &Request) */
inline FWorld removeComponent(const FRemoveComponentRequest &Request) {
  const FWorld World = func::match(
      func::find_map_value<ComponentType, ComponentTable>(
          Request.World.Components, Request.Type),
      removeComponentFromTable(Request),
      [&Request]() { return Request.World; });
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/** User Story: As a core ecs world consumer, I need to invoke set tag through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld setTag(const FSetTagRequest &Request) */
inline FWorld setTag(const FSetTagRequest &Request) {
  FWorld World = Request.World;
  World.Tags.FindOrAdd(Request.Entity).AddUnique(Request.TagValue);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/** User Story: As a core ecs world consumer, I need to invoke has tag through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline bool hasTag(const FHasTagRequest &Request) */
inline bool hasTag(const FHasTagRequest &Request) {
  return func::map_array_contains<EntityKey, Tag>(
      Request.World.Tags, Request.Entity, Request.TagValue);
}

} // namespace ecs
