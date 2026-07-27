#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

namespace ecs {

typedef TFunction<FWorld(const FSystemExecutionPayload &)> SystemFn;

struct FDescriptor {
  TArray<ComponentType> RequiredComponents;
  TArray<Tag> RequiredTags;
  func::Maybe<DomainPathKey> Domain;
  SystemFn System;
};

struct FRunSystemRequest {
  FWorld World;
  TArray<ComponentType> Components;
  TArray<EntityKey> Entities;
  SystemFn System;
};

struct FRunSystemDescriptorRequest {
  FWorld World;
  TArray<EntityKey> Entities;
  FDescriptor Descriptor;
};

/**
 * @fn inline std::function<bool(const EntityKey &)> requireEntityTags(const FWorld &World, const TArray<Tag> &Tags)
 * @brief Creates a reusable predicate that checks all required tags.
 *
 * User Story: As ECS system matching, tag requirements should be expressed as
 * a function factory returning a unary predicate over entities.
 */
inline std::function<bool(const EntityKey &)>
requireEntityTags(const FWorld &World, const TArray<Tag> &Tags) {
  return [&World, Tags](const EntityKey &Entity) {
    return func::all_array<Tag>(
        Tags,
        [&World, &Entity](const Tag &TagValue) {
          return hasTag({World, Entity, TagValue});
        });
  };
}

/**
 * @fn inline std::function<bool(const EntityKey &)> requireEntityOptionalDomain(const FWorld &World, const func::Maybe<DomainPathKey> &Domain)
 * @brief Creates a reusable predicate that checks an optional domain guard.
 *
 * User Story: As ECS system matching, optional domain guards should be
 * first-class unary predicates that compose with tag predicates.
 */
inline std::function<bool(const EntityKey &)>
requireEntityOptionalDomain(const FWorld &World,
                      const func::Maybe<DomainPathKey> &Domain) {
  return [&World, Domain](const EntityKey &Entity) {
    return func::match(
        Domain,
        [&World, &Entity](const DomainPathKey &DomainValue) {
          return isEntityInDomain({World, Entity, DomainValue});
        },
        []() { return true; });
  };
}

/**
 * @fn inline std::function<bool(const EntityKey &)> requireSystemDescriptor(const FWorld &World, const FDescriptor &Descriptor)
 * @brief Creates a reusable predicate for descriptor tag/domain requirements.
 *
 * User Story: As ECS system execution, descriptor matching should be a
 * functional composition of smaller entity predicates.
 */
inline std::function<bool(const EntityKey &)>
requireSystemDescriptor(const FWorld &World,
                        const FDescriptor &Descriptor) {
  return func::all_pass<EntityKey>(
      {requireEntityTags(World, Descriptor.RequiredTags),
       requireEntityOptionalDomain(World, Descriptor.Domain)});
}

/**
 * @fn inline FWorld runSystem(const FRunSystemRequest &Request)
 * @brief Runs one pure ECS system over matching entities.
 *
 * User Story: As ECS execution code, running a system should fold over matching
 * entities and invoke systems with a single execution payload.
 */
inline FWorld runSystem(const FRunSystemRequest &Request) {
  const TArray<EntityKey> Matching =
      queryComponents({Request.World, Request.Components, Request.Entities});
  return func::fold_array<EntityKey, FWorld>(
      Matching, Request.World,
      [&Request](const FWorld &World, const EntityKey &Entity) {
        return Request.System({World, Entity,
                               gatherComponents(
                                   {World, Entity, Request.Components})});
      });
}

/**
 * @fn inline FWorld runSystemDescriptor(const FRunSystemDescriptorRequest &Request)
 * @brief Runs a system descriptor over entities matching components, tags, and
 * optional domain.
 *
 * User Story: As ECS execution code, descriptor execution should compose
 * component queries, descriptor guards, and system invocation through shared
 * request helpers.
 */
inline FWorld
runSystemDescriptor(const FRunSystemDescriptorRequest &Request) {
  const TArray<EntityKey> ComponentMatches =
      queryComponents({Request.World, Request.Descriptor.RequiredComponents,
                       Request.Entities});
  const std::function<bool(const EntityKey &)> MatchesDescriptor =
      requireSystemDescriptor(Request.World, Request.Descriptor);
  const TArray<EntityKey> Matching =
      func::filter_array<EntityKey>(ComponentMatches, MatchesDescriptor);
  return runSystem({Request.World, Request.Descriptor.RequiredComponents,
                    Matching, Request.Descriptor.System});
}

struct FValidateEntityDomainRequest {
  const FWorld &World;
  EntityKey Entity;
  DomainPathKey Domain;
};

struct FValidateSystemSpecRequest {
  const FGraph &Registry;
  FSystemSpec Spec;
};

/**
 * @fn inline std::function<func::Either<FString, bool>(const DomainPathKey &)> requireDomain(const FGraph &Registry)
 * @brief Creates a reusable domain-exists validator for a registry.
 *
 * User Story: As registry validation, domain validation should be a reusable
 * function factory over a registry.
 */
inline std::function<func::Either<FString, bool>(const DomainPathKey &)>
requireDomain(const FGraph &Registry) {
  return func::require_map_key<DomainPathKey, FNode>(
      Registry.Nodes, [](const DomainPathKey &Domain) {
        return FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV41C5683C9669), *Domain);
      });
}

