#pragma once

namespace ecs {


enum class EKind {
  Component,
  Entity,
  System,
  Ui,
  Service,
  Session,
  Data,
  Input,
  Audio,
  Rendering,
  Physics,
  Ai,
  Narrative,
  Procgen,
  Unknown
};

struct FPath {
  TArray<FString> Segments;
};

struct FFieldSchema {
  FString Name;
  FString Kind;
  bool bRequired;
  FString Description;
};

struct FComponentSchema {
  ComponentType Type;
  DomainPathKey Domain;
  TArray<FFieldSchema> Fields;
  TArray<Tag> Tags;
  FString Description;
};

struct FCapabilitySpec {
  FString Name;
  DomainPathKey Domain;
  TArray<ComponentType> Reads;
  TArray<ComponentType> Writes;
  TArray<ResourceName> Resources;
  TArray<EventType> Emits;
};

struct FSystemSpec {
  SystemName Name;
  DomainPathKey Domain;
  TArray<ComponentType> RequiredComponents;
  TArray<Tag> RequiredTags;
  TArray<ResourceName> Resources;
  TArray<EventType> Emits;
};

struct FResourceSpec {
  ResourceName Name;
  DomainPathKey Domain;
  FString Kind;
  FString Description;
};

struct FEventSpec {
  EventType Type;
  DomainPathKey Domain;
  TArray<FFieldSchema> Fields;
  FString Description;
};

struct FNode {
  FPath Path;
  EKind Kind;
  TArray<DomainPathKey> Children;
  TArray<ComponentType> ComponentSchemas;
  TArray<FString> Capabilities;
  TArray<SystemName> Systems;
  TArray<ResourceName> Resources;
  TArray<EventType> Events;
};

struct FGraph {
  TMap<DomainPathKey, FNode> Nodes;
  TMap<ComponentType, FComponentSchema> ComponentSchemas;
  TMap<FString, FCapabilitySpec> Capabilities;
  TMap<SystemName, FSystemSpec> Systems;
  TMap<ResourceName, FResourceSpec> ResourceSpecs;
  TMap<EventType, FEventSpec> EventSpecs;
};

struct FCreateDomainNodeRequest {
  FPath Path;
  EKind Kind = EKind::Unknown;
};

struct FCreateFieldSchemaRequest {
  FString Name;
  FString Kind;
  bool bRequired = false;
  FString Description;
};

struct FCreateComponentSchemaRequest {
  ComponentType Type;
  DomainPathKey Domain;
  TArray<FFieldSchema> Fields;
  TArray<Tag> Tags;
  FString Description;
};

struct FCreateCapabilitySpecRequest {
  FString Name;
  DomainPathKey Domain;
  TArray<ComponentType> Reads;
  TArray<ComponentType> Writes;
  TArray<ResourceName> Resources;
  TArray<EventType> Emits;
};

struct FCreateSystemSpecRequest {
  SystemName Name;
  DomainPathKey Domain;
  TArray<ComponentType> Components;
  TArray<Tag> Tags;
  TArray<ResourceName> Resources;
  TArray<EventType> Emits;
};

struct FCreateResourceSpecRequest {
  ResourceName Name;
  DomainPathKey Domain;
  FString Kind;
  FString Description;
};

struct FCreateEventSpecRequest {
  EventType Type;
  DomainPathKey Domain;
  TArray<FFieldSchema> Fields;
  FString Description;
};

struct FRegisterDomainRequest {
  FGraph Registry;
  FNode Node;
};

struct FRegisterDomainPathRequest {
  FGraph Registry;
  TArray<FString> Segments;
  EKind Kind = EKind::Unknown;
};

struct FPathRegistration {
  TArray<FString> Segments;
  EKind Kind = EKind::Unknown;

  FPathRegistration() {}

  FPathRegistration(std::initializer_list<const char *> InSegments,
                          EKind InKind)
      : Kind(InKind) {
    const TArray<const char *> SegmentAtoms(InSegments);
    Segments = func::map_array<const char *, FString>(
        SegmentAtoms, [](const char *Segment) {
          return FString(UTF8_TO_TCHAR(Segment));
        });
  }
};

struct FRegisterDomainPathsRequest {
  FGraph Registry;
  TArray<FPathRegistration> Registrations;
};

struct FRegisterComponentSchemaRequest {
  FGraph Registry;
  FComponentSchema Schema;
};

struct FRegisterCapabilitySpecRequest {
  FGraph Registry;
  FCapabilitySpec Spec;
};

struct FRegisterSystemSpecRequest {
  FGraph Registry;
  FSystemSpec Spec;
};

struct FRegisterResourceSpecRequest {
  FGraph Registry;
  FResourceSpec Spec;
};

struct FRegisterEventSpecRequest {
  FGraph Registry;
  FEventSpec Spec;
};

/**
 * @brief Creates a neutral ECS domain path from ordered path segments.
 * @signature inline FPath createDomainPath(const TArray<FString> &Segments)
 *
 * User Story: As a feature author, I need stable domain paths so higher
 * gameplay domains can import downward into shared ECS primitives.
 */

} // namespace ecs
