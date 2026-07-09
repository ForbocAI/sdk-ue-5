#pragma once

namespace ecs {

 * @signature inline bool hasTag(const FHasTagRequest &Request)
 *
 * User Story: As ECS query code, tag membership should use Maybe lookup plus
 * reusable array predicates.
 */
inline bool hasTag(const FHasTagRequest &Request) {
  return func::map_array_contains<EntityKey, Tag>(Request.World.Tags, Request.Entity,
                                          Request.TagValue);
}

/**
 * @brief Sets one world resource using a request payload.
 */
inline FWorld setResource(const FSetResourceRequest &Request) {
  FWorld World = Request.World;
  World.Resources.Add(Request.Name, Request.Value);
  ++World.Generation;
  return World;
}

/**
 * @brief Reads one resource value from the world.
 * @signature inline func::Maybe<FComponentValue> getResource(const FGetResourceRequest &Request)
 *
 * User Story: As ECS side-effect orchestration code, resource reads should be
 * optional values instead of nullable pointer branches.
 */
inline func::Maybe<FComponentValue>
getResource(const FGetResourceRequest &Request) {
  return func::find_map_value<ResourceName, FComponentValue>(Request.World.Resources,
                                                    Request.Name);
}

/**
 * @brief Appends one ECS event payload to its event queue.
 */
inline FWorld pushEvent(const FPushEventRequest &Request) {
  FWorld World = Request.World;
  World.Events.FindOrAdd(Request.Type).Add(Request.Payload);
  return World;
}

/**
 * @brief Reads queued event payloads for one event type.
 * @signature inline TArray<FComponentValue> readEvents(const FReadEventsRequest &Request)
 *
 * User Story: As ECS systems code, event queues should materialize through a
 * Maybe match at the boundary.
 */
inline TArray<FComponentValue> readEvents(const FReadEventsRequest &Request) {
  return func::or_else(
      func::find_map_value<EventType, TArray<FComponentValue>>(Request.World.Events,
                                                       Request.Type),
      TArray<FComponentValue>());
}

/**
 * @brief Clears all queued ECS events.
 */
inline FWorld clearEvents(FWorld World) {
  World.Events.Empty();
  return World;
}

/**
 * @brief Associates one entity with one ECS domain via request payload.
 */
inline FWorld setEntityDomain(const FSetEntityDomainRequest &Request) {
  FWorld World = Request.World;
  World.EntityDomains.FindOrAdd(Request.Entity).AddUnique(Request.Domain);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @brief Checks whether an entity belongs to a domain path.
 * @signature inline bool isEntityInDomain(const FEntityInDomainRequest &Request)
 *
 * User Story: As ECS domain queries, membership should be a Maybe lookup plus
 * an array predicate instead of borrowing sibling-domain logic.
 */
inline bool isEntityInDomain(const FEntityInDomainRequest &Request) {
  return func::map_array_contains<EntityKey, DomainPathKey>(
      Request.World.EntityDomains, Request.Entity, Request.Domain);
}

/**
 * @brief Filters entities that contain every requested component type.
 * @signature inline TArray<EntityKey> queryComponents(const FQueryComponentsRequest &Request)
 *
 * User Story: As ECS system execution, component filtering should compose from
 * reusable predicates and request-shaped readers.
 */
inline TArray<EntityKey> queryComponents(const FQueryComponentsRequest &Request) {
  return func::filter_array<EntityKey>(
      Request.Entities,
      [&Request](const EntityKey &Entity) {
        return func::all_array<ComponentType>(
            Request.Types,
            [&Request, &Entity](const ComponentType &Type) {
              return hasComponent({Request.World, Entity, Type});
            });
      });
}

/**
 * @brief Queries all world entities by required components.
 * @signature inline TArray<EntityKey> queryEntitiesByComponents(const FQueryEntitiesByComponentsRequest &Request)
 *
 * User Story: As selectors or reducers query ECS state, component queries
 * should pass through one payload and remain view-neutral.
 */
inline TArray<EntityKey>
queryEntitiesByComponents(const FQueryEntitiesByComponentsRequest &Request) {
  return queryComponents(
      {Request.World, Request.Types, collectEntityKeys(Request.World)});
}

/**
 * @brief Queries all world entities by tag.
 * @signature inline TArray<EntityKey> queryEntitiesByTag(const FQueryEntitiesByTagRequest &Request)
 *
 * User Story: As selectors derive entity lists, tag filtering should reuse the
 * same functional query primitives as systems.
 */
inline TArray<EntityKey>
queryEntitiesByTag(const FQueryEntitiesByTagRequest &Request) {
  const TArray<EntityKey> Entities = collectEntityKeys(Request.World);
  return func::filter_array<EntityKey>(
      Entities,
      [&Request](const EntityKey &Entity) {
        return hasTag({Request.World, Entity, Request.TagValue});
      });
}

/**
 * @brief Queries all world entities by domain path.
 * @signature inline TArray<EntityKey> queryEntitiesByDomain(const FQueryEntitiesByDomainRequest &Request)
 *
 * User Story: As feature domains query ECS ownership, domain filtering should
 * import downward into neutral ECS predicates.
 */
inline TArray<EntityKey>
queryEntitiesByDomain(const FQueryEntitiesByDomainRequest &Request) {
  const TArray<EntityKey> Entities = collectEntityKeys(Request.World);
  return func::filter_array<EntityKey>(
      Entities,
      [&Request](const EntityKey &Entity) {
        return isEntityInDomain({Request.World, Entity, Request.Domain});
      });
}

/**
 * @brief Returns entities dirtied by recent world transforms.
 */
inline TArray<EntityKey> queryDirtyWorldEntities(const FWorld &World) {
  return World.DirtyEntities;
}

/**
 * @brief Returns child entity keys for a parent.
 * @signature inline TArray<EntityKey> queryChildren(const FQueryChildrenRequest &Request)
 *
 * User Story: As hierarchy selectors, child reads should materialize through
 * Maybe instead of nullable pointer branches.
 */
inline TArray<EntityKey> queryChildren(const FQueryChildrenRequest &Request) {
  return getRelationshipOrDefault(createGetRelationshipRequest(
             Request.World.Relationships, Request.Parent))
      .Children;
}

/**
 * @brief Gathers available component values for a system input entity.
 * @signature inline TMap<ComponentType, FComponentValue> gatherComponents(const FGatherComponentsRequest &Request)
 *
 * User Story: As ECS system execution, component gathering should fold over
 * requested types and let Maybe decide whether each component is present.
 */
inline TMap<ComponentType, FComponentValue>
gatherComponents(const FGatherComponentsRequest &Request) {
  return func::fold_array<ComponentType,
                          TMap<ComponentType, FComponentValue>>(
      Request.Types, TMap<ComponentType, FComponentValue>(),
      [&Request](const TMap<ComponentType, FComponentValue> &Acc,
                 const ComponentType &Type) {
        return func::match(
            getComponent({Request.World, Request.Entity, Type}),
            [&Acc, &Type](const FComponentValue &Value) {
              TMap<ComponentType, FComponentValue> Next = Acc;
              Next.Add(Type, Value);
              return Next;
            },
            [&Acc]() { return Acc; });
      });
}

struct FSystemExecutionPayload {
  FWorld World;
  EntityKey Entity;

} // namespace ecs
