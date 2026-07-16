#include "Features/Memory/Vector/MemoryVectorAdapters.h"

#include "Core/fp.hpp"
#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "Internationalization/Regex.h"

namespace {

struct FSimilarityComponents {
  float Dot;
  float NormA;
  float NormB;
};

/** User Story: As SDK-owned vector memory, I need deterministic unsigned hash mixing so every supported runtime assigns features to the same buckets. @fn uint32 mixHash(uint32 Hash, uint32 Value) */
uint32 mixHash(uint32 Hash, uint32 Value) {
  return (Hash ^ Value) * MemoryConfiguration::memoryData().Vector.HashPrime;
}

/** User Story: As SDK-owned vector memory, I need text hashed recursively so embedding behavior remains pure and reproducible. @fn uint32 hashText(const FString &Text, int32 Index, uint32 Hash) */
uint32 hashText(const FString &Text, int32 Index, uint32 Hash) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Text.Len()
             ? Hash
             : hashText(Text, Index + Data.Iteration.Step,
                        mixHash(Hash, static_cast<uint32>(Text[Index])));
}

/** User Story: As SDK-owned vector memory, I need regex matches collected recursively so TS and UE segment recall text with the same authored grammar. @fn TArray<FString> collectTokens(FRegexMatcher &Matcher, const TArray<FString> &Tokens) */
TArray<FString> collectTokens(FRegexMatcher &Matcher,
                              const TArray<FString> &Tokens) {
  return !Matcher.FindNext()
             ? Tokens
             : collectTokens(
                   Matcher,
                   func::append_value<FString>(
                       Tokens, Matcher.GetCaptureGroup(
                                   MemoryConfiguration::memoryData()
                                       .Iteration.InitialIndex)));
}

/** User Story: As SDK-owned vector memory, I need an authored token grammar so TS and UE segment recall text identically. @fn TArray<FString> tokenize(const FString &Text) */
TArray<FString> tokenize(const FString &Text) {
  const FRegexPattern Pattern(
      MemoryConfiguration::memoryData().Vector.TokenPattern);
  FRegexMatcher Matcher(Pattern, Text.ToLower());
  return collectTokens(Matcher, TArray<FString>());
}

/** User Story: As SDK-owned vector memory, I need immutable signed feature updates so embedding construction stays functional. @fn TArray<float> addFeature(const TArray<float> &Vector, uint32 Hash, float Weight) */
TArray<float> addFeature(const TArray<float> &Vector, uint32 Hash,
                         float Weight) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  TArray<float> Next = Vector;
  const int32 Index = static_cast<int32>(
      Hash % static_cast<uint32>(Data.Vector.Dimension));
  const float Sign = (Hash & Data.Vector.SignMask) == Data.Vector.EvenRemainder
                         ? Data.Vector.PositiveWeight
                         : Data.Vector.NegativeWeight;
  Next[Index] += Sign * Weight;
  return Next;
}

/** User Story: As SDK-owned vector memory, I need ordered token features folded into a fixed vector so embeddings are stable across calls and platforms. @fn TArray<float> embedTokens(const TArray<FString> &Tokens, int32 Index, const TArray<float> &Vector) */
TArray<float> embedTokens(const TArray<FString> &Tokens, int32 Index,
                          const TArray<float> &Vector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Tokens.Num()
             ? Vector
             : embedTokens(
                   Tokens, Index + Data.Iteration.Step,
                   addFeature(
                       Vector,
                       hashText(FString::FromInt(Index) +
                                    Data.Vector.FeatureSeparator + Tokens[Index],
                                Data.Iteration.InitialIndex,
                                Data.Vector.HashSeed),
                       Data.Vector.PositiveWeight +
                           FMath::Loge(Data.Vector.PositiveWeight +
                                       static_cast<float>(Tokens[Index].Len()))));
}

