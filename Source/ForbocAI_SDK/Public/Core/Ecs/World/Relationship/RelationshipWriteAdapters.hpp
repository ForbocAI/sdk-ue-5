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
 * @fn inline FRelationship assignRelationshipParent(const EntityKey &Parent, const FRelationship &Value)
 * @brief Assigns a parent on one relationship value.
 * User Story: As a core ecs world consumer, I need to invoke assign relationship parent through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationship assignRelationshipParent(const EntityKey &Parent,
                                              const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Parent = func::just(Parent);
  return Next;
}

/**
 * @fn inline FRelationship clearRelationshipParent(const FRelationship &Value)
 * @brief Clears the parent on one relationship value.
 * User Story: As a core ecs world consumer, I need to invoke clear relationship parent through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationship clearRelationshipParent(const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Parent = func::nothing<EntityKey>();
  return Next;
}

/**
 * @fn inline FRelationship appendRelationshipChild(const EntityKey &Child, const FRelationship &Value)
 * @brief Adds a child to one relationship value.
 * User Story: As a core ecs world consumer, I need to invoke append relationship child through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationship appendRelationshipChild(const EntityKey &Child,
                                             const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Children.AddUnique(Child);
  return Next;
}

/**
 * @fn inline FRelationship removeRelationshipChildKey(const EntityKey &Child, const FRelationship &Value)
 * @brief Removes a child from one relationship value.
 * User Story: As a core ecs world consumer, I need to invoke remove relationship child key through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationship removeRelationshipChildKey(const EntityKey &Child,
                                            const FRelationship &Value) {
  FRelationship Next = Value;
  Next.Children.Remove(Child);
  return Next;
}

/** User Story: As a core ecs world consumer, I need to invoke relationship write related entity through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline EntityKey relationshipWriteRelatedEntity(const FRelationshipWriteDeclaration &Declaration) */
inline EntityKey
relationshipWriteRelatedEntity(const FRelationshipWriteDeclaration &Declaration) {
  check(Declaration.RelatedEntity.hasValue);
  return Declaration.RelatedEntity.value;
}

/** User Story: As a core ecs world consumer, I need to invoke create related relationship transform through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FRelationshipTransform createRelatedRelationshipTransform( const FRelationshipWriteDeclaration &Declaration, FRelatedRelationshipTransform Transform) */
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
 * @fn inline FRelationshipTransform createRelationshipWriteTransform( const FRelationshipWriteDeclaration &Declaration)
 * @brief Expands one relationship write declaration into an ECS transform.
 * User Story: As a core ecs world consumer, I need to invoke create relationship write transform through a stable signature so the core ecs world workflow remains explicit and composable.
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
 * @fn inline FRelationshipWrite createRelationshipWrite( const FRelationshipWriteDeclaration &Declaration)
 * @brief Expands one relationship write declaration into the fold payload.
 * User Story: As a core ecs world consumer, I need to invoke create relationship write through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FRelationshipWrite createRelationshipWrite(
    const FRelationshipWriteDeclaration &Declaration) {
  return {Declaration.Entity, createRelationshipWriteTransform(Declaration)};
}

/**
 * @fn inline TArray<FRelationshipWrite> createRelationshipWrites( const TArray<FRelationshipWriteDeclaration> &Declarations)
 * @brief Expands relationship write declarations through one composer.
 * User Story: As a core ecs world consumer, I need to invoke create relationship writes through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline TArray<FRelationshipWrite> createRelationshipWrites(
    const TArray<FRelationshipWriteDeclaration> &Declarations) {
  return func::map_array<FRelationshipWriteDeclaration, FRelationshipWrite>(
      Declarations, createRelationshipWrite);
}

/** User Story: As a core ecs world consumer, I need to invoke create relationship writes through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline TArray<FRelationshipWrite> createRelationshipWrites( std::initializer_list<FRelationshipWriteDeclaration> Declarations) */
inline TArray<FRelationshipWrite> createRelationshipWrites(
    std::initializer_list<FRelationshipWriteDeclaration> Declarations) {
  return createRelationshipWrites(
      TArray<FRelationshipWriteDeclaration>(Declarations));
}

/**
 * @fn inline TMap<EntityKey, FRelationship> writeRelationship(TMap<EntityKey, FRelationship> Relationships, const FRelationshipWrite &Write)
 * @brief Writes one relationship row by upserting a default-or-present value.
 * User Story: As a core ecs world consumer, I need to invoke write relationship through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline TMap<EntityKey, FRelationship>
writeRelationship(TMap<EntityKey, FRelationship> Relationships,
                  const FRelationshipWrite &Write) {
  return func::upsert_map_value<EntityKey, FRelationship>(
      Relationships, Write.Entity, createRelationship(), Write.Transform);
}

/**
 * @fn inline TMap<EntityKey, FRelationship> removeRelationship(TMap<EntityKey, FRelationship> Relationships, const EntityKey &Entity)
 * @brief Removes one relationship row from the relationship table.
 * User Story: As a core ecs world consumer, I need to invoke remove relationship through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline TMap<EntityKey, FRelationship>
removeRelationship(TMap<EntityKey, FRelationship> Relationships,
                   const EntityKey &Entity) {
  Relationships.Remove(Entity);
  return Relationships;
}

/**
 * @fn inline FWorld applyRelationshipWrite(const FWorld &World, const FRelationshipWrite &Write)
 * @brief Applies one relationship write by reading default-or-present first.
 * User Story: As a core ecs world consumer, I need to invoke apply relationship write through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld applyRelationshipWrite(const FWorld &World,
                                     const FRelationshipWrite &Write) {
  FWorld Next = World;
  Next.Relationships = writeRelationship(Next.Relationships, Write);
  return Next;
}

/**
 * @fn inline FWorld applyRelationshipWrites(const FWorld &World, const TArray<FRelationshipWrite> &Writes)
 * @brief Applies relationship writes through one world update fold.
 * User Story: As a core ecs world consumer, I need to invoke apply relationship writes through a stable signature so the core ecs world workflow remains explicit and composable.
 */
inline FWorld applyRelationshipWrites(const FWorld &World,
                                      const TArray<FRelationshipWrite> &Writes) {
  return func::fold_array<FRelationshipWrite, FWorld>(
      Writes, World, [](const FWorld &Acc, const FRelationshipWrite &Write) {
        return applyRelationshipWrite(Acc, Write);
      });
}

} // namespace ecs
