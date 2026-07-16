#pragma once

namespace ecs {


struct FRelationship {
  func::Maybe<EntityKey> Parent;
  TArray<EntityKey> Children;
};

struct FGetRelationshipRequest {
  const TMap<EntityKey, FRelationship> &Relationships;
  EntityKey Entity;
};

/**
 * @fn inline FRelationship createRelationship()
 * @brief Creates an empty parent/children relationship record.
 * User Story: As a core ecs world consumer, I need to invoke create relationship through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationship createRelationship() {
  FRelationship Relationship;
  Relationship.Parent = func::nothing<EntityKey>();
  return Relationship;
}

/** User Story: As a core ecs world consumer, I need to compare values for equality through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline bool operator==(const FRelationship &Left, const FRelationship &Right) */
inline bool operator==(const FRelationship &Left,
                       const FRelationship &Right) {
  return Left.Parent.hasValue == Right.Parent.hasValue &&
         (!Left.Parent.hasValue || Left.Parent.value == Right.Parent.value) &&
         Left.Children == Right.Children;
}

/** User Story: As a core ecs world consumer, I need to compare values for inequality through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline bool operator!=(const FRelationship &Left, const FRelationship &Right) */
inline bool operator!=(const FRelationship &Left,
                       const FRelationship &Right) {
  return !(Left == Right);
}

/**
 * @fn inline FGetRelationshipRequest createGetRelationshipRequest(const TMap<EntityKey, FRelationship> &Relationships, const EntityKey &Entity)
 * @brief Builds a relationship lookup request payload.
 *
 * User Story: As hierarchy ECS code, I need relationship reads to share one
 * lookup payload instead of repeating map branches.
 */
inline FGetRelationshipRequest
createGetRelationshipRequest(const TMap<EntityKey, FRelationship> &Relationships,
                             const EntityKey &Entity) {
  return {Relationships, Entity};
}

/**
 * @fn inline FRelationship getRelationshipOrDefault(const FGetRelationshipRequest &Request)
 * @brief Reads a relationship or returns an empty default relationship.
 *
 * User Story: As relationship reducers, parent and child updates should share
 * one Maybe-backed read helper.
 */
inline FRelationship
getRelationshipOrDefault(const FGetRelationshipRequest &Request) {
  return func::or_else(
      func::find_map_value<EntityKey, FRelationship>(Request.Relationships,
                                             Request.Entity),
      createRelationship());
}

struct FWorld {
  FAllocator Allocator;
  ComponentStore Components;
  TagStore Tags;
  ResourceStore Resources;
  EventQueue Events;
  TMap<EntityKey, TArray<DomainPathKey>> EntityDomains;
  TMap<EntityKey, FRelationship> Relationships;
  FGraph Domains;
  TArray<EntityKey> DirtyEntities;
  int64 Generation;
};

typedef std::function<FWorld(const FWorld &)> FWorldTransform;
typedef std::function<FWorldTransform(const ComponentType &)>
    FComponentTypeWorldTransformFactory;

struct FSpawnedEntity {
  FWorld World;
  FEntityId Id;
  EntityKey Entity;
};

struct FSetComponentRequest {
  FWorld World;
  EntityKey Entity;
  ComponentType Type;
  FComponentValue Value;
};

struct FSetTagRequest {
  FWorld World;
  EntityKey Entity;
  Tag TagValue;
};

struct FSetResourceRequest {
  FWorld World;
  ResourceName Name;
  FComponentValue Value;
};

struct FPushEventRequest {
  FWorld World;
  EventType Type;
  FComponentValue Payload;
};

struct FSetEntityDomainRequest {
  FWorld World;
  EntityKey Entity;
  DomainPathKey Domain;
};

struct FRecordEntityDirtyRequest {
  FWorld World;
  EntityKey Entity;
};

struct FRecordEntitiesDirtyRequest {
  FWorld World;
  TArray<EntityKey> Entities;
};

struct FSpawnEntityInDomainRequest {
  FWorld World;
  DomainPathKey Domain;
};

struct FRemoveEntityRequest {
  FWorld World;
  EntityKey Entity;
};

struct FDespawnEntityRequest {
  FWorld World;
  FEntityId Id;
};

struct FSetRelationshipParentRequest {
  FWorld World;
  EntityKey Child;
  EntityKey Parent;
};

struct FAddRelationshipChildRequest {
  FWorld World;
  EntityKey Parent;
  EntityKey Child;
};

struct FRemoveRelationshipChildRequest {
  FWorld World;
  EntityKey Parent;
  EntityKey Child;
};

struct FGetComponentRequest {
  const FWorld &World;
  EntityKey Entity;
  ComponentType Type;
};

struct FHasComponentRequest {
  const FWorld &World;
  EntityKey Entity;
  ComponentType Type;
};

struct FRemoveComponentRequest {
  FWorld World;
  EntityKey Entity;
  ComponentType Type;
};

struct FHasTagRequest {
  const FWorld &World;
  EntityKey Entity;
  Tag TagValue;
};

struct FGetResourceRequest {
  const FWorld &World;
  ResourceName Name;
};

struct FReadEventsRequest {
  const FWorld &World;
  EventType Type;
};

struct FEntityInDomainRequest {
  const FWorld &World;
  EntityKey Entity;
  DomainPathKey Domain;
};

struct FQueryComponentsRequest {
  const FWorld &World;
  TArray<ComponentType> Types;
  TArray<EntityKey> Entities;
};

struct FQueryEntitiesByComponentsRequest {
  const FWorld &World;
  TArray<ComponentType> Types;
};

struct FQueryEntitiesByTagRequest {
  const FWorld &World;
  Tag TagValue;
};

struct FQueryEntitiesByDomainRequest {
  const FWorld &World;
  DomainPathKey Domain;
};

struct FQueryChildrenRequest {
  const FWorld &World;
  EntityKey Parent;
};

struct FGatherComponentsRequest {
  const FWorld &World;
  EntityKey Entity;
  TArray<ComponentType> Types;
};

} // namespace ecs
