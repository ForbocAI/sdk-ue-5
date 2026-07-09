#pragma once
#ifndef FORBOC_AI_GAME_CORE_ECS_HPP
#define FORBOC_AI_GAME_CORE_ECS_HPP

#include "CoreMinimal.h"
#include "Core/ue_fp.hpp"

#include <initializer_list>

/**
 * @file ecs.hpp
 * @brief Game-project ECS primitives that depend on FP only and never on RTK.
 *
 * User Stories:
 * - As a gameplay feature author, I need neutral Components / Entities /
 *   Systems taxonomy so Unreal features can share ECS vocabulary with
 *   Rust and GDScript game projects.
 * - As a reducer author, I need ECS worlds to be plain value structs so project
 *   files can compose ECS transitions with RTK state without ECS importing RTK.
 * - As a maintainer, I need this runtime header outside the SDK core because the
 *   SDK should not own game-project ECS primitives.
 *
 * Functional-core cookbook boundary:
 * - ECS helpers are pure value transforms over request structs.
 * - RTK slices/actions/selectors own store semantics and unidirectional data
 *   flow.
 * - ue_fp.hpp is used for composition, Maybe/Either, and lazy values;
 *   it must not model a replacement action, reducer, selector, or store layer.
 * - Neutral FP helpers for arrays, maps, predicates, traversal, lookup,
 *   validation, and catalogs belong in ue_fp.hpp. Do not re-declare those
 *   generic combinators in ecs.hpp; ECS should import them and only add
 *   world/entity/component/system semantics.
 * - When only nouns change, keep the nouns in catalogs and fold through one
 *   ECS transform. Paired catalogs cover selector/projector and descriptor/run
 *   relationships without feature-local recursive families.
 * - Do not replace every `if` with ternary chains. Domain alternatives should
 *   use `func::match` for Maybe/Either/result values, dispatcher or
 *   `multi_match` case tables for enum/string routing, and FP-core
 *   map/filter/fold/find/traverse helpers for collection decisions. Explicit
 *   guards are reserved for UE/effect boundaries.
 */
#include "Core/Ecs/Types/Entity.hpp"
#include "Core/Ecs/Types/Value.hpp"
#include "Core/Ecs/Domain/Types.hpp"
#include "Core/Ecs/Domain/Factory.hpp"
#include "Core/Ecs/Domain/Registry.hpp"
#include "Core/Ecs/World/Types.hpp"
#include "Core/Ecs/World/Factory.hpp"
#include "Core/Ecs/World/Relationship.hpp"
#include "Core/Ecs/World/Entity.hpp"
#include "Core/Ecs/World/Query.hpp"
#include "Core/Ecs/System/Types.hpp"
#include "Core/Ecs/System/Execution.hpp"
#include "Core/Ecs/Inspect/Types.hpp"
#include "Core/Ecs/Inspect/Selectors.hpp"

#endif // FORBOC_AI_GAME_CORE_ECS_HPP
