#pragma once

#include "Core/Ecs/World/Relationship/RelationshipWriteAdapters.hpp"

namespace ecs {

/** User Story: As an ECS relationship cleanup consumer, I need to identify relationship write entities through a stable signature so dirty tracking remains explicit and composable. @fn inline EntityKey relationshipWriteEntity(const FRelationshipWrite &Write) */
inline EntityKey relationshipWriteEntity(const FRelationshipWrite &Write) {
  return Write.Entity;
}

/** User Story: As an ECS relationship cleanup consumer, I need to test child membership through a stable signature so parent cleanup remains explicit and composable. @fn inline bool relationshipContainsChild(const EntityKey &Child, const FRelationship &Relationship) */
inline bool relationshipContainsChild(const EntityKey &Child,
                                      const FRelationship &Relationship) {
  return func::contains_value<EntityKey>(Relationship.Children, Child);
}

/** User Story: As an ECS relationship cleanup consumer, I need to find every parent for a child through a stable signature so stale links can be removed compositionally. @fn inline TArray<EntityKey> findRelationshipParentsForChild( const FWorld &World, const EntityKey &Child) */
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

/** User Story: As an ECS relationship cleanup consumer, I need to declare one child removal through a stable signature so cleanup writes use canonical data. @fn inline FRelationshipWriteDeclaration createRemoveRelationshipChildDeclaration( const EntityKey &Parent, const EntityKey &Child) */
inline FRelationshipWriteDeclaration createRemoveRelationshipChildDeclaration(
    const EntityKey &Parent, const EntityKey &Child) {
  return {Parent, ERelationshipWriteKind::RemoveChild, func::just(Child)};
}

/** User Story: As an ECS relationship cleanup consumer, I need to compose child cleanup declarations through a stable signature so every parent link is removed consistently. @fn inline TArray<FRelationshipWriteDeclaration> createRelationshipChildCleanupDeclarations(const FWorld &World, const EntityKey &Child) */
inline TArray<FRelationshipWriteDeclaration>
createRelationshipChildCleanupDeclarations(const FWorld &World,
                                           const EntityKey &Child) {
  return func::map_array<EntityKey, FRelationshipWriteDeclaration>(
      findRelationshipParentsForChild(World, Child),
      [&Child](const EntityKey &Parent) {
        return createRemoveRelationshipChildDeclaration(Parent, Child);
      });
}

/** User Story: As an ECS relationship cleanup consumer, I need to record every written entity through a stable signature so dirty tracking matches relationship updates. @fn inline FWorld recordRelationshipWriteDirtyEntities(const FWorld &World, const TArray<FRelationshipWrite> &Writes) */
inline FWorld
recordRelationshipWriteDirtyEntities(const FWorld &World,
                                     const TArray<FRelationshipWrite> &Writes) {
  return recordEntitiesDirty(createRecordEntitiesDirtyRequest(
      World, func::map_array<FRelationshipWrite, EntityKey>(
                 Writes, relationshipWriteEntity)));
}

/** User Story: As an ECS relationship cleanup consumer, I need to apply writes and dirty tracking together through a stable signature so world updates remain atomic. @fn inline FWorld applyRelationshipWritesAndRecordDirty( const FWorld &World, const TArray<FRelationshipWrite> &Writes) */
inline FWorld applyRelationshipWritesAndRecordDirty(
    const FWorld &World, const TArray<FRelationshipWrite> &Writes) {
  return recordRelationshipWriteDirtyEntities(
      applyRelationshipWrites(World, Writes), Writes);
}

/** User Story: As an ECS relationship cleanup consumer, I need to apply declared writes through a stable signature so relationship transitions remain data-driven. @fn inline FWorld applyRelationshipWriteDeclarations( const FWorld &World, const TArray<FRelationshipWriteDeclaration> &Declarations) */
inline FWorld applyRelationshipWriteDeclarations(
    const FWorld &World,
    const TArray<FRelationshipWriteDeclaration> &Declarations) {
  return applyRelationshipWritesAndRecordDirty(
      World, createRelationshipWrites(Declarations));
}

/** User Story: As an ECS relationship cleanup consumer, I need to apply initializer-list declarations through a stable signature so small write sets remain composable. @fn inline FWorld applyRelationshipWriteDeclarations( const FWorld &World, std::initializer_list<FRelationshipWriteDeclaration> Declarations) */
inline FWorld applyRelationshipWriteDeclarations(
    const FWorld &World,
    std::initializer_list<FRelationshipWriteDeclaration> Declarations) {
  return applyRelationshipWriteDeclarations(
      World, TArray<FRelationshipWriteDeclaration>(Declarations));
}

} // namespace ecs