/** User Story: As SDK-owned vector memory, I need vector magnitude derived recursively so normalization remains a pure transformation. @fn float sumSquares(const TArray<float> &Vector, int32 Index, float Sum) */
float sumSquares(const TArray<float> &Vector, int32 Index, float Sum) {
  return Index >= Vector.Num()
             ? Sum
             : sumSquares(
                   Vector,
                   Index + MemoryConfiguration::memoryData().Iteration.Step,
                   Sum + Vector[Index] * Vector[Index]);
}

/** User Story: As SDK-owned vector memory, I need L2-normalized features so cosine comparisons are consistent across storage engines. @fn TArray<float> normalize(const TArray<float> &Vector, int32 Index, float Norm, const TArray<float> &Normalized) */
TArray<float> normalize(const TArray<float> &Vector, int32 Index, float Norm,
                        const TArray<float> &Normalized) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Vector.Num()
             ? Normalized
             : normalize(
                   Vector, Index + Data.Iteration.Step, Norm,
                   func::append_value<float>(
                       Normalized,
                       Norm > Data.Defaults.Similarity
                           ? Vector[Index] / Norm
                           : Data.Defaults.Similarity));
}

/** User Story: As SDK-owned vector memory, I need vector allocation driven by authored dimensions so storage schemas and embeddings cannot drift. @fn TArray<float> emptyVector() */
TArray<float> emptyVector() {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  TArray<float> Vector;
  Vector.Init(Data.Defaults.Similarity, Data.Vector.Dimension);
  return Vector;
}

/**
 * User Story: As memory recall, I need cosine components accumulated functionally so every engine ranks records with one shared implementation.
 * @fn FSimilarityComponents computeSimilarityComponents( const TArray<float> &A, const TArray<float> &B, int32 Index, int32 Length, const FSimilarityComponents &Components)
 */
FSimilarityComponents computeSimilarityComponents(
    const TArray<float> &A, const TArray<float> &B, int32 Index, int32 Length,
    const FSimilarityComponents &Components) {
  return Index >= Length
             ? Components
             : computeSimilarityComponents(
                   A, B,
                   Index + MemoryConfiguration::memoryData().Iteration.Step,
                   Length,
                   {Components.Dot + A[Index] * B[Index],
                    Components.NormA + A[Index] * A[Index],
                    Components.NormB + B[Index] * B[Index]});
}

} // namespace

namespace MemoryVectorAdapters {

/** User Story: As SDK-owned memory, I need deterministic model-free embeddings so the API can own the SLM while each SDK owns vector memory. @fn TArray<float> embed(const FString &Text) */
TArray<float> embed(const FString &Text) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const TArray<float> Features = embedTokens(
      tokenize(Text), Data.Iteration.InitialIndex, emptyVector());
  const float Norm = FMath::Sqrt(sumSquares(
      Features, Data.Iteration.InitialIndex, Data.Defaults.Similarity));
  return normalize(Features, Data.Iteration.InitialIndex, Norm,
                   TArray<float>());
}

/** User Story: As memory recall, I need one cosine selector shared by all engines so ranking semantics cannot diverge. @fn float cosineSimilarity(const TArray<float> &A, const TArray<float> &B) */
float cosineSimilarity(const TArray<float> &A, const TArray<float> &B) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const int32 Length = FMath::Min(A.Num(), B.Num());
  return Length == Data.Iteration.InitialIndex
             ? Data.Defaults.Similarity
             : [&]() {
                 const FSimilarityComponents Components =
                     computeSimilarityComponents(
                         A, B, Data.Iteration.InitialIndex, Length,
                         {Data.Defaults.Similarity,
                          Data.Defaults.Similarity,
                          Data.Defaults.Similarity});
                 const float Denominator =
                     FMath::Sqrt(Components.NormA) *
                     FMath::Sqrt(Components.NormB);
                 return Denominator > Data.Defaults.Similarity
                            ? Components.Dot / Denominator
                            : Data.Defaults.Similarity;
               }();
}

} // namespace MemoryVectorAdapters
