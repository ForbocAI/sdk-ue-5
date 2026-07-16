#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 9. CALLABLE: Composed (Function Composition) Combines two functions: compose(f, g)(x) == f(g(x)) Construction: use the compose() factory function. operator() is the C++ mechanism for callable types. Usage: auto double_it = [](int x) { return x * 2; }; auto add_one   = [](int x) { return x + 1; }; auto both      = func::compose(add_one, double_it); int result = both(5);  // add_one(double_it(5)) = 11
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename F, typename G> struct Composed
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename F, typename G> struct Composed {
  F f;
  G g;

  /** User Story: As a core fp composition consumer, I need to invoke the callable value through a stable signature so the core fp composition workflow remains explicit and composable. @fn template <typename... Args> auto operator()(Args &&...args) const -> decltype(f(g(std::forward<Args>(args)...))) */
  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(f(g(std::forward<Args>(args)...))) {
    return f(g(std::forward<Args>(args)...));
  }
};

/**
 * @fn template <typename F, typename G> Composed<F, G> compose(F f, G g)
 * @brief Composes two functions so the result of `g` feeds into `f`.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional composition code, I need reusable composition so
 * runtime transforms can be assembled declaratively.
 */
template <typename F, typename G> Composed<F, G> compose(F f, G g) {
  return Composed<F, G>{f, g};
}


