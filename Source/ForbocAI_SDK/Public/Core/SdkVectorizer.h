#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace SDK {
namespace Vectorizer {

const int32 SDK_VECTOR_DIMENSION = 384;
const uint32 HASH_SEED = 2166136261u;
const uint32 HASH_PRIME = 16777619u;

inline uint32 MixHash(uint32 Hash, uint32 Value) {
  return (Hash ^ Value) * HASH_PRIME;
}

inline uint32 HashTextRecursive(const FString &Text, int32 Index, uint32 Hash) {
  return Index >= Text.Len()
             ? Hash
             : HashTextRecursive(Text, Index + 1,
                                 MixHash(Hash, static_cast<uint32>(Text[Index])));
}

inline TArray<float> AddFeature(const TArray<float> &Vector, uint32 Hash,
                                float Weight) {
  TArray<float> Next = Vector;
  const int32 Bucket = static_cast<int32>(Hash % SDK_VECTOR_DIMENSION);
  const float Sign = (Hash & 1u) == 0u ? 1.0f : -1.0f;
  Next[Bucket] += Sign * Weight;
  return Next;
}

inline TArray<float> EmbedTextRecursive(const FString &Text, int32 Index,
                                        const TArray<float> &Vector) {
  return Index >= Text.Len()
             ? Vector
             : EmbedTextRecursive(
                   Text, Index + 1,
                   FChar::IsWhitespace(Text[Index])
                       ? Vector
                       : AddFeature(
                             Vector,
                             HashTextRecursive(
                                 FString::Printf(
                                     TEXT("%d:%d"), Index,
                                     static_cast<int32>(Text[Index])),
                                 0, HASH_SEED),
                             1.0f));
}

inline float SumSquaresRecursive(const TArray<float> &Vector, int32 Index,
                                 float Acc) {
  return Index >= Vector.Num()
             ? Acc
             : SumSquaresRecursive(Vector, Index + 1,
                                   Acc + Vector[Index] * Vector[Index]);
}

inline TArray<float> NormalizeRecursive(const TArray<float> &Vector,
                                        int32 Index, float Norm,
                                        const TArray<float> &Acc) {
  return Index >= Vector.Num()
             ? Acc
             : [&]() {
                 TArray<float> Next = Acc;
                 Next.Add(Norm > 0.0f ? Vector[Index] / Norm : 0.0f);
                 return NormalizeRecursive(Vector, Index + 1, Norm, Next);
               }();
}

inline TArray<float> Normalize(const TArray<float> &Vector) {
  const float Norm = FMath::Sqrt(SumSquaresRecursive(Vector, 0, 0.0f));
  return NormalizeRecursive(Vector, 0, Norm, TArray<float>());
}

inline TArray<float> EmptyVector() {
  TArray<float> Vector;
  Vector.Init(0.0f, SDK_VECTOR_DIMENSION);
  return Vector;
}

inline TArray<float> Embed(const FString &Text) {
  return Normalize(EmbedTextRecursive(Text.ToLower(), 0, EmptyVector()));
}

} // namespace Vectorizer
} // namespace SDK
} // namespace ForbocAI
