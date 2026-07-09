#pragma once

namespace ecs {

                                                const EntityKey &Entity) {
  return {World, Entity};
}

/**
 * @brief Builds the dirty entity batch recording request payload.
 * @signature inline FRecordEntitiesDirtyRequest createRecordEntitiesDirtyRequest(FWorld World, const TArray<EntityKey> &Entities)
 *
 * User Story: As relationship reducers, I need repeated dirty marking to use a
 * foldable payload rather than nested calls.
 */
inline FRecordEntitiesDirtyRequest
createRecordEntitiesDirtyRequest(FWorld World,
                                 const TArray<EntityKey> &Entities) {
  return {World, Entities};
}

/**
 * @brief Builds the spawn-in-domain request payload.
 * @signature inline FSpawnEntityInDomainRequest createSpawnEntityInDomainRequest(FWorld World, const DomainPathKey &Domain)
 *
 * User Story: As spawn code, I need a reusable payload factory for domain-bound
 * entity allocation.
 */
inline FSpawnEntityInDomainRequest
createSpawnEntityInDomainRequest(FWorld World, const DomainPathKey &Domain) {
  return {World, Domain};
}

/**
 * @brief Builds the entity-removal request payload.
 * @signature inline FRemoveEntityRequest createRemoveEntityRequest(FWorld World, const EntityKey &Entity)
 *
 * User Story: As lifecycle code, I need entity cleanup payloads constructed
 * consistently from one factory.
 */
inline FRemoveEntityRequest createRemoveEntityRequest(FWorld World,
                                                      const EntityKey &Entity) {
  return {World, Entity};
}

/**
 * @brief Builds the despawn request payload.
 * @signature inline FDespawnEntityRequest createDespawnEntityRequest(FWorld World, const FEntityId &Id)
 *
 * User Story: As lifecycle code, I need allocator cleanup payloads constructed
 * consistently from one factory.
 */
inline FDespawnEntityRequest createDespawnEntityRequest(FWorld World,
                                                        const FEntityId &Id) {
  return {World, Id};
}

/**
 * @brief Builds the resource-read request payload.
 * @signature inline FGetResourceRequest createGetResourceRequest(const FWorld &World, const ResourceName &Name)
 *
 * User Story: As ECS readers, I need resource lookup payloads created through
 * one factory before Maybe composition.
 */
inline FGetResourceRequest createGetResourceRequest(const FWorld &World,
                                                    const ResourceName &Name) {
  return {World, Name};
}

/**
 * @brief Builds the event-read request payload.
 * @signature inline FReadEventsRequest createReadEventsRequest(const FWorld &World, const EventType &Type)
 *
 * User Story: As ECS systems, I need event reads created through one factory
 * before matching optional queues.
 */
inline FReadEventsRequest createReadEventsRequest(const FWorld &World,
                                                  const EventType &Type) {
  return {World, Type};
}

/**
 * @brief Builds the all-entities component-query request payload.
 * @signature inline FQueryEntitiesByComponentsRequest createQueryEntitiesByComponentsRequest(const FWorld &World, const TArray<ComponentType> &Types)
 *
 * User Story: As selectors, I need all-entity component queries to share one
 * request factory.
 */
inline FQueryEntitiesByComponentsRequest
createQueryEntitiesByComponentsRequest(const FWorld &World,
                                       const TArray<ComponentType> &Types) {
  return {World, Types};
}

/**
 * @brief Builds the tag-query request payload.
 * @signature inline FQueryEntitiesByTagRequest createQueryEntitiesByTagRequest(const FWorld &World, const Tag &TagValue)
 *
 * User Story: As selectors, I need tag queries to share one request factory.
 */
inline FQueryEntitiesByTagRequest
createQueryEntitiesByTagRequest(const FWorld &World, const Tag &TagValue) {
  return {World, TagValue};
}

/**
 * @brief Builds the domain-query request payload.
 * @signature inline FQueryEntitiesByDomainRequest createQueryEntitiesByDomainRequest(const FWorld &World, const DomainPathKey &Domain)
 *
 * User Story: As selectors, I need domain queries to share one request factory.
 */
inline FQueryEntitiesByDomainRequest
createQueryEntitiesByDomainRequest(const FWorld &World,
                                   const DomainPathKey &Domain) {
  return {World, Domain};
}

/**
 * @brief Builds the child-query request payload.
 * @signature inline FQueryChildrenRequest createQueryChildrenRequest(const FWorld &World, const EntityKey &Parent)
 *
 * User Story: As hierarchy selectors, I need child queries to share one
 * payload factory.
 */
inline FQueryChildrenRequest createQueryChildrenRequest(const FWorld &World,
                                                        const EntityKey &Parent) {
  return {World, Parent};
}

inline FWorld setEntityDomain(const FSetEntityDomainRequest &Request);

