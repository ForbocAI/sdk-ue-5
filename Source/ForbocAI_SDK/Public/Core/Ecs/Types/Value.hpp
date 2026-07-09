#pragma once

namespace ecs {


enum class EComponentValueKind {
  None,
  Bool,
  Int,
  Float,
  Text,
  Vec2,
  Vec3,
  Map,
  List
};

struct FComponentValue {
  EComponentValueKind Kind;
  bool BoolValue;
  int64 IntValue;
  float FloatValue;
  FString TextValue;
  FVector2D Vec2Value;
  FVector Vec3Value;
  TMap<FString, TSharedPtr<FComponentValue>> MapValue;
  TArray<TSharedPtr<FComponentValue>> ListValue;
};

inline FComponentValue createNoneComponentValue() {
  FComponentValue Value;
  Value.Kind = EComponentValueKind::None;
  Value.BoolValue = false;
  Value.IntValue = int64{};
  Value.FloatValue = float{};
  return Value;
}

inline FComponentValue createBoolComponentValue(bool RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Bool;
  Value.BoolValue = RawValue;
  return Value;
}

inline FComponentValue createIntComponentValue(int64 RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Int;
  Value.IntValue = RawValue;
  return Value;
}

inline FComponentValue createFloatComponentValue(float RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Float;
  Value.FloatValue = RawValue;
  return Value;
}

inline FComponentValue createTextComponentValue(const FString &RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Text;
  Value.TextValue = RawValue;
  return Value;
}

inline FComponentValue createVec3ComponentValue(const FVector &RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Vec3;
  Value.Vec3Value = RawValue;
  return Value;
}

inline FComponentValue createVec2ComponentValue(const FVector2D &RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Vec2;
  Value.Vec2Value = RawValue;
  return Value;
}

inline FComponentValue
createMapComponentValue(const TMap<FString, FComponentValue> &RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::Map;
  const TArray<FString> Keys = func::map_keys<FString, FComponentValue>(RawValue);
  Value.MapValue =
      func::fold_array<FString, TMap<FString, TSharedPtr<FComponentValue>>>(
      Keys, TMap<FString, TSharedPtr<FComponentValue>>(),
      [&RawValue](const TMap<FString, TSharedPtr<FComponentValue>> &Acc,
                  const FString &Key) {
        return func::match(
            func::find_map_value_ptr<FString, FComponentValue>(RawValue, Key),
            [&Acc, &Key](const FComponentValue *Found) {
              TMap<FString, TSharedPtr<FComponentValue>> Next = Acc;
              Next.Add(Key, MakeShared<FComponentValue>(*Found));
              return Next;
            },
            [&Acc]() { return Acc; });
      });
  return Value;
}

inline FComponentValue createListComponentValue(const TArray<FComponentValue> &RawValue) {
  FComponentValue Value = createNoneComponentValue();
  Value.Kind = EComponentValueKind::List;
  Value.ListValue =
      func::fold_array<FComponentValue, TArray<TSharedPtr<FComponentValue>>>(
      RawValue, TArray<TSharedPtr<FComponentValue>>(),
      [](const TArray<TSharedPtr<FComponentValue>> &Acc,
         const FComponentValue &Entry) {
        TArray<TSharedPtr<FComponentValue>> Next = Acc;
        Next.Add(MakeShared<FComponentValue>(Entry));
        return Next;
      });
  return Value;
}

/**
 * @brief Maps source rows into ECS component values through the neutral array mapper.
 * @signature template <typename Source> inline TArray<FComponentValue> mapComponentValues(const TArray<Source> &Items, std::function<FComponentValue(const Source &)> MapValue)
 *
 * User Story: As projection adapters, component-list values should share one
 * ECS mapper instead of building request/factory families per source noun.
 */
template <typename Source>
inline TArray<FComponentValue>
mapComponentValues(const TArray<Source> &Items,
                   std::function<FComponentValue(const Source &)> MapValue) {
  return func::map_array<Source, FComponentValue>(Items, MapValue);
}

inline bool componentValuePtrEquals(const TSharedPtr<FComponentValue> &Left,
                                    const TSharedPtr<FComponentValue> &Right);

inline bool componentMapEquals(
    const TMap<FString, TSharedPtr<FComponentValue>> &Left,
    const TMap<FString, TSharedPtr<FComponentValue>> &Right) {
  return func::map_values_equal<FString, TSharedPtr<FComponentValue>>(
      Left, Right,
      [](const TSharedPtr<FComponentValue> &LeftValue,
         const TSharedPtr<FComponentValue> &RightValue) {
        return componentValuePtrEquals(LeftValue, RightValue);
      });
}

inline bool componentListEquals(
    const TArray<TSharedPtr<FComponentValue>> &Left,
    const TArray<TSharedPtr<FComponentValue>> &Right) {
  const TArray<int32> Indices = func::index_range(Left.Num());
  return Left.Num() == Right.Num() &&
         func::all_array<int32>(
             Indices,
             [&Left, &Right](const int32 &Index) {
               return componentValuePtrEquals(Left[Index], Right[Index]);
             });
}

inline bool operator==(const FComponentValue &Left,
                       const FComponentValue &Right) {
  return Left.Kind == Right.Kind && Left.BoolValue == Right.BoolValue &&
         Left.IntValue == Right.IntValue &&
         FMath::IsNearlyEqual(Left.FloatValue, Right.FloatValue) &&
         Left.TextValue == Right.TextValue &&
         Left.Vec2Value == Right.Vec2Value &&
         Left.Vec3Value == Right.Vec3Value &&
         componentMapEquals(Left.MapValue, Right.MapValue) &&
         componentListEquals(Left.ListValue, Right.ListValue);
}

inline bool operator!=(const FComponentValue &Left,
                       const FComponentValue &Right) {
  return !(Left == Right);
}

inline bool componentValuePtrEquals(const TSharedPtr<FComponentValue> &Left,
                                    const TSharedPtr<FComponentValue> &Right) {
  return (!Left.IsValid() && !Right.IsValid()) ||
         (Left.IsValid() && Right.IsValid() && *Left == *Right);
}

typedef TMap<EntityKey, FComponentValue> ComponentTable;
typedef TMap<ComponentType, ComponentTable> ComponentStore;
typedef TMap<EntityKey, TArray<Tag>> TagStore;
typedef TMap<ResourceName, FComponentValue> ResourceStore;
typedef TMap<EventType, TArray<FComponentValue>> EventQueue;

} // namespace ecs
