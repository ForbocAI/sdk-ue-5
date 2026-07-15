#pragma once

namespace ecs {

template <typename Item>
using TWorldRowProjector =
    std::function<FWorld(const FWorld &, const Item &)>;

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

template <typename TransformCatalog>
inline FWorld applyWorldTransformCatalog(const FWorld &World,
                                         const TransformCatalog &Transforms) {
  return func::fold_catalog(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

inline FWorld applyWorldTransforms(const FWorld &World,
                                   const TArray<FWorldTransform> &Transforms) {
  return func::fold_array<FWorldTransform, FWorld>(
      Transforms, World,
      [](const FWorld &Acc, const FWorldTransform &Transform) {
        return Transform(Acc);
      });
}

} // namespace ecs