/**
 * @brief Creates a fresh ECS world with allocator and cross-domain registry.
 *
 * Architecture: The world is plain value state. RTK reducers may store and
 * replace it, but ECS never owns the runtime store or dispatch semantics.
 */
inline FWorld createWorld(const FGraph &Domains) {
  FWorld World;
  World.Allocator = createEntityAllocator();
  World.Domains = Domains;
  World.Generation = int64{};
  return World;
}

inline FWorld createWorld() { return createWorld(createDomainRegistry()); }

inline bool componentTableEquals(const ComponentTable &Left,
                                 const ComponentTable &Right) {
  return func::map_values_equal<EntityKey, FComponentValue>(
      Left, Right,
      [](const FComponentValue &LeftValue,
         const FComponentValue &RightValue) { return LeftValue == RightValue; });
}

inline bool componentStoreEquals(const ComponentStore &Left,
                                 const ComponentStore &Right) {
  return func::map_values_equal<ComponentType, ComponentTable>(
      Left, Right,
      [](const ComponentTable &LeftValue, const ComponentTable &RightValue) {
        return componentTableEquals(LeftValue, RightValue);
      });
}

inline bool tagStoreEquals(const TagStore &Left, const TagStore &Right) {
  return func::map_values_equal<EntityKey, TArray<Tag>>(
      Left, Right,
      [](const TArray<Tag> &LeftValue, const TArray<Tag> &RightValue) {
        return LeftValue == RightValue;
      });
}

inline bool resourceStoreEquals(const ResourceStore &Left,
                                const ResourceStore &Right) {
  return func::map_values_equal<ResourceName, FComponentValue>(
      Left, Right,
      [](const FComponentValue &LeftValue,
         const FComponentValue &RightValue) { return LeftValue == RightValue; });
}

inline bool relationshipStoreEquals(
    const TMap<EntityKey, FRelationship> &Left,
    const TMap<EntityKey, FRelationship> &Right) {
  return func::map_values_equal<EntityKey, FRelationship>(
      Left, Right,
      [](const FRelationship &LeftValue, const FRelationship &RightValue) {
        return LeftValue == RightValue;
      });
}

inline bool eventQueueEquals(const EventQueue &Left, const EventQueue &Right) {
  return func::map_values_equal<EventType, TArray<FComponentValue>>(
      Left, Right,
      [](const TArray<FComponentValue> &LeftValue,
         const TArray<FComponentValue> &RightValue) {
        return LeftValue == RightValue;
      });
}

inline bool entityDomainStoreEquals(
    const TMap<EntityKey, TArray<DomainPathKey>> &Left,
    const TMap<EntityKey, TArray<DomainPathKey>> &Right) {
  return func::map_values_equal<EntityKey, TArray<DomainPathKey>>(
      Left, Right,
      [](const TArray<DomainPathKey> &LeftValue,
         const TArray<DomainPathKey> &RightValue) {
        return LeftValue == RightValue;
      });
}

inline bool operator==(const FWorld &Left, const FWorld &Right) {
  return Left.Allocator == Right.Allocator &&
         componentStoreEquals(Left.Components, Right.Components) &&
         tagStoreEquals(Left.Tags, Right.Tags) &&
         resourceStoreEquals(Left.Resources, Right.Resources) &&
         eventQueueEquals(Left.Events, Right.Events) &&
         entityDomainStoreEquals(Left.EntityDomains, Right.EntityDomains) &&
         relationshipStoreEquals(Left.Relationships, Right.Relationships) &&
         Left.Domains == Right.Domains &&
         Left.DirtyEntities == Right.DirtyEntities &&
         Left.Generation == Right.Generation;
}

inline bool operator!=(const FWorld &Left, const FWorld &Right) {
  return !(Left == Right);
}

/**
 * @brief Records one dirty entity and increments the world generation.
 * @signature inline FWorld recordEntityDirty(const FRecordEntityDirtyRequest &Request)
 *
 * User Story: As a reducer-owned ECS transition, dirty tracking should be a
 * unary world transform that can be composed without hidden side effects.
 */
inline FWorld recordEntityDirty(const FRecordEntityDirtyRequest &Request) {
  FWorld World = Request.World;
  World.DirtyEntities.AddUnique(Request.Entity);
  ++World.Generation;
  return World;
}

/**
 * @brief Records multiple dirty entities through a world update fold.
 * @signature inline FWorld recordEntitiesDirty(const FRecordEntitiesDirtyRequest &Request)
 *
 * User Story: As relationship reducers, I need multi-entity dirty updates to
 * compose from the single-entity reducer rather than repeating nested calls.
 */
inline FWorld recordEntitiesDirty(const FRecordEntitiesDirtyRequest &Request) {
  return func::fold_array<EntityKey, FWorld>(
      Request.Entities, Request.World,
      [](const FWorld &World, const EntityKey &Entity) {
        return recordEntityDirty(createRecordEntityDirtyRequest(World, Entity));
      });
}

} // namespace ecs
