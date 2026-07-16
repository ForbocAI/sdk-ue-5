#pragma once

namespace ecs {

/** User Story: As a core ecs world consumer, I need to invoke spawn entity through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FSpawnedEntity spawnEntity(FWorld World) */
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

/** User Story: As a core ecs world consumer, I need to invoke spawn entity in domain through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FSpawnedEntity spawnEntityInDomain(const FSpawnEntityInDomainRequest &Request) */
inline FSpawnedEntity
spawnEntityInDomain(const FSpawnEntityInDomainRequest &Request) {
  const FSpawnedEntity Spawned = spawnEntity(Request.World);
  FSpawnedEntity WithDomain = Spawned;
  WithDomain.World =
      setEntityDomain({Spawned.World, Spawned.Entity, Request.Domain});
  return WithDomain;
}

/** User Story: As a core ecs world consumer, I need to invoke remove entity from component type through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FComponentTypeWorldTransformFactory removeEntityFromComponentType(const EntityKey &Entity) */
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

/** User Story: As a core ecs world consumer, I need to invoke remove entity component indexes through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorldTransform removeEntityComponentIndexes(const EntityKey &Entity) */
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

/** User Story: As a core ecs world consumer, I need to invoke remove entity direct indexes through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorldTransform removeEntityDirectIndexes(const EntityKey &Entity) */
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

/** User Story: As a core ecs world consumer, I need to invoke remove entity relationship child indexes through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorldTransform removeEntityRelationshipChildIndexes(const EntityKey &Entity) */
inline FWorldTransform
removeEntityRelationshipChildIndexes(const EntityKey &Entity) {
  return [Entity](const FWorld &World) {
    return applyRelationshipWriteDeclarations(
        World, createRelationshipChildCleanupDeclarations(World, Entity));
  };
}

/** User Story: As a core ecs world consumer, I need to invoke remove entity through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld removeEntity(const FRemoveEntityRequest &Request) */
inline FWorld removeEntity(const FRemoveEntityRequest &Request) {
  const FWorld World = applyWorldTransformCatalog(
      Request.World,
      func::catalog(removeEntityComponentIndexes(Request.Entity),
                    removeEntityDirectIndexes(Request.Entity),
                    removeEntityRelationshipChildIndexes(Request.Entity)));
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/** User Story: As a core ecs world consumer, I need to invoke despawn entity through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld despawnEntity(const FDespawnEntityRequest &Request) */
inline FWorld despawnEntity(const FDespawnEntityRequest &Request) {
  FWorld World = Request.World;
  World.Allocator = freeEntityId(World.Allocator, Request.Id);
  return removeEntity(
      createRemoveEntityRequest(World, createEntityKey(Request.Id)));
}

/** User Story: As a core ecs world consumer, I need to invoke set relationship parent through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld setRelationshipParent(const FSetRelationshipParentRequest &Request) */
inline FWorld
setRelationshipParent(const FSetRelationshipParentRequest &Request) {
  return applyRelationshipWriteDeclarations(
      Request.World,
      {{Request.Child, ERelationshipWriteKind::AssignParent,
        func::just(Request.Parent)},
       {Request.Parent, ERelationshipWriteKind::AppendChild,
        func::just(Request.Child)}});
}

/** User Story: As a core ecs world consumer, I need to invoke add relationship child through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld addRelationshipChild(const FAddRelationshipChildRequest &Request) */
inline FWorld
addRelationshipChild(const FAddRelationshipChildRequest &Request) {
  return setRelationshipParent({Request.World, Request.Child, Request.Parent});
}

/** User Story: As a core ecs world consumer, I need to invoke remove relationship child through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld removeRelationshipChild(const FRemoveRelationshipChildRequest &Request) */
inline FWorld
removeRelationshipChild(const FRemoveRelationshipChildRequest &Request) {
  return applyRelationshipWriteDeclarations(
      Request.World,
      {{Request.Parent, ERelationshipWriteKind::RemoveChild,
        func::just(Request.Child)},
       {Request.Child, ERelationshipWriteKind::ClearParent,
        func::nothing<EntityKey>()}});
}

/** User Story: As a core ecs world consumer, I need to invoke collect entity keys through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline TArray<EntityKey> collectEntityKeys(const FWorld &World) */
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
                func::find_map_value_ptr<ComponentType, ComponentTable>(
                    World.Components, Type),
                [&Acc](const ComponentTable *Table) {
                  return func::append_map_keys<EntityKey, FComponentValue>(
                      Acc, *Table);
                },
                [&Acc]() { return Acc; });
          });
  return func::unique_array<EntityKey>(func::concat_arrays<EntityKey>(
      {DomainEntities, TaggedEntities, ComponentEntities}));
}

} // namespace ecs
