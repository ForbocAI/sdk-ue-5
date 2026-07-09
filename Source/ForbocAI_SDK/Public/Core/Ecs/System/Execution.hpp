#pragma once

namespace ecs {


inline FString createTextAtom(const char *Atom) {
  return FString(UTF8_TO_TCHAR(Atom));
}

struct FComponentValueFormatCase {
  EComponentValueKind Kind;
  std::function<FString(const FComponentValue &)> Format;
};

/**
 * @brief Converts a component value kind into a dispatcher key.
 * @signature inline int32 componentValueKindKey(EComponentValueKind Kind)
 *
 * User Story: As ECS inspection code, enum formatting should use a stable
 * dispatcher key that composes with ue_fp dispatch helpers.
 */
inline int32 componentValueKindKey(EComponentValueKind Kind) {
  return static_cast<int32>(Kind);
}

/**
 * @brief Creates a reusable formatter registration function.
 * @signature inline std::function<FComponentValueFormatter(FComponentValueFormatter)> registerComponentValueFormatter(EComponentValueKind Kind, std::function<FString(const FComponentValue &)> Format)
 *
 * User Story: As ECS inspection code, component formatter registration should
 * be a reusable function factory for pipeline composition.
 */
inline std::function<FComponentValueFormatter(FComponentValueFormatter)>
registerComponentValueFormatter(
    EComponentValueKind Kind,
    std::function<FString(const FComponentValue &)> Format) {
  return [Kind, Format](FComponentValueFormatter Dispatcher) {
    return func::arg_dispatcher_register<int32, FComponentValue, FString>(
        Dispatcher, componentValueKindKey(Kind), Format);
  };
}

/**
 * @brief Builds the formatter dispatcher from registered case declarations.
 */
inline FComponentValueFormatter createComponentValueFormatter(
    std::initializer_list<FComponentValueFormatCase> Cases) {
  const TArray<FComponentValueFormatCase> Declarations(Cases);
  return func::fold_array<FComponentValueFormatCase, FComponentValueFormatter>(
      Declarations,
      func::create_arg_dispatcher<int32, FComponentValue, FString>(),
      [](const FComponentValueFormatter &Dispatcher,
         const FComponentValueFormatCase &Case) {
        return registerComponentValueFormatter(Case.Kind,
                                               Case.Format)(Dispatcher);
      });
}

/**
 * @brief Builds the component value formatter through functional composition.
 * @signature inline FComponentValueFormatter createComponentValueFormatter()
 *
 * User Story: As ECS inspection code, component formatting should be a
 * dispatcher assembled from reusable registrations instead of a branch ladder.
 */
inline FComponentValueFormatter createComponentValueFormatter() {
  return createComponentValueFormatter(
      {{EComponentValueKind::None,
        [](const FComponentValue &) { return createTextAtom("None"); }},
       {EComponentValueKind::Bool,
        [](const FComponentValue &Value) {
          return Value.BoolValue ? createTextAtom("true")
                                 : createTextAtom("false");
       }},
       {EComponentValueKind::Int,
        [](const FComponentValue &Value) {
          return FString::Printf(TEXT("%lld"), Value.IntValue);
        }},
       {EComponentValueKind::Float,
        [](const FComponentValue &Value) {
          return FString::SanitizeFloat(Value.FloatValue);
        }},
       {EComponentValueKind::Text,
        [](const FComponentValue &Value) { return Value.TextValue; }},
       {EComponentValueKind::Vec2,
        [](const FComponentValue &Value) {
          return Value.Vec2Value.ToString();
        }},
       {EComponentValueKind::Vec3,
        [](const FComponentValue &Value) {
          return Value.Vec3Value.ToString();
        }},
       {EComponentValueKind::Map,
        [](const FComponentValue &Value) {
          return FString::Printf(TEXT("Map(len=%d)"), Value.MapValue.Num());
        }},
       {EComponentValueKind::List,
        [](const FComponentValue &Value) {
          return FString::Printf(TEXT("List(len=%d)"),
                                 Value.ListValue.Num());
        }}});
}

/**
 * @brief Returns the lazily initialized component value formatter.
 * @signature inline const FComponentValueFormatter &componentValueFormatter()
 *
 * User Story: As ECS inspection code, formatter registration should be built
 * once and reused by every inspection call.
 */
inline const FComponentValueFormatter &componentValueFormatter() {
  static const func::Lazy<FComponentValueFormatter> Formatter =
      func::lazy([]() { return createComponentValueFormatter(); });
  return func::eval(Formatter);
}

/**
 * @brief Formats a component value for ECS inspection output.
 * @signature inline FString componentValueToString(const FComponentValue &Value)
 *
 * User Story: As ECS inspection code, component formatting should dispatch
 * through a reusable functional table.
 */
inline FString componentValueToString(const FComponentValue &Value) {
  const FComponentValueFormatter &Formatter = componentValueFormatter();
  const int32 Key = componentValueKindKey(Value.Kind);
  const func::Maybe<FString> Formatted =
      func::arg_dispatcher_dispatch_maybe<int32, FComponentValue, FString>(
      {&Formatter, &Key, &Value});
  checkf(Formatted.hasValue, TEXT("Unhandled ECS component value kind: %d"),
         Key);
  return Formatted.value;
}

struct FWorldStorageInspection {
  int32 EntityCount;
  int32 ComponentTypeCount;
  int32 ResourceCount;
  int32 EventTypeCount;
};

struct FWorldRuntimeInspection {
  int32 DirtyEntityCount;
  int32 DomainCount;
  int64 Generation;
};

struct FWorldInspection {
  FWorldStorageInspection Storage;
  FWorldRuntimeInspection Runtime;

} // namespace ecs
