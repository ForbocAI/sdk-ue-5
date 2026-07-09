#pragma once

namespace ecs {

    FDomainRegistryTransform;

inline func::Maybe<DomainPathKey> findParentDomainKey(const FPath &Path) {
  return func::fmap(
      func::maybe_filter(
	          func::just<FPath>(Path),
	          [](const FPath &Candidate) {
	            return func::match(
	                func::maybe_filter(
	                    func::just<TArray<FString>>(Candidate.Segments),
	                    [](const TArray<FString> &Segments) {
	                      return !Segments.IsEmpty();
	                    }),
	                [](const TArray<FString> &Segments) {
	                  TArray<FString> ParentCandidate = Segments;
	                  ParentCandidate.Pop(EAllowShrinking::No);
	                  return !ParentCandidate.IsEmpty();
	                },
	                []() { return false; });
	          }),
	      [](const FPath &Candidate) {
	        FPath ParentPath = Candidate;
	        ParentPath.Segments.Pop(EAllowShrinking::No);
	        return createDomainPathKey(ParentPath);
	      });
}

inline FDomainRegistryTransform
updateDomainNode(const DomainPathKey &Key, FDomainNodeTransform Transform) {
  return [Key, Transform](const FGraph &Registry) {
    FGraph Next = Registry;
    Next.Nodes =
        func::update_map_value_when_present<DomainPathKey, FNode>(
            Next.Nodes, Key, Transform);
    return Next;
  };
}

inline FNode appendDomainChild(const DomainPathKey &Child,
                                     const FNode &Node) {
  FNode Next = Node;
  Next.Children =
      func::append_unique_value<DomainPathKey>(Next.Children, Child);
  return Next;
}

inline FNode appendDomainComponentSchema(const ComponentType &Type,
                                               const FNode &Node) {
  FNode Next = Node;
  Next.ComponentSchemas =
      func::append_unique_value<ComponentType>(Next.ComponentSchemas, Type);
  return Next;
}

inline FNode appendDomainCapability(const FString &Name,
                                          const FNode &Node) {
  FNode Next = Node;
  Next.Capabilities =
      func::append_unique_value<FString>(Next.Capabilities, Name);
  return Next;
}

inline FNode appendDomainSystem(const SystemName &Name,
                                      const FNode &Node) {
  FNode Next = Node;
  Next.Systems = func::append_unique_value<SystemName>(Next.Systems, Name);
  return Next;
}

inline FNode appendDomainResource(const ResourceName &Name,
                                        const FNode &Node) {
  FNode Next = Node;
  Next.Resources =
      func::append_unique_value<ResourceName>(Next.Resources, Name);
  return Next;
}

inline FNode appendDomainEvent(const EventType &Type,
                                     const FNode &Node) {
  FNode Next = Node;
  Next.Events = func::append_unique_value<EventType>(Next.Events, Type);
  return Next;
}

/**
 * @brief Registers one domain node and wires it to its parent when present.
 * @signature inline FGraph registerDomain(const FRegisterDomainRequest &Request)
 *
 * Architecture: This keeps neutral domain topology below feature slices so
 * feature code can import downward without borrowing sibling-domain helpers.
 */
inline FGraph registerDomain(const FRegisterDomainRequest &Request) {
  FGraph Registry = Request.Registry;
  const FNode Node = Request.Node;
  const DomainPathKey Key = createDomainPathKey(Node.Path);
  Registry.Nodes.Add(Key, Node);
  return func::match(
      findParentDomainKey(Node.Path),
      [Registry, Key](const DomainPathKey &ParentKey) mutable {
        return updateDomainNode(
            ParentKey,
            [Key](const FNode &Parent) {
              return appendDomainChild(Key, Parent);
            })(Registry);
      },
      [Registry]() { return Registry; });
}

/**
 * @brief Unary helper for registering a domain from path segments and kind.
 * @signature inline FGraph registerDomainPath(const FRegisterDomainPathRequest &Request)
 *
 * User Story: As a feature author, I can register ECS subdomains through one
 * payload, matching the unary functional-core cookbook pattern.
 */
inline FGraph
registerDomainPath(const FRegisterDomainPathRequest &Request) {
  return registerDomain(
      {Request.Registry,
       createDomainNode({createDomainPath(Request.Segments), Request.Kind})});
}

/**
 * @brief Registers domain path declarations through the shared array fold.
 * @signature inline FGraph registerDomainPaths(FRegisterDomainPathsRequest Request)
 *
 * User Story: As a maintainer, I can extend the ECS taxonomy by appending data
 * declarations while one fold owns the traversal shape.
 */
inline FGraph
registerDomainPaths(FRegisterDomainPathsRequest Request) {
  return func::fold_array<FPathRegistration, FGraph>(
      Request.Registrations, Request.Registry,
      [](const FGraph &Registry,
         const FPathRegistration &Registration) {
        return registerDomainPath(
            {Registry, Registration.Segments, Registration.Kind});
      });
}

/**
 * @brief Registers ECS domain path declarations through one boundary function.
 * @signature inline FGraph registerDomainPathDeclarations(FGraph Registry, std::initializer_list<FPathRegistration> Declarations)
 *
 * User Story: As taxonomy code, path declarations should be plain atoms while
 * this ECS helper owns platform string conversion, request construction, and
 * fold execution.
 */
inline FGraph registerDomainPathDeclarations(
    FGraph Registry,
    std::initializer_list<FPathRegistration> Declarations) {
  const TArray<FPathRegistration> Registrations(Declarations);
  return registerDomainPaths({Registry, Registrations});
}

/**
 * @brief Registers one component schema and indexes it under its owning domain.
 * @signature inline FGraph registerComponentSchema(const FRegisterComponentSchemaRequest &Request)
 *
 * User Story: As a component author, schema registration is a unary payload
 * transition over a registry value.
 */
inline FGraph
registerComponentSchema(const FRegisterComponentSchemaRequest &Request) {
  FGraph Registry = Request.Registry;
  const FComponentSchema Schema = Request.Schema;
  Registry.ComponentSchemas.Add(Schema.Type, Schema);
  return updateDomainNode(
      Schema.Domain, [Schema](const FNode &Domain) {
        return appendDomainComponentSchema(Schema.Type, Domain);
      })(Registry);
}

/**
 * @brief Registers one capability spec and indexes it under its owning domain.
 * @signature inline FGraph registerCapabilitySpec(const FRegisterCapabilitySpecRequest &Request)
 *
 * User Story: As a systems author, capability registration stays a value
 * transition that can be composed below RTK slices.
 */
inline FGraph
registerCapabilitySpec(const FRegisterCapabilitySpecRequest &Request) {
  FGraph Registry = Request.Registry;
  const FCapabilitySpec Spec = Request.Spec;
  Registry.Capabilities.Add(Spec.Name, Spec);
  return updateDomainNode(
      Spec.Domain, [Spec](const FNode &Domain) {
        return appendDomainCapability(Spec.Name, Domain);
      })(Registry);
}

/**
 * @brief Registers one system spec and indexes it under its owning domain.
 * @signature inline FGraph registerSystemSpec(const FRegisterSystemSpecRequest &Request)
 *
 * User Story: As an ECS systems author, system registration remains a pure
 * registry transform and never owns store semantics.
 */
inline FGraph
registerSystemSpec(const FRegisterSystemSpecRequest &Request) {
  FGraph Registry = Request.Registry;
  const FSystemSpec Spec = Request.Spec;
  Registry.Systems.Add(Spec.Name, Spec);
  return updateDomainNode(
      Spec.Domain, [Spec](const FNode &Domain) {
        return appendDomainSystem(Spec.Name, Domain);
      })(Registry);
}

/**
 * @brief Registers one resource spec and indexes it under its owning domain.
 * @signature inline FGraph registerResourceSpec(const FRegisterResourceSpecRequest &Request)
 *
 * User Story: As a resource author, resource registration stays below feature
 * domains and composes as a unary ECS transform.
 */
inline FGraph
registerResourceSpec(const FRegisterResourceSpecRequest &Request) {
  FGraph Registry = Request.Registry;
  const FResourceSpec Spec = Request.Spec;
  Registry.ResourceSpecs.Add(Spec.Name, Spec);
  return updateDomainNode(
      Spec.Domain, [Spec](const FNode &Domain) {
        return appendDomainResource(Spec.Name, Domain);
      })(Registry);
}

/**
 * @brief Alias for resource-spec registration used by feature code.
 * @signature inline FGraph registerResource(const FRegisterResourceSpecRequest &Request)
 *
 * User Story: As feature code, I can use RTK-style explicit names while ECS
 * resources remain plain registry values.
 */
inline FGraph
registerResource(const FRegisterResourceSpecRequest &Request) {
  return registerResourceSpec(Request);
}

/**
 * @brief Registers one event spec and indexes it under its owning domain.
 * @signature inline FGraph registerEventSpec(const FRegisterEventSpecRequest &Request)
 *
 * User Story: As an ECS event author, event registration is declarative domain
 * metadata and does not replace RTK actions.
 */
inline FGraph
registerEventSpec(const FRegisterEventSpecRequest &Request) {
  FGraph Registry = Request.Registry;
  const FEventSpec Spec = Request.Spec;
  Registry.EventSpecs.Add(Spec.Type, Spec);
  return updateDomainNode(
      Spec.Domain, [Spec](const FNode &Domain) {
        return appendDomainEvent(Spec.Type, Domain);
      })(Registry);
}

/**
 * @brief Alias for event-spec registration used by feature code.
 * @signature inline FGraph registerEventType(const FRegisterEventSpecRequest &Request)
 *
 * User Story: As feature code, I can declare ECS event metadata without
 * confusing it with dispatched RTK action events.
 */
inline FGraph
registerEventType(const FRegisterEventSpecRequest &Request) {
  return registerEventSpec(Request);
}

/**
 * @brief Builds a domain registry from authored domain path declarations.
 * @signature inline FGraph createDomainRegistry(const TArray<FPathRegistration> &Registrations)
 *
 * User Story: As a runtime data author, ECS taxonomy should be loaded from
 * authored settings while the core still owns the registry fold machinery.
 */
inline FGraph
createDomainRegistry(const TArray<FPathRegistration> &Registrations) {
  return registerDomainPaths({createDomainRegistry(), Registrations});
}

} // namespace ecs
