#pragma once

namespace ecs {

};

inline bool operator==(const FWorldStorageInspection &Left,
                       const FWorldStorageInspection &Right) {
  return Left.EntityCount == Right.EntityCount &&
         Left.ComponentTypeCount == Right.ComponentTypeCount &&
         Left.ResourceCount == Right.ResourceCount &&
         Left.EventTypeCount == Right.EventTypeCount;
}

inline bool operator!=(const FWorldStorageInspection &Left,
                       const FWorldStorageInspection &Right) {
  return !(Left == Right);
}

inline bool operator==(const FWorldRuntimeInspection &Left,
                       const FWorldRuntimeInspection &Right) {
  return Left.DirtyEntityCount == Right.DirtyEntityCount &&
         Left.DomainCount == Right.DomainCount &&
         Left.Generation == Right.Generation;
}

inline bool operator!=(const FWorldRuntimeInspection &Left,
                       const FWorldRuntimeInspection &Right) {
  return !(Left == Right);
}

/**
 * @brief Compares aggregate ECS world inspection counters.
 */
inline bool operator==(const FWorldInspection &Left,
                       const FWorldInspection &Right) {
  return Left.Storage == Right.Storage && Left.Runtime == Right.Runtime;
}

inline bool operator!=(const FWorldInspection &Left,
                       const FWorldInspection &Right) {
  return !(Left == Right);
}

/**
 * @brief Inspects aggregate world counters without exposing mutable state.
 */
inline FWorldInspection inspectWorld(const FWorld &World) {
  FWorldInspection Inspection;
  Inspection.Storage.EntityCount = collectEntityKeys(World).Num();
  Inspection.Storage.ComponentTypeCount = World.Components.Num();
  Inspection.Storage.ResourceCount = World.Resources.Num();
  Inspection.Storage.EventTypeCount = World.Events.Num();
  Inspection.Runtime.DirtyEntityCount = World.DirtyEntities.Num();
  Inspection.Runtime.DomainCount = World.Domains.Nodes.Num();
  Inspection.Runtime.Generation = World.Generation;
  return Inspection;
}

/**
 * @brief Formats aggregate ECS world inspection data for debugging.
 */
inline FString debugWorldSummary(const FWorld &World) {
  const FWorldInspection Inspection = inspectWorld(World);
  return FString::Printf(
      TEXT("ECS World: entities=%d componentTypes=%d resources=%d events=%d dirty=%d domains=%d generation=%lld"),
      Inspection.Storage.EntityCount, Inspection.Storage.ComponentTypeCount,
      Inspection.Storage.ResourceCount, Inspection.Storage.EventTypeCount,
      Inspection.Runtime.DirtyEntityCount, Inspection.Runtime.DomainCount,
      Inspection.Runtime.Generation);
}

struct FEntityInspection {
  EntityKey Entity;
  TArray<Tag> Tags;
  TArray<DomainPathKey> Domains;
  TMap<ComponentType, FComponentValue> Components;
};

typedef std::function<FEntityInspection(const FEntityInspection &)>
    FEntityInspectionTransform;
typedef std::function<FEntityInspectionTransform(const ComponentType &)>
    FComponentInspectionTransformFactory;
typedef std::function<FComponentInspectionTransformFactory(const EntityKey &)>
    FEntityComponentInspectionFactory;
typedef std::function<TArray<Tag>(const EntityKey &)> FTagInspectionSelector;
typedef std::function<TArray<DomainPathKey>(const EntityKey &)>
    FDomainInspectionSelector;

inline bool operator==(const FEntityInspection &Left,
                       const FEntityInspection &Right) {
  return Left.Entity == Right.Entity && Left.Tags == Right.Tags &&
         Left.Domains == Right.Domains &&
         Left.Components.OrderIndependentCompareEqual(Right.Components);
}

inline bool operator!=(const FEntityInspection &Left,
                       const FEntityInspection &Right) {
  return !(Left == Right);
}

/**
 * @brief Creates an empty inspection record for one entity.
 * @signature inline FEntityInspection createEntityInspection(const EntityKey &Entity)
 *
 * User Story: As inspection code, I need the display record initialized in one
 * reusable pure constructor before selectors add derived values.
 */
inline FEntityInspection createEntityInspection(const EntityKey &Entity) {
  FEntityInspection Inspection;
  Inspection.Entity = Entity;
  return Inspection;
}

/**
 * @brief Builds a selector for entity tags used by inspection.
 * @signature inline FTagInspectionSelector selectEntityTagsForInspection(const FWorld &World)
 *
 * User Story: As display inspection code, tag lookup should be a unary selector
 * over entity keys instead of nullable pointer logic.
 */
inline FTagInspectionSelector selectEntityTagsForInspection(const FWorld &World) {

} // namespace ecs
