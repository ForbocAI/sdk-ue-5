#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 14. ConfigBuilder (Functional Configuration Builder) A data-first builder for creating immutable configuration objects using functional composition. Usage: auto builder = configBuilder<MyConfig>(); builder = setMember(builder, &MyConfig::name, std::string("MyApp")); builder = setMember(builder, &MyConfig::port, 8080); auto config = buildConfig(builder);
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename Config> struct ConfigBuilder
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <typename Config> struct ConfigBuilder {
  std::vector<std::function<void(Config &)>> Setters;
};

/**
 * @fn template <typename Config> ConfigBuilder<Config> configBuilder()
 * @brief Creates an empty functional configuration builder.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As config assembly flows, I need a builder entry point so
 * immutable config values can be constructed declaratively.
 */
template <typename Config> ConfigBuilder<Config> configBuilder() {
  return ConfigBuilder<Config>{{}};
}

/**
 * @fn template <typename Config, typename Func> ConfigBuilder<Config> with(ConfigBuilder<Config> Builder, Func Setter)
 * @brief Adds an explicit mutating transform to the eventual config value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As config assembly flows, I need queued setters so immutable
 * config objects can be built through composable transforms.
 */
template <typename Config, typename Func>
ConfigBuilder<Config> with(ConfigBuilder<Config> Builder, Func Setter) {
  std::function<void(Config &)> WrappedSetter = Setter;
  Builder.Setters.push_back(WrappedSetter);
  return Builder;
}

/**
 * @fn template <typename Config, typename T> ConfigBuilder<Config> setMember(ConfigBuilder<Config> Builder, T Config::*Member, T Value)
 * @brief Assigns a concrete member through a pointer-to-member.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As config assembly flows, I need member assignment helpers so
 * config values can be declared without repetitive boilerplate.
 */
template <typename Config, typename T>
ConfigBuilder<Config> setMember(ConfigBuilder<Config> Builder,
                                T Config::*Member, T Value) {
  return with(std::move(Builder),
              [Member, Value](Config &ConfigValue) mutable {
                ConfigValue.*Member = std::move(Value);
              });
}

/**
 * @fn template <typename Config, typename T> ConfigBuilder<Config> set(ConfigBuilder<Config> Builder, const std::string &Key, T Value)
 * @brief Delegates string-keyed assignment to config types that expose `set`.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As config assembly flows, I need key-based setters so dynamic
 * config types can participate in the same builder pattern.
 */
template <typename Config, typename T>
ConfigBuilder<Config> set(ConfigBuilder<Config> Builder, const std::string &Key,
                          T Value) {
  return with(std::move(Builder), [Key, Value](Config &ConfigValue) mutable {
    ConfigValue.set(Key, std::move(Value));
  });
}

/**
 * @brief Materializes the configured value by replaying all queued setters.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature namespace detail
 *
 * User Story: As config assembly flows, I need a final build step so queued
 * transforms can produce one immutable config value.
 */
namespace detail {
/** User Story: As a core fp config consumer, I need to invoke apply config setters recursive through a stable signature so the core fp config workflow remains explicit and composable. @fn template <typename Config> Config applyConfigSettersRecursive( const std::vector<std::function<void(Config &)>> &Setters, size_t Index, Config ConfigValue) */
template <typename Config>
Config applyConfigSettersRecursive(
    const std::vector<std::function<void(Config &)>> &Setters, size_t Index,
    Config ConfigValue) {
  return Index == Setters.size()
             ? ConfigValue
             : (Setters[Index](ConfigValue),
                applyConfigSettersRecursive(Setters, Index + 1,
                                            std::move(ConfigValue)));
}
} // namespace detail

/** User Story: As a core fp config consumer, I need to invoke build config through a stable signature so the core fp config workflow remains explicit and composable. @fn template <typename Config> Config buildConfig(const ConfigBuilder<Config> &Builder) */
template <typename Config> Config buildConfig(const ConfigBuilder<Config> &Builder) {
  return detail::applyConfigSettersRecursive(Builder.Setters, 0, Config{});
}

} // namespace func
