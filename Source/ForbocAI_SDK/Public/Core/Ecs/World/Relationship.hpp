#pragma once

namespace ecs {


typedef std::function<FRelationship(const FRelationship &)>
    FRelationshipTransform;
typedef std::function<FRelationship(const EntityKey &, const FRelationship &)>
    FRelatedRelationshipTransform;

struct FRelationshipWrite {
  EntityKey Entity;
  FRelationshipTransform Transform;
};

enum class ERelationshipWriteKind {
  AssignParent,
  ClearParent,
  AppendChild,
  RemoveChild
};

struct FRelationshipWriteDeclaration {
  EntityKey Entity;
  ERelationshipWriteKind Kind;
  func::Maybe<EntityKey> RelatedEntity;
};

/**
 * @brief Assigns a parent on one relationship value.
 */
inline FRelationship assignRelationshipParent(const EntityKey &Parent,
                                              const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Parent = func::just(Parent);
  return Next;
}

/**
 * @brief Clears the parent on one relationship value.
 */
inline FRelationship clearRelationshipParent(const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Parent = func::nothing<EntityKey>();
  return Next;
}

/**
 * @brief Adds a child to one relationship value.
 */
inline FRelationship appendRelationshipChild(const EntityKey &Child,
                                             const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Children.AddUnique(Child);
  return Next;
}

/**
 * @brief Removes a child from one relationship value.
 */
inline FRelationship removeRelationshipChildKey(const EntityKey &Child,
                                            const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Children.Remove(Child);
  return Next;
}

inline EntityKey
relationshipWriteRelatedEntity(const FRelationshipWriteDeclaration &Declaration) {
  check(Declaration.RelatedEntity.hasValue);
  return Declaration.RelatedEntity.value;
}

inline FRelationshipTransform createRelatedRelationshipTransform(
    const FRelationshipWriteDeclaration &Declaration,
    FRelatedRelationshipTransform Transform) {
  const EntityKey RelatedEntity = relationshipWriteRelatedEntity(Declaration);
  return FRelationshipTransform(
      [RelatedEntity, Transform](const FRelationship &Relationship) {
        return Transform(RelatedEntity, Relationship);
      });
}

/**
 * @brief Expands one relationship write declaration into an ECS transform.
 */
inline FRelationshipTransform createRelationshipWriteTransform(
    const FRelationshipWriteDeclaration &Declaration) {
  const func::Maybe<FRelationshipTransform> Transform =
      func::multi_match<ERelationshipWriteKind, FRelationshipTransform>(
          Declaration.Kind,
          {func::when<ERelationshipWriteKind, FRelationshipTransform>(
               func::equals(ERelationshipWriteKind::AssignParent),
               [&Declaration](const ERelationshipWriteKind &) {
                 return createRelatedRelationshipTransform(
                     Declaration, assignRelationshipParent);
               }),
           func::when<ERelationshipWriteKind, FRelationshipTransform>(
               func::equals(ERelationshipWriteKind::ClearParent),
               [](const ERelationshipWriteKind &) {
                 return FRelationshipTransform(
                     [](const FRelationship &Relationship) {
                       return clearRelationshipParent(Relationship);
                     });
               }),
           func::when<ERelationshipWriteKind, FRelationshipTransform>(
               func::equals(ERelationshipWriteKind::AppendChild),
               [&Declaration](const ERelationshipWriteKind &) {
                 return createRelatedRelationshipTransform(
                     Declaration, appendRelationshipChild);
               }),
           func::when<ERelationshipWriteKind, FRelationshipTransform>(
               func::equals(ERelationshipWriteKind::RemoveChild),
               [&Declaration](const ERelationshipWriteKind &) {
                 return createRelatedRelationshipTransform(
                     Declaration, removeRelationshipChildKey);
               })});
  check(Transform.hasValue);
  return Transform.value;
}

/**
 * @brief Expands one relationship write declaration into the fold payload.
 */
inline FRelationshipWrite createRelationshipWrite(
    const FRelationshipWriteDeclaration &Declaration) {
  return {Declaration.Entity, createRelationshipWriteTransform(Declaration)};
}

/**
 * @brief Expands relationship write declarations through one composer.
 */
inline TArray<FRelationshipWrite> createRelationshipWrites(
    const TArray<FRelationshipWriteDeclaration> &Declarations) {
  return func::map_array<FRelationshipWriteDeclaration, FRelationshipWrite>(
      Declarations, createRelationshipWrite);
}

inline TArray<FRelationshipWrite> createRelationshipWrites(
    std::initializer_list<FRelationshipWriteDeclaration> Declarations) {
  return createRelationshipWrites(
      TArray<FRelationshipWriteDeclaration>(Declarations));
}

/**
 * @brief Writes one relationship row by upserting a default-or-present value.
 */
inline TMap<EntityKey, FRelationship>
writeRelationship(TMap<EntityKey, FRelationship> Relationships,
                  const FRelationshipWrite &Write) {
  return func::upsert_map_value<EntityKey, FRelationship>(
      Relationships, Write.Entity, createRelationship(), Write.Transform);
}

/**
 * @brief Removes one relationship row from the relationship table.
 */
inline TMap<EntityKey, FRelationship>
removeRelationship(TMap<EntityKey, FRelationship> Relationships,
                   const EntityKey &Entity) {
  Relationships.Remove(Entity);
  return Relationships;
}

/**
 * @brief Applies one relationship write by reading default-or-present first.
 */
inline FWorld applyRelationshipWrite(const FWorld &World,
                                     const FRelationshipWrite &Write) {
  FWorld Next = World;
  Next.Relationships = writeRelationship(Next.Relationships, Write);
  return Next;
}

/**
 * @brief Applies relationship writes through one world update fold.
 */
inline FWorld applyRelationshipWrites(const FWorld &World,
                                      const TArray<FRelationshipWrite> &Writes) {
  return func::fold_array<FRelationshipWrite, FWorld>(
      Writes, World, [](const FWorld &Acc, const FRelationshipWrite &Write) {
        return applyRelationshipWrite(Acc, Write);
      });
}

inline EntityKey relationshipWriteEntity(const FRelationshipWrite &Write) {
  return Write.Entity;
}

inline bool relationshipContainsChild(const EntityKey &Child,
                                      const FRelationship &Relationship) {
  return func::contains_value<EntityKey>(Relationship.Children, Child);
}

inline TArray<EntityKey> findRelationshipParentsForChild(
    const FWorld &World, const EntityKey &Child) {
  return func::filter_array<EntityKey>(
      func::map_keys<EntityKey, FRelationship>(World.Relationships),
      [&World, &Child](const EntityKey &Parent) {
        return relationshipContainsChild(
            Child,
            getRelationshipOrDefault(createGetRelationshipRequest(
                World.Relationships, Parent)));
      });
}

inline FRelationshipWriteDeclaration createRemoveRelationshipChildDeclaration(
    const EntityKey &Parent, const EntityKey &Child) {
  return {Parent, ERelationshipWriteKind::RemoveChild, func::just(Child)};
}

inline TArray<FRelationshipWriteDeclaration>
createRelationshipChildCleanupDeclarations(const FWorld &World,
                                           const EntityKey &Child) {
  return func::map_array<EntityKey, FRelationshipWriteDeclaration>(
      findRelationshipParentsForChild(World, Child),
      [&Child](const EntityKey &Parent) {
        return createRemoveRelationshipChildDeclaration(Parent, Child);
      });
}

/**
 * @brief Records every dirty entity touched by relationship writes.
 */
inline FWorld
recordRelationshipWriteDirtyEntities(const FWorld &World,
                             const TArray<FRelationshipWrite> &Writes) {
  return recordEntitiesDirty(createRecordEntitiesDirtyRequest(
      World,
      func::map_array<FRelationshipWrite, EntityKey>(Writes, relationshipWriteEntity)));
}

/**
 * @brief Applies relationship writes and records touched relationship entities.
 */
inline FWorld applyRelationshipWritesAndRecordDirty(
    const FWorld &World, const TArray<FRelationshipWrite> &Writes) {
  return recordRelationshipWriteDirtyEntities(
      applyRelationshipWrites(World, Writes), Writes);
}

/**
 * @brief Applies declared relationship writes and marks touched entities dirty.
 */
inline FWorld applyRelationshipWriteDeclarations(
    const FWorld &World,
    const TArray<FRelationshipWriteDeclaration> &Declarations) {
  return applyRelationshipWritesAndRecordDirty(
      World, createRelationshipWrites(Declarations));
}

inline FWorld applyRelationshipWriteDeclarations(
    const FWorld &World,
    std::initializer_list<FRelationshipWriteDeclaration> Declarations) {
  return applyRelationshipWriteDeclarations(
      World, TArray<FRelationshipWriteDeclaration>(Declarations));
}

/**
 * @brief Allocates and marks one entity in the supplied world.
 *
 * @return FSpawnedEntity carrying the next world, id, and entity key.
 */

} // namespace ecs