/**
 * @brief Unary-composition cookbook: neutral point-free helpers built on the
 * existing FP core so feature code can compose small reusable functions
 * instead of inventing domain-shaped substitutes for RTK or ECS.
 *
 * @signature template helpers: identity, constant, flip, complement, both,
 * either_pred, all_pass, any_pass, converge2, juxt2, pipe3, pipe4,
 * catalog, fold_catalog, zip_catalog_fold, tail_tuple, fold,
 * filter, find_vector, contains_value, unique_by, traverse,
 * sequence_maybe, fold_either, lift2, lift3, eq_by, has_key
 *
 * User Story: As a cross-SDK maintainer, I need the same unary-composition
 * recipes available in C++ as Rust, GDScript, and TypeScript so reducers,
 * selectors, and ECS systems can share examples and semantics.
 *
 * Cookbook rules:
 * - Functional core supplies pure composition, lazy values, Maybe/Either,
 *   predicate builders, collection transforms, and strict dispatch helpers.
 * - RTK owns the single store, event-style actions, reducers, selectors,
 *   thunks, adapters, and unidirectional data flow.
 * - ECS owns entity/component/system/world value transitions. ECS may use
 *   functional helpers, but functional helpers must not become a parallel
 *   action/store/reducer model.
 * - Views dispatch actions and read selectors. They do not own reducer logic,
 *   ECS query decisions, component matching, or derived domain state.
 * - Prefer reusable factories that return unary functions:
 *     auto HasTag = requireTag(World, Tag);
 *     auto InDomain = requireDomain(World, Domain);
 *     auto Matches = func::all_pass<EntityKey>({HasTag, InDomain});
 * - Do not stop at a repeated family of typed factories. If call sites still
 *   repeat `BindBool<Domain>(Name, Reader)`, `BindText<Domain>(Name, Reader)`,
 *   `ComponentField(Name, Member)`, `field_names(...)` plus
 *   `field_readers(...)`, or equivalent noun/type plumbing, push the
 *   abstraction down another level. The lower layer should compose functions
 *   into functions:
 *     domain declaration -> registry lookup -> value transform -> fold
 *   so higher domains provide only declaration data and the minimal
 *   domain-specific facts. The target is a composed unary transform built from
 *   smaller composed unary transforms, not a nicer-looking wrapper around the
 *   same repeated call shape.
 *
 *   Definitive generic style: name the composition boundary once, then pass a
 *   collection of irreducible domain atoms to one reusable composer. The source
 *   type, reducer slice, validator target, or dispatch target should come from
 *   the surrounding generic composer/context. Do not invent noun wrappers such
 *   as `FTownspersonSeeds(...)` just to forward a field list.
 *
 *   📣 MEGAPHONE RULE: a generic helper repeated once per field/member is still
 *   the wrong shape. Do not write `SettingField(...)`, `Required(...)`,
 *   `ComponentField(...)`, `BindFloat(...)`, or equivalent per-field wrapper
 *   calls at feature call sites. Settings, parser, projection, validation, and
 *   formatter code should pass grouped declaration atoms such as
 *   `ReadSettingsFields<FPlayerPresentationSettings>(
 *       Object, {"CapsuleRadius", "CapsuleHalfHeight", ...})`.
 *   The reusable composer owns `TEXT(...)`, snake_case/path conversion,
 *   typed reader selection, validation, traversal, and fold expansion.
 *   Digit-suffixed helper or macro families such as `READ_FIELD_1`,
 *   `READ_FIELD_2`, `FIELDS_32`, or `GET_FIELD_READER_10` are not
 *   compositional; they are arity plumbing hidden in names. Use one named
 *   composition boundary plus grouped declaration data instead.
 *
 *   This rule applies at every layer, including registries. A registry,
 *   adapter, validator, formatter, selector, reducer helper, or execution
 *   runner is not permission to hand-write
 *   `compose(ReadMember(A), ReadMember(B))` chains. Source/target lookup must
 *   also be declaration-shaped: use atoms, paths, conversions, predicates, and
 *   cases as data, then let one generic composer expand those declarations
 *   into readers, projectors, validators, dispatchers, reducers, or folds.
 *
 *   Definitive grouped declaration shape: the outer function owns the inner
 *   expansion. A call site supplies grouped atoms and paths, not nested helper
 *   calls:
 *     RegisteredGroups({
 *         {"Runtime/Bots",
 *          {"HasActiveGoal",
 *           "ActiveGoal",
 *           "GoalQueue",
 *           {"KnownLandmarkIds", {"Knowledge", "KnownLandmarkIds"}},
 *           {"KnownBotIds", {"Knowledge", "KnownBotIds"}}}},
 *         {"Runtime/Spatial", {"LocalLocation", "WorldLocation"}}});
 *
 *   A narrower boundary can accept only field/path atoms when the surrounding
 *   composer already knows the group:
 *     RegisteredFields({
 *         {"KnownLandmarkIds", {"Knowledge", "KnownLandmarkIds"}},
 *         {"KnownBotIds", {"Knowledge", "KnownBotIds"}}});
 *
 *   Converted values follow the same rule: declare the source path and the
 *   conversion function as data, then let the generic composer build the unary
 *   transform. This applies to component projection, validation, formatting,
 *   selector derivation, reducer payload shaping, and execution pipelines:
 *     RegisteredFields({
 *         "BehaviorState", {"BehaviorState"}, Convert(BotBehaviorText),
 *         "KnownBotIds", {"Knowledge", "KnownBotIds"}});
 *
 *   Domain grouping is still generic declaration data. The boundary function
 *   owns the inner helper expansion; do not nest `RegisteredFields`,
 *   `RegisteredRules`, `Required`, `Sequence`, or similar helper calls inside
 *   the declaration unless the argument is genuinely custom/one-off behavior:
 *     RegisteredDomain(
 *         "Runtime/Bots",
 *         {"Persona",
 *          "InteractionPrompt",
 *          "DefaultPlayerLine",
 *          "PinnedResponse",
 *          "InteractionIntent"});
 *
 *     RegisteredDomain(
 *         "Validation/Bots",
 *         {"Id", "Persona", "Goals"},
 *         {"Type", "Priority", "TargetEntityId"});
 *
 *     RegisteredCases({"Entity", "Component", "Relationship", "System"});
 *     RegisteredProjectionPasses({"Terrain", "Spawn", "Townsperson"});
 *
 *   Generic declaration helpers own platform string conversion. Do not repeat
 *   `TEXT(...)` around every atom and do not hide that repetition behind a
 *   noun wrapper. Do not replace field/path declarations with per-field
 *   `ReadMember`, `ComponentField`, `BindText`, or hand-composed reader
 *   chains.
 *
 *   The same rule applies to every repeated composition idiom, not only member
 *   reading: path assembly, entity selection, source selection, value
 *   conversion, binding construction, validation, formatting, lookup,
 *   traversal, and execution folds. If a call site repeats
 *   `ComposeX(Names, Readers)`, `ProjectPayloadY(Entity, Source, Fields)`, or
 *   matching lambda chains across domains, that is still an abstraction leak.
 *   Move the repeated shape into a higher composer and leave the current layer
 *   with grouped declarations. If the current layer is a registry, it still
 *   declares fields/paths/conversions as data.
 *
 *   Rule of thumb: if a feature file repeats a function call per field, per
 *   type, per domain prefix, per selector/projector pair, per validation rule,
 *   per formatting case, or per execution pass, it is not abstracted far
 *   enough. Move that shape into one composer and feed it grouped declaration
 *   data. Repetition is acceptable only for irreducible domain atoms inside
 *   those declarations.
 * - Use request/payload structs at public domain boundaries and for genuine
 *   multi-input domain operations. Do not create a new request struct when a
 *   reusable unary function, predicate, fold, lift, or traversal is the actual
 *   abstraction.
 * - When only nouns change, put selectors, projectors, validators, or
 *   transforms in registries/catalogs and fold those with generic declaration
 *   functions. Do not create `FNounThing(...)` wrapper families that only
 *   forward names into the same composer. When two noun lists advance
 *   together, use zip_catalog_fold so recursion is one primitive instead of a
 *   family of domain-shaped wrappers.
 * - Neutral primitives live below feature domains. Feature domains import
 *   downward into these primitives instead of borrowing helpers from siblings.
 * - Hidden substitute behavior belongs at explicit integration boundaries only.
 *   Reducers, selectors, and ECS systems should prefer Maybe/Either-returning
 *   helpers so missing data is visible in the type.
 */
} // namespace func
