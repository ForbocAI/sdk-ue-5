#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "Core/FP/Either/Either.hpp"

namespace func {
/**
 * @brief 19. Dispatcher (Dictionary-Based Typed Dispatch) A lookup table mapping keys to handler functions. Returns Maybe<Result> from dispatch — just(handler()) if the key exists, nothing<Result>() if not. Construction: use createDispatcher<Key, Result>() with a vector of {key, handler} pairs. Dispatch:     use the dispatch() free function. Usage: auto d = func::createDispatcher<FString, int>({ {TEXT("a"), []() { return 1; }}, {TEXT("b"), []() { return 2; }}, }); auto result = func::dispatch(d, TEXT("a")); // just(1) auto miss   = func::dispatch(d, TEXT("z")); // nothing
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename Key, typename Result> struct Dispatcher
 *
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */

template <typename Key, typename Result> struct Dispatcher {
  std::unordered_map<Key, std::function<Result()>> table;
};

namespace detail {
/** User Story: As a core fp dispatcher consumer, I need to invoke create dispatcher recursive through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn template <typename Key, typename Result> Dispatcher<Key, Result> createDispatcherRecursive( const std::vector<std::pair<Key, std::function<Result()>>> &Entries, size_t Index, Dispatcher<Key, Result> Current) */
template <typename Key, typename Result>
Dispatcher<Key, Result>
createDispatcherRecursive(
    const std::vector<std::pair<Key, std::function<Result()>>> &Entries,
    size_t Index, Dispatcher<Key, Result> Current) {
  return Index == Entries.size()
             ? Current
             : (Current.table[Entries[Index].first] = Entries[Index].second,
                createDispatcherRecursive<Key, Result>(Entries, Index + 1,
                                                       std::move(Current)));
}

/** User Story: As a core fp dispatcher consumer, I need to invoke dispatcher keys recursive through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn template <typename Key, typename Result> std::vector<Key> dispatcherKeysRecursive( typename std::unordered_map<Key, std::function<Result()>>::const_iterator It, typename std::unordered_map<Key, std::function<Result()>>::const_iterator End, std::vector<Key> Current) */
template <typename Key, typename Result>
std::vector<Key> dispatcherKeysRecursive(
    typename std::unordered_map<Key, std::function<Result()>>::const_iterator It,
    typename std::unordered_map<Key, std::function<Result()>>::const_iterator End,
    std::vector<Key> Current) {
  return It == End
             ? Current
             : (Current.push_back(It->first),
                dispatcherKeysRecursive<Key, Result>(++It, End,
                                                     std::move(Current)));
}
} // namespace detail

/**
 * @fn template <typename Key, typename Result> Dispatcher<Key, Result> createDispatcher( std::vector<std::pair<Key, std::function<Result()>>> entries)
 * @brief Builds a dispatcher table from key-to-handler entries.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As keyed dispatch flows, I need a typed dispatcher table so
 * string or enum keys can resolve handlers declaratively.
 */
template <typename Key, typename Result>
Dispatcher<Key, Result> createDispatcher(
    std::vector<std::pair<Key, std::function<Result()>>> entries) {
  return detail::createDispatcherRecursive<Key, Result>(entries, 0,
                                                        Dispatcher<Key, Result>{});
}

/**
 * @fn template <typename Key, typename Result> Maybe<Result> dispatch(const Dispatcher<Key, Result> &d, const Key &key)
 * @brief Looks up and invokes a handler by key when one exists.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As keyed dispatch flows, I need dispatch to return Maybe so
 * missing handlers do not require exceptions or sentinels.
 */
template <typename Key, typename Result>
Maybe<Result> dispatch(const Dispatcher<Key, Result> &d, const Key &key) {
  typename std::unordered_map<Key, std::function<Result()>>::const_iterator it =
      d.table.find(key);
  return it != d.table.end() ? just(it->second()) : nothing<Result>();
}

/**
 * @fn template <typename E, typename Key, typename Result> Either<E, Result> dispatch_either(const Dispatcher<Key, Result> &d, const Key &key, E error)
 * @brief Looks up and invokes a handler or returns a typed error.
 *
 *
 * User Story: As reducer and ECS code, I need strict dispatch misses to remain
 * explicit errors instead of falling through to hidden defaults.
 */
template <typename E, typename Key, typename Result>
Either<E, Result> dispatch_either(const Dispatcher<Key, Result> &d,
                                  const Key &key, E error) {
  Maybe<Result> result = dispatch(d, key);
  return result.hasValue ? make_right<E, Result>(result.value)
                         : make_left<E, Result>(error);
}

/**
 * @fn template <typename Key, typename Result> bool has(const Dispatcher<Key, Result> &d, const Key &key)
 * @brief Reports whether a dispatcher has a handler for the given key.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As keyed dispatch flows, I need a presence check so callers can
 * branch before invoking optional handlers.
 */
template <typename Key, typename Result>
bool has(const Dispatcher<Key, Result> &d, const Key &key) {
  return d.table.find(key) != d.table.end();
}

/**
 * @fn template <typename Key, typename Result> std::vector<Key> keys(const Dispatcher<Key, Result> &d)
 * @brief Returns every key currently registered in the dispatcher.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As keyed dispatch flows, I need access to registered keys so
 * tools and tests can inspect available handlers.
 */
template <typename Key, typename Result>
std::vector<Key> keys(const Dispatcher<Key, Result> &d) {
  return detail::dispatcherKeysRecursive<Key, Result>(d.table.begin(),
                                                      d.table.end(),
                                                      std::vector<Key>());
}

/**
 * @brief Strict keyed dispatcher for handlers that consume one argument.
 *
 * @signature template <typename Key, typename Arg, typename Result> struct ArgDispatcher
 *
 * User Story: As ECS and adapter code, I need keyed argument handlers without
 * substitute behavior so absent handlers return Maybe/Either at the boundary.
 */
template <typename Key, typename Arg, typename Result>
struct ArgDispatcher {
  std::unordered_map<Key, std::function<Result(const Arg &)>> table;
};

/**
 * @brief Request payload for strict argument dispatch.
 *
 * @signature template <typename Key, typename Arg, typename Result> struct ArgDispatcherDispatch
 *
 * User Story: As function-composition code, I need multi-input dispatch calls
 * carried through one payload while the dispatcher itself remains reusable.
 */
template <typename Key, typename Arg, typename Result>
struct ArgDispatcherDispatch {
  const ArgDispatcher<Key, Arg, Result> *dispatcher;
  const Key *key;
  const Arg *arg;
};

/**
 * @fn template <typename Key, typename Arg, typename Result> ArgDispatcher<Key, Arg, Result> create_arg_dispatcher()
 * @brief Creates an empty strict argument dispatcher.
 *
 *
 * User Story: As keyed dispatch setup code, I need an empty value that can be
 * extended through registration functions and lazy cached when needed.
 */
template <typename Key, typename Arg, typename Result>
ArgDispatcher<Key, Arg, Result> create_arg_dispatcher() {
  return ArgDispatcher<Key, Arg, Result>();
}

/**
 * @fn template <typename Key, typename Arg, typename Result> ArgDispatcher<Key, Arg, Result> arg_dispatcher_register( ArgDispatcher<Key, Arg, Result> dispatcher, Key key, std::function<Result(const Arg &)> handler)
 * @brief Registers a key-to-argument-handler entry.
 *
 *
 * User Story: As formatter and adapter code, I need registration to return the
 * next dispatcher value so tables compose through pipe/lazy helpers.
 */
template <typename Key, typename Arg, typename Result>
ArgDispatcher<Key, Arg, Result> arg_dispatcher_register(
    ArgDispatcher<Key, Arg, Result> dispatcher, Key key,
    std::function<Result(const Arg &)> handler) {
  dispatcher.table[std::move(key)] = std::move(handler);
  return dispatcher;
}

/**
 * @fn template <typename Key, typename Arg, typename Result> Maybe<Result> arg_dispatcher_dispatch_maybe( const ArgDispatcherDispatch<Key, Arg, Result> &request)
 * @brief Looks up and invokes an argument handler when one exists.
 *
 *
 * User Story: As ECS code, I need formatter and routing misses to be explicit
 * Maybe values instead of implicit substitute paths.
 */
template <typename Key, typename Arg, typename Result>
Maybe<Result> arg_dispatcher_dispatch_maybe(
    const ArgDispatcherDispatch<Key, Arg, Result> &request) {
  typename std::unordered_map<Key, std::function<Result(const Arg &)>>::
      const_iterator It = request.dispatcher->table.find(*request.key);
  return It != request.dispatcher->table.end() ? just(It->second(*request.arg))
                                               : nothing<Result>();
}

/**
 * @fn template <typename E, typename Key, typename Arg, typename Result> Either<E, Result> arg_dispatcher_dispatch_either( const ArgDispatcherDispatch<Key, Arg, Result> &request, E error)
 * @brief Looks up and invokes an argument handler or returns a typed error.
 *
 *
 * User Story: As reducer and ECS code, I need strict dispatch to return a typed
 * error when a table misses instead of silently choosing a default branch.
 */
template <typename E, typename Key, typename Arg, typename Result>
Either<E, Result> arg_dispatcher_dispatch_either(
    const ArgDispatcherDispatch<Key, Arg, Result> &request, E error) {
  Maybe<Result> result = arg_dispatcher_dispatch_maybe(request);
  return result.hasValue ? make_right<E, Result>(result.value)
                         : make_left<E, Result>(error);
}

} // namespace func