/**
 * @fn inline std::function<func::Either<FString, bool>(const ComponentType &)> requireComponentSchema(const FGraph &Registry)
 * @brief Creates a reusable component-schema validator for a registry.
 *
 * User Story: As registry validation, component schema validation should be a
 * reusable function factory over a registry.
 */
inline std::function<func::Either<FString, bool>(const ComponentType &)>
requireComponentSchema(const FGraph &Registry) {
  return func::require_map_key<ComponentType, FComponentSchema>(
      Registry.ComponentSchemas, [](const ComponentType &Type) {
        return FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV92006BD5179E),
                               *Type);
      });
}

/**
 * @fn inline FValidateSystemSpecRequest createValidateSystemSpecRequest(const FGraph &Registry, const FSystemSpec &Spec)
 * @brief Builds a system-spec validation request payload.
 *
 * User Story: As registry validation, system spec validation remains one
 * payload at the public multi-input boundary.
 */
inline FValidateSystemSpecRequest
createValidateSystemSpecRequest(const FGraph &Registry,
                                const FSystemSpec &Spec) {
  return {Registry, Spec};
}

/**
 * @fn inline func::Either<FString, bool> validateEntityDomain(const FValidateEntityDomainRequest &Request)
 * @brief Validates entity membership in a domain.
 *
 * User Story: As ECS validation, domain membership failures should return
 * Either errors and reuse entity-domain predicates.
 */
inline func::Either<FString, bool>
validateEntityDomain(const FValidateEntityDomainRequest &Request) {
  return isEntityInDomain({Request.World, Request.Entity, Request.Domain})
             ? func::make_right<FString, bool>(true)
             : func::make_left<FString, bool>(FString::Printf(
                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC45D95512FAE), *Request.Entity,
                   *Request.Domain));
}

/**
 * @fn inline func::Either<FString, bool> validateSystemSpec(const FValidateSystemSpecRequest &Request)
 * @brief Validates a system spec against registered domain/component contracts.
 *
 * User Story: As registry validation, system spec checks should compose domain
 * and component validation through fp Either semantics.
 */
inline func::Either<FString, bool>
validateSystemSpec(const FValidateSystemSpecRequest &Request) {
  return func::ebind(
      requireDomain(Request.Registry)(Request.Spec.Domain),
      [&Request](const bool &) {
        return func::fold_either_array<FString, ComponentType, bool>(
            Request.Spec.RequiredComponents, true,
            [&Request](const bool &, const ComponentType &Type) {
              return requireComponentSchema(Request.Registry)(Type);
            });
      });
}

typedef func::ArgDispatcher<int32, FComponentValue, FString>
    FComponentValueFormatter;

} // namespace ecs
