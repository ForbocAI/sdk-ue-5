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
 * @brief Creates an empty parent/children relationship record.
 */
inline FRelationship createRelationship() {
  FRelationship Relationship;
  Relationship.Parent = func::nothing<EntityKey>();
  return Relationship;
}

inline bool operator==(const FRelationship &Left,
                       const FRelationship &Right) {
  return Left.Parent.hasValue == Right.Parent.hasValue &&
         (!Left.Parent.hasValue || Left.Parent.value == Right.Parent.value) &&
         Left.Children == Right.Children;
}

inline bool operator!=(const FRelationship &Left,
                       const FRelationship &Right) {
  return !(Left == Right);
}

/**
 * @brief Builds a relationship lookup request payload.
 * @signature inline FGetRelationshipRequest createGetRelationshipRequest(const TMap<EntityKey, FRelationship> &Relationships, const EntityKey &Entity)
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
 * @brief Reads a relationship or returns an empty default relationship.
 * @signature inline FRelationship getRelationshipOrDefault(const FGetRelationshipRequest &Request)
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

template <typename Item>
using TWorldRowProjector =
    std::function<FWorld(const FWorld &, const Item &)>;

/**
 * @brief Projects selected rows into a world through one row projector.
 * @signature template <typename Item> inline FWorldTransform projectRowsIntoWorld(const TArray<Item> &Items, TWorldRowProjector<Item> Project)
 *
 * User Story: As RTK selector projection code, selected entity-adapter rows
 * should fold into ECS through one reusable world transform shape.
 */
template <typename Item>
inline FWorldTransform projectRowsIntoWorld(const TArray<Item> &Items,
                                            TWorldRowProjector<Item> Project) {
  return [Items, Project](const FWorld &World) {
    return func::fold_array<Item, FWorld>(
        Items, World, [Project](const FWorld &Acc, const Item &ItemValue) {
          return Project(Acc, ItemValue);
        });
  };
}

/**
 * @brief Applies a catalog of world transforms into one next world.
 * @signature template <typename TransformCatalog> inline FWorld applyWorldTransformCatalog(const FWorld &World, const TransformCatalog &Transforms)
 *
 * User Story: As lifecycle ECS code, named world transforms should be
 * registered as a catalog when only the cleanup noun changes.
 */
template <typename TransformCatalog>
inline FWorld applyWorldTransformCatalog(const FWorld &World,
                                         const TransformCatalog &Transforms) {
  return func::fold_catalog(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

/**
 * @brief Applies a list of world transforms to one next world.
 * @signature inline FWorld applyWorldTransforms(const FWorld &World, const TArray<FWorldTransform> &Transforms)
 *
 * User Story: As reducer-owned ECS projection, system execution should read as
 * a fold over named world transforms instead of hand-composed nested calls.
 */
inline FWorld applyWorldTransforms(const FWorld &World,
                                   const TArray<FWorldTransform> &Transforms) {
  return func::fold_array<FWorldTransform, FWorld>(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

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

/**
 * @brief Builds the dirty entity recording request payload.
 * @signature inline FRecordEntityDirtyRequest createRecordEntityDirtyRequest(FWorld World, const EntityKey &Entity)
 *
 * User Story: As ECS reducer code, I need one reusable factory for dirty
 * marking payloads instead of scattered aggregate construction.
 */
inline FRecordEntityDirtyRequest createRecordEntityDirtyRequest(FWorld World,

} // namespace ecs
