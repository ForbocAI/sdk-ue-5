#pragma once

namespace ecs {

/**
 * @fn inline FWorld setResource(const FSetResourceRequest &Request)
 * @brief Sets one world resource using a request payload.
 * User Story: As a core ecs world consumer, I need to invoke set resource through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld setResource(const FSetResourceRequest &Request) {
  FWorld World = Request.World;
  World.Resources.Add(Request.Name, Request.Value);
  ++World.Generation;
  return World;
}

/**
 * @fn inline func::Maybe<FComponentValue> getResource(const FGetResourceRequest &Request)
 * @brief Reads one resource value from the world.
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
 * @fn inline FWorld pushEvent(const FPushEventRequest &Request)
 * @brief Appends one ECS event payload to its event queue.
 * User Story: As a core ecs world consumer, I need to invoke push event through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld pushEvent(const FPushEventRequest &Request) {
  FWorld World = Request.World;
  World.Events.FindOrAdd(Request.Type).Add(Request.Payload);
  return World;
}

/**
 * @fn inline TArray<FComponentValue> readEvents(const FReadEventsRequest &Request)
 * @brief Reads queued event payloads for one event type.
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
 * @fn inline FWorld clearEvents(FWorld World)
 * @brief Clears all queued ECS events.
 * User Story: As a core ecs world consumer, I need to invoke clear events through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld clearEvents(FWorld World) {
  World.Events.Empty();
  return World;
}

/**
 * @fn inline FWorld setEntityDomain(const FSetEntityDomainRequest &Request)
 * @brief Associates one entity with one ECS domain via request payload.
 * User Story: As a core ecs world consumer, I need to invoke set entity domain through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld setEntityDomain(const FSetEntityDomainRequest &Request) {
  FWorld World = Request.World;
  World.EntityDomains.FindOrAdd(Request.Entity).AddUnique(Request.Domain);
  return recordEntityDirty(
      createRecordEntityDirtyRequest(World, Request.Entity));
}

/**
 * @fn inline bool isEntityInDomain(const FEntityInDomainRequest &Request)
 * @brief Checks whether an entity belongs to a domain path.
 *
 * User Story: As ECS domain queries, membership should be a Maybe lookup plus
 * an array predicate instead of borrowing sibling-domain logic.
 */
inline bool isEntityInDomain(const FEntityInDomainRequest &Request) {
  return func::map_array_contains<EntityKey, DomainPathKey>(
      Request.World.EntityDomains, Request.Entity, Request.Domain);
}

/**
 * @fn inline TArray<EntityKey> queryComponents(const FQueryComponentsRequest &Request)
 * @brief Filters entities that contain every requested component type.
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
 * @fn inline TArray<EntityKey> queryEntitiesByComponents(const FQueryEntitiesByComponentsRequest &Request)
 * @brief Queries all world entities by required components.
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
 * @fn inline TArray<EntityKey> queryEntitiesByTag(const FQueryEntitiesByTagRequest &Request)
 * @brief Queries all world entities by tag.
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
 * @fn inline TArray<EntityKey> queryEntitiesByDomain(const FQueryEntitiesByDomainRequest &Request)
 * @brief Queries all world entities by domain path.
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
 * @fn inline TArray<EntityKey> queryDirtyWorldEntities(const FWorld &World)
 * @brief Returns entities dirtied by recent world transforms.
 * User Story: As a core ecs world consumer, I need to invoke query dirty world entities through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline TArray<EntityKey> queryDirtyWorldEntities(const FWorld &World) {
  return World.DirtyEntities;
}

/**
 * @fn inline TArray<EntityKey> queryChildren(const FQueryChildrenRequest &Request)
 * @brief Returns child entity keys for a parent.
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
 * @fn inline TMap<ComponentType, FComponentValue> gatherComponents(const FGatherComponentsRequest &Request)
 * @brief Gathers available component values for a system input entity.
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
  TMap<ComponentType, FComponentValue> Components;
};

} // namespace ecs
