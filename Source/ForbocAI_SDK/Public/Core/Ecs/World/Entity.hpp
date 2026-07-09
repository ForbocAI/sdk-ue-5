#pragma once

namespace ecs {

inline FSpawnedEntity spawnEntity(FWorld World) {
  const FAllocatedEntity Allocated = allocateEntity(World.Allocator);
  World.Allocator = Allocated.Allocator;
  const EntityKey Entity = createEntityKey(Allocated.Entity);
  FSpawnedEntity Spawned;
  Spawned.World =
      recordEntityDirty(createRecordEntityDirtyRequest(World, Entity));
  Spawned.Id = Allocated.Entity;
  Spawned.Entity = Entity;
  return Spawned;
}

/**
 * @brief Allocates one entity and associates it with an ECS domain.
 * @signature inline FSpawnedEntity spawnEntityInDomain(const FSpawnEntityInDomainRequest &Request)
 *
 * User Story: As a spawn reducer, I need entity creation and domain wiring to
 * return one next world value from a request payload.
 */
inline FSpawnedEntity
spawnEntityInDomain(const FSpawnEntityInDomainRequest &Request) {
  const FSpawnedEntity Spawned = spawnEntity(Request.World);
  FSpawnedEntity WithDomain = Spawned;
  WithDomain.World =
      setEntityDomain({Spawned.World, Spawned.Entity, Request.Domain});
  return WithDomain;
}

/**
 * @brief Builds a transform factory that removes one entity from component tables.
 * @signature inline FComponentTypeWorldTransformFactory removeEntityFromComponentType(const EntityKey &Entity)
 *
 * User Story: As lifecycle reducers, entity component cleanup should be a
 * curried factory that folds through component types without local loops.
 */
inline FComponentTypeWorldTransformFactory
removeEntityFromComponentType(const EntityKey &Entity) {
  return [Entity](const ComponentType &Type) {
    return [Entity, Type](const FWorld &World) {
      FWorld Next = World;
      ComponentTable Table = Next.Components.FindChecked(Type);
      Table.Remove(Entity);
      Next.Components.Add(Type, Table);
      return Next;
    };
  };
}

/**
 * @brief Builds a world transform that removes an entity from every component table.
 * @signature inline FWorldTransform removeEntityComponentIndexes(const EntityKey &Entity)
 *
 * User Story: As lifecycle reducers, component index cleanup should compose as
 * one unary world transform inside the despawn pipeline.
 */
inline FWorldTransform removeEntityComponentIndexes(const EntityKey &Entity) {
  return [Entity](const FWorld &World) {
    const FComponentTypeWorldTransformFactory RemoveFromType =
        removeEntityFromComponentType(Entity);
    return func::fold_array<ComponentType, FWorld>(
        func::map_keys<ComponentType, ComponentTable>(World.Components), World,
        [RemoveFromType](const FWorld &Acc, const ComponentType &Type) {
          return RemoveFromType(Type)(Acc);
        });
  };
}

/**
 * @brief Builds a world transform that removes direct entity-owned indexes.
 * @signature inline FWorldTransform removeEntityDirectIndexes(const EntityKey &Entity)
 *
 * User Story: As lifecycle reducers, direct tag/domain/relationship/dirty
 * cleanup should be one named unary transform.
 */
inline FWorldTransform removeEntityDirectIndexes(const EntityKey &Entity) {
  return [Entity](const FWorld &World) {
    FWorld Next = World;
    Next.Tags.Remove(Entity);
    Next.EntityDomains.Remove(Entity);
    Next.Relationships = removeRelationship(Next.Relationships, Entity);
    Next.DirtyEntities.Remove(Entity);
    return Next;
  };
}

/**
 * @brief Builds a world transform that removes an entity from parent child lists.
 * @signature inline FWorldTransform removeEntityRelationshipChildIndexes(const EntityKey &Entity)
 *
 * User Story: As hierarchy reducers, child-list cleanup should reuse
 * relationship writes so row mutation and dirty propagation stay centralized.
 */
inline FWorldTransform
removeEntityRelationshipChildIndexes(const EntityKey &Entity) {
  return [Entity](const FWorld &World) {
    return applyRelationshipWriteDeclarations(
        World, createRelationshipChildCleanupDeclarations(World, Entity));
  };
}

/**
 * @brief Removes an entity's component, tag, domain, and relationship data.
 * @signature inline FWorld removeEntity(const FRemoveEntityRequest &Request)
 *
 * User Story: As a despawn reducer, I need every entity-owned ECS index cleaned
 * through one world-in/world-out request.
 */
inline FWorld removeEntity(const FRemoveEntityRequest &Request) {
  const FWorld World = applyWorldTransformCatalog(
      Request.World,
      func::catalog(removeEntityComponentIndexes(Request.Entity),
                    removeEntityDirectIndexes(Request.Entity),
                    removeEntityRelationshipChildIndexes(Request.Entity)));
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @brief Frees an id and removes the matching entity data from the world.
 * @signature inline FWorld despawnEntity(const FDespawnEntityRequest &Request)
 *
 * User Story: As a lifecycle reducer, I need allocator and entity-index
 * cleanup to move through one typed request.
 */
inline FWorld despawnEntity(const FDespawnEntityRequest &Request) {
  FWorld World = Request.World;
  World.Allocator = freeEntityId(World.Allocator, Request.Id);
  return removeEntity(createRemoveEntityRequest(World, createEntityKey(Request.Id)));
}

/**
 * @brief Sets a parent relationship using immutable world-in/world-out data.
 * @signature inline FWorld setRelationshipParent(const FSetRelationshipParentRequest &Request)
 *
 * User Story: As relationship ECS code, I need parent/child writes to stay a
 * pure request transform and mark both touched entities.
 */
inline FWorld setRelationshipParent(const FSetRelationshipParentRequest &Request) {
  return applyRelationshipWriteDeclarations(
      Request.World,
      {{Request.Child, ERelationshipWriteKind::AssignParent,
        func::just(Request.Parent)},
       {Request.Parent, ERelationshipWriteKind::AppendChild,
        func::just(Request.Child)}});
}

/**
 * @brief Adds one child to a parent relationship.
 * @signature inline FWorld addRelationshipChild(const FAddRelationshipChildRequest &Request)
 *
 * User Story: As ECS relationship code, adding children should compose through
 * the same parent-setting primitive instead of duplicating write logic.
 */
inline FWorld addRelationshipChild(const FAddRelationshipChildRequest &Request) {
  return setRelationshipParent({Request.World, Request.Child, Request.Parent});
}

/**
 * @brief Removes one child from a parent relationship.
 * @signature inline FWorld removeRelationshipChild(const FRemoveRelationshipChildRequest &Request)
 *
 * User Story: As relationship ECS code, removing children should be a typed
 * unary transition over the world value.
 */
inline FWorld removeRelationshipChild(const FRemoveRelationshipChildRequest &Request) {
  return applyRelationshipWriteDeclarations(
      Request.World,
      {{Request.Parent, ERelationshipWriteKind::RemoveChild,
        func::just(Request.Child)},
       {Request.Child, ERelationshipWriteKind::ClearParent,
        func::nothing<EntityKey>()}});
}

/**
 * @brief Returns all known entities across domains, tags, and components.
 */
inline TArray<EntityKey> collectEntityKeys(const FWorld &World) {
  const TArray<EntityKey> DomainEntities =
      func::map_keys<EntityKey, TArray<DomainPathKey>>(World.EntityDomains);
  const TArray<EntityKey> TaggedEntities =
      func::map_keys<EntityKey, TArray<Tag>>(World.Tags);
  const TArray<ComponentType> ComponentTypes =
      func::map_keys<ComponentType, ComponentTable>(World.Components);
  const TArray<EntityKey> ComponentEntities =
      func::fold_array<ComponentType, TArray<EntityKey>>(
      ComponentTypes, TArray<EntityKey>(),
      [&World](const TArray<EntityKey> &Acc, const ComponentType &Type) {
        return func::match(
            func::find_map_value_ptr<ComponentType, ComponentTable>(World.Components,
                                                           Type),
            [&Acc](const ComponentTable *Table) {
              return func::append_map_keys<EntityKey, FComponentValue>(Acc, *Table);
            },
            [&Acc]() { return Acc; });
      });
  return func::unique_array<EntityKey>(func::concat_arrays<EntityKey>(
      {DomainEntities, TaggedEntities, ComponentEntities}));
}

/**
 * @brief Sets one component value on one entity.
 * @signature inline FWorld setComponent(FWorld World, const EntityKey &Entity, const ComponentType &Type, const FComponentValue &Value)
 *
 * User Story: As a mechanic, I need pure component writes that return the next
 * world value and mark the entity dirty for project-level rendering or syncing.
 */
inline FWorld setComponent(const FSetComponentRequest &Request) {
  FWorld World = Request.World;
  World.Components.FindOrAdd(Request.Type).Add(Request.Entity, Request.Value);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @brief Reads one component value from an entity.
 * @signature inline func::Maybe<FComponentValue> getComponent(const FGetComponentRequest &Request)
 *
 * User Story: As ECS system code, nested component-table lookups should
 * short-circuit through Maybe rather than branching around nullable pointers.
 */
inline func::Maybe<FComponentValue>
getComponent(const FGetComponentRequest &Request) {
  return func::mbind(
      func::find_map_value_ptr<ComponentType, ComponentTable>(
          Request.World.Components, Request.Type),
      [&Request](const ComponentTable *Table) {
        return func::find_map_value<EntityKey, FComponentValue>(*Table,
                                                        Request.Entity);
      });
}

/**
 * @brief Checks whether an entity currently has a component type.
 * @signature inline bool hasComponent(const FHasComponentRequest &Request)
 *
 * User Story: As ECS query code, component membership should reuse the Maybe
 * reader so read semantics stay centralized.
 */
inline bool hasComponent(const FHasComponentRequest &Request) {
  return func::is_just(
      getComponent({Request.World, Request.Entity, Request.Type}));
}

/**
 * @brief Builds a table-present branch for removing one component value.
 * @signature inline std::function<FWorld(const ComponentTable &)> removeComponentFromTable(const FRemoveComponentRequest &Request)
 *
 * User Story: As ECS reducers, component removal should isolate the present
 * table branch so Maybe matching replaces imperative nullable checks.
 */
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

/**
 * @brief Removes one component value from an entity and marks it dirty.
 * @signature inline FWorld removeComponent(const FRemoveComponentRequest &Request)
 *
 * User Story: As ECS reducers, component removal should remain a unary
 * world-in/world-out transform.
 */
inline FWorld removeComponent(const FRemoveComponentRequest &Request) {
  const FWorld World = func::match(
      func::find_map_value<ComponentType, ComponentTable>(Request.World.Components,
                                                 Request.Type),
      removeComponentFromTable(Request),
      [&Request]() { return Request.World; });
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @brief Adds one tag to one entity using a request payload.
 */
inline FWorld setTag(const FSetTagRequest &Request) {
  FWorld World = Request.World;
  World.Tags.FindOrAdd(Request.Entity).AddUnique(Request.TagValue);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @brief Checks whether an entity currently has a tag.

} // namespace ecs
