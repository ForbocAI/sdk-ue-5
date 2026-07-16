#pragma once

namespace ecs {

template <typename Item>
using TWorldRowProjector =
    std::function<FWorld(const FWorld &, const Item &)>;

/** User Story: As a core ecs world consumer, I need to invoke project rows into world through a stable signature so the core ecs world workflow remains explicit and composable. @fn template <typename Item> inline FWorldTransform projectRowsIntoWorld(const TArray<Item> &Items, TWorldRowProjector<Item> Project) */
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

/** User Story: As a core ecs world consumer, I need to invoke apply world transform catalog through a stable signature so the core ecs world workflow remains explicit and composable. @fn template <typename TransformCatalog> inline FWorld applyWorldTransformCatalog(const FWorld &World, const TransformCatalog &Transforms) */
template <typename TransformCatalog>
inline FWorld applyWorldTransformCatalog(const FWorld &World,
                                         const TransformCatalog &Transforms) {
  return func::fold_catalog(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

/** User Story: As a core ecs world consumer, I need to invoke apply world transforms through a stable signature so the core ecs world workflow remains explicit and composable. @fn inline FWorld applyWorldTransforms(const FWorld &World, const TArray<FWorldTransform> &Transforms) */
inline FWorld applyWorldTransforms(const FWorld &World,
                                   const TArray<FWorldTransform> &Transforms) {
  return func::fold_array<FWorldTransform, FWorld>(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

} // namespace ecs
