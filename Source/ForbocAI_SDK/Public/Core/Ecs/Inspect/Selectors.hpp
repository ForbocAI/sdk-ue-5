#pragma once

namespace ecs {

  return [&World](const EntityKey &Entity) {
    return func::match(
        func::find_map_value<EntityKey, TArray<Tag>>(World.Tags, Entity),
        [](const TArray<Tag> &Tags) { return Tags; },
        []() { return TArray<Tag>(); });
  };
}

/**
 * @brief Builds a selector for entity domains used by inspection.
 * @signature inline FDomainInspectionSelector selectEntityDomainsForInspection(const FWorld &World)
 *
 * User Story: As display inspection code, domain lookup should be a unary
 * selector over entity keys instead of nullable pointer logic.
 */
inline FDomainInspectionSelector selectEntityDomainsForInspection(const FWorld &World) {
  return [&World](const EntityKey &Entity) {
    return func::match(
        func::find_map_value<EntityKey, TArray<DomainPathKey>>(World.EntityDomains,
                                                      Entity),
        [](const TArray<DomainPathKey> &Domains) { return Domains; },
        []() { return TArray<DomainPathKey>(); });
  };
}

/**
 * @brief Builds curried component inspection transforms for one world.
 * @signature inline FEntityComponentInspectionFactory selectEntityComponentsForInspection(const FWorld &World)
 *
 * User Story: As ECS inspection code, component display data should fold
 * through curried transforms instead of component table loops.
 */
inline FEntityComponentInspectionFactory
selectEntityComponentsForInspection(const FWorld &World) {
  return [&World](const EntityKey &Entity) {
    return [&World, Entity](const ComponentType &Type) {
      return [&World, Entity, Type](const FEntityInspection &Inspection) {
        return func::match(
            getComponent({World, Entity, Type}),
            [&Type, &Inspection](const FComponentValue &Value) {
              FEntityInspection Next = Inspection;
              Next.Components.Add(Type, Value);
              return Next;
            },
            [&Inspection]() { return Inspection; });
      };
    };
  };
}

/**
 * @brief Inspects one entity's tags, domains, and component values.
 * @signature inline FEntityInspection inspectEntity(const FWorld &World, const EntityKey &Entity)
 *
 * User Story: As display code, entity inspection should derive read-only
 * projection data from selectors and ECS lookups without owning logic.
 */
inline FEntityInspection inspectEntity(const FWorld &World, const EntityKey &Entity) {
  FEntityInspection Inspection = createEntityInspection(Entity);
  Inspection.Tags = selectEntityTagsForInspection(World)(Entity);
  Inspection.Domains = selectEntityDomainsForInspection(World)(Entity);
  const FComponentInspectionTransformFactory AddComponent =
      selectEntityComponentsForInspection(World)(Entity);
  return func::fold_array<ComponentType, FEntityInspection>(
      func::map_keys<ComponentType, ComponentTable>(World.Components),
      Inspection,
      [AddComponent](const FEntityInspection &Acc,
                     const ComponentType &Type) {
        return AddComponent(Type)(Acc);
      });
}


} // namespace ecs
