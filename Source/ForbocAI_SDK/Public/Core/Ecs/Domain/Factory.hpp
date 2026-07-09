#pragma once

namespace ecs {

inline FPath createDomainPath(const TArray<FString> &Segments) {
  FPath Path;
  Path.Segments = Segments;
  return Path;
}

inline bool operator==(const FPath &Left, const FPath &Right) {
  return Left.Segments == Right.Segments;
}

inline bool operator!=(const FPath &Left, const FPath &Right) {
  return !(Left == Right);
}

/**
 * @brief Converts a domain path into its registry key.
 * @signature inline DomainPathKey createDomainPathKey(const FPath &Path)
 */
inline DomainPathKey createDomainPathKey(const FPath &Path) {
  const FString Separator = FString::Chr(TCHAR('/'));
  return FString::Join(Path.Segments, *Separator);
}

/**
 * @brief Creates a registry node describing a domain/subdomain boundary.
 * @signature inline FNode createDomainNode(const FCreateDomainNodeRequest &Request)
 *
 * User Story: As a feature maintainer, I need domain-node construction to use
 * one request payload so ECS taxonomy helpers stay unary and composable.
 */
inline FNode createDomainNode(const FCreateDomainNodeRequest &Request) {
  FNode Node;
  Node.Path = Request.Path;
  Node.Kind = Request.Kind;
  return Node;
}

/**
 * @brief Creates a field schema for component/resource/event contracts.
 * @signature inline FFieldSchema createFieldSchema(const FCreateFieldSchemaRequest &Request)
 *
 * User Story: As a registry author, I need schema fields to be built from one
 * explicit payload so neutral domain metadata composes predictably.
 */
inline FFieldSchema createFieldSchema(const FCreateFieldSchemaRequest &Request) {
  FFieldSchema Schema;
  Schema.Name = Request.Name;
  Schema.Kind = Request.Kind;
  Schema.bRequired = Request.bRequired;
  Schema.Description = Request.Description;
  return Schema;
}

/**
 * @brief Creates an ECS component schema owned by one domain path.
 * @signature inline FComponentSchema createComponentSchema(const FCreateComponentSchemaRequest &Request)
 *
 * User Story: As a component-domain owner, I need component schema
 * construction to stay unary and data-shaped for registry folds.
 */
inline FComponentSchema
createComponentSchema(const FCreateComponentSchemaRequest &Request) {
  FComponentSchema Schema;
  Schema.Type = Request.Type;
  Schema.Domain = Request.Domain;
  Schema.Fields = Request.Fields;
  Schema.Tags = Request.Tags;
  Schema.Description = Request.Description;
  return Schema;
}

/**
 * @brief Creates a capability contract describing component/resource access.
 * @signature inline FCapabilitySpec createCapabilitySpec(const FCreateCapabilitySpecRequest &Request)
 *
 * User Story: As an ECS systems author, I need capability metadata to be a
 * plain request-to-value transform below RTK slices.
 */
inline FCapabilitySpec
createCapabilitySpec(const FCreateCapabilitySpecRequest &Request) {
  FCapabilitySpec Spec;
  Spec.Name = Request.Name;
  Spec.Domain = Request.Domain;
  Spec.Reads = Request.Reads;
  Spec.Writes = Request.Writes;
  Spec.Resources = Request.Resources;
  Spec.Emits = Request.Emits;
  return Spec;
}

/**
 * @brief Creates a system contract for registry inspection and validation.
 * @signature inline FSystemSpec createSystemSpec(const FCreateSystemSpecRequest &Request)
 *
 * User Story: As an ECS system author, I need system metadata to remain a
 * value object that reducers may reference without owning system execution.
 */
inline FSystemSpec createSystemSpec(const FCreateSystemSpecRequest &Request) {
  FSystemSpec Spec;
  Spec.Name = Request.Name;
  Spec.Domain = Request.Domain;
  Spec.RequiredComponents = Request.Components;
  Spec.RequiredTags = Request.Tags;
  Spec.Resources = Request.Resources;
  Spec.Emits = Request.Emits;
  return Spec;
}

/**
 * @brief Creates a named ECS resource contract owned by one domain.
 * @signature inline FResourceSpec createResourceSpec(const FCreateResourceSpecRequest &Request)
 *
 * User Story: As a resource-domain maintainer, I need resource specs created
 * from one payload so registration remains foldable and neutral.
 */
inline FResourceSpec
createResourceSpec(const FCreateResourceSpecRequest &Request) {
  FResourceSpec Spec;
  Spec.Name = Request.Name;
  Spec.Domain = Request.Domain;
  Spec.Kind = Request.Kind;
  Spec.Description = Request.Description;
  return Spec;
}

/**
 * @brief Creates a typed event contract owned by one domain.
 * @signature inline FEventSpec createEventSpec(const FCreateEventSpecRequest &Request)
 *
 * User Story: As an event-domain maintainer, I need event specs to remain ECS
 * metadata rather than replacing RTK action semantics.
 */
inline FEventSpec createEventSpec(const FCreateEventSpecRequest &Request) {
  FEventSpec Spec;
  Spec.Type = Request.Type;
  Spec.Domain = Request.Domain;
  Spec.Fields = Request.Fields;
  Spec.Description = Request.Description;
  return Spec;
}

inline bool operator==(const FFieldSchema &Left, const FFieldSchema &Right) {
  return Left.Name == Right.Name && Left.Kind == Right.Kind &&
         Left.bRequired == Right.bRequired &&
         Left.Description == Right.Description;
}

inline bool operator!=(const FFieldSchema &Left, const FFieldSchema &Right) {
  return !(Left == Right);
}

inline bool operator==(const FComponentSchema &Left,
                       const FComponentSchema &Right) {
  return Left.Type == Right.Type && Left.Domain == Right.Domain &&
         Left.Fields == Right.Fields && Left.Tags == Right.Tags &&
         Left.Description == Right.Description;
}

inline bool operator!=(const FComponentSchema &Left,
                       const FComponentSchema &Right) {
  return !(Left == Right);
}

inline bool operator==(const FCapabilitySpec &Left,
                       const FCapabilitySpec &Right) {
  return Left.Name == Right.Name && Left.Domain == Right.Domain &&
         Left.Reads == Right.Reads && Left.Writes == Right.Writes &&
         Left.Resources == Right.Resources && Left.Emits == Right.Emits;
}

inline bool operator!=(const FCapabilitySpec &Left,
                       const FCapabilitySpec &Right) {
  return !(Left == Right);
}

inline bool operator==(const FSystemSpec &Left, const FSystemSpec &Right) {
  return Left.Name == Right.Name && Left.Domain == Right.Domain &&
         Left.RequiredComponents == Right.RequiredComponents &&
         Left.RequiredTags == Right.RequiredTags &&
         Left.Resources == Right.Resources && Left.Emits == Right.Emits;
}

inline bool operator!=(const FSystemSpec &Left, const FSystemSpec &Right) {
  return !(Left == Right);
}

inline bool operator==(const FResourceSpec &Left,
                       const FResourceSpec &Right) {
  return Left.Name == Right.Name && Left.Domain == Right.Domain &&
         Left.Kind == Right.Kind && Left.Description == Right.Description;
}

inline bool operator!=(const FResourceSpec &Left,
                       const FResourceSpec &Right) {
  return !(Left == Right);
}

inline bool operator==(const FEventSpec &Left, const FEventSpec &Right) {
  return Left.Type == Right.Type && Left.Domain == Right.Domain &&
         Left.Fields == Right.Fields && Left.Description == Right.Description;
}

inline bool operator!=(const FEventSpec &Left, const FEventSpec &Right) {
  return !(Left == Right);
}

inline bool operator==(const FNode &Left, const FNode &Right) {
  return Left.Path == Right.Path && Left.Kind == Right.Kind &&
         Left.Children == Right.Children &&
         Left.ComponentSchemas == Right.ComponentSchemas &&
         Left.Capabilities == Right.Capabilities &&
         Left.Systems == Right.Systems && Left.Resources == Right.Resources &&
         Left.Events == Right.Events;
}

inline bool operator!=(const FNode &Left, const FNode &Right) {
  return !(Left == Right);
}

inline bool operator==(const FGraph &Left,
                       const FGraph &Right) {
  return Left.Nodes.OrderIndependentCompareEqual(Right.Nodes) &&
         Left.ComponentSchemas.OrderIndependentCompareEqual(
             Right.ComponentSchemas) &&
         Left.Capabilities.OrderIndependentCompareEqual(Right.Capabilities) &&
         Left.Systems.OrderIndependentCompareEqual(Right.Systems) &&
         Left.ResourceSpecs.OrderIndependentCompareEqual(Right.ResourceSpecs) &&
         Left.EventSpecs.OrderIndependentCompareEqual(Right.EventSpecs);
}

inline bool operator!=(const FGraph &Left,
                       const FGraph &Right) {
  return !(Left == Right);
}

/**
 * @brief Creates an empty ECS domain registry.
 * @signature inline FGraph createDomainRegistry()
 *
 * User Story: As an ECS feature author, I need a fresh registry value before
 * composing domain registrations into a world.
 */
inline FGraph createDomainRegistry() { return FGraph(); }

typedef std::function<FNode(const FNode &)> FDomainNodeTransform;
typedef std::function<FGraph(const FGraph &)>

} // namespace ecs
