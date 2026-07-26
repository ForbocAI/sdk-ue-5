#pragma once

#include "Misc/DateTime.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"
#include "MicroGame/Features/Systems/Quality/Command/QualityCommandAdapters.h"
#include "MicroGame/Features/Systems/Quality/Metadata/MetadataAdapters.h"
#include "MicroGame/Features/Systems/Quality/QualityActions.h"
#include "MicroGame/Features/Systems/Quality/Scoring/ScoringAdapters.h"

namespace MicroGame {

struct FQualityRunDependencies {
  FString Host;
  func::Maybe<FQualityReport> Baseline;
  TFunction<CommandRunner::FCommandOutput(const FCommandSpec &)> ExecuteCommand;
  TFunction<void(const FCommandSpec &,
                 const CommandRunner::FCommandOutput &)>
      OnCommandCompleted;
};

namespace QualityThunksDetail {

/**
 * User Story: As a live game player, I need quality commands presented through the same lifecycle as canonical CLI commands.
 * @fn inline void completeCommand( const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result, const FQualityRunDependencies &Dependencies)
 */
inline void completeCommand(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result,
    const FQualityRunDependencies &Dependencies) {
  Dependencies.OnCommandCompleted
      ? (Dependencies.OnCommandCompleted(Command, Result), void())
      : void();
}

/**
 * User Story: As a quality evaluator, I need every probe executed once through the injected public CLI boundary.
 * @fn inline CommandRunner::FCommandOutput executeQualityCommand( const FCommandSpec &Command, const FQualityRunDependencies &Dependencies)
 */
inline CommandRunner::FCommandOutput executeQualityCommand(
    const FCommandSpec &Command,
    const FQualityRunDependencies &Dependencies) {
  const CommandRunner::FCommandOutput Result =
      Dependencies.ExecuteCommand(Command);
  completeCommand(Command, Result, Dependencies);
  return Result;
}

/** User Story: As a release reviewer, I need setup failures terminate the quality run explicitly without creating passing empty metrics. @fn inline void failEvaluation(FMicroGameStore &Store, const FString &Error) */
inline void failEvaluation(FMicroGameStore &Store, const FString &Error) {
  Store.dispatch(QualityActions::evaluationFailed(
      {FDateTime::UtcNow().ToIso8601(), Error}));
}

/**
 * User Story: As a generic NPC evaluator, I need neutral persona and memory setup sequenced before dialogue probes.
 * @fn inline func::Maybe<FQualitySetupFailure> runSetupCommands( const TArray<FCommandSpec> &Commands, const FQualityRunDependencies &Dependencies, int32 Index)
 */
inline func::Maybe<FQualitySetupFailure> runSetupCommands(
    const TArray<FCommandSpec> &Commands,
    const FQualityRunDependencies &Dependencies, int32 Index) {
  return Index >= Commands.Num()
             ? func::nothing<FQualitySetupFailure>()
             : [&]() {
                 const CommandRunner::FCommandOutput Result =
                     executeQualityCommand(Commands[Index], Dependencies);
                 return Result.Status != qualityData().CommandStatuses.Ok
                            ? func::just(
                                  FQualitySetupFailure{Commands[Index], Result})
                            : runSetupCommands(
                                  Commands, Dependencies,
                                  Index + qualityData().Numbers.NextIndex);
               }();
}

/**
 * User Story: As an end-to-end evaluator, I need every authored quality category exercised even when an earlier probe fails.
 * @fn inline void runQualityProbes( const TArray<FQualityProbe> &Probes, const FQualityModelMetadata &Metadata, FMicroGameStore &Store, const FQualityRunDependencies &Dependencies, int32 Index)
 */
inline void runQualityProbes(
    const TArray<FQualityProbe> &Probes,
    const FQualityModelMetadata &Metadata, FMicroGameStore &Store,
    const FQualityRunDependencies &Dependencies, int32 Index) {
  Index >= Probes.Num()
      ? void()
      : [&]() {
          const FQualityProbe &Probe = Probes[Index];
          const CommandRunner::FCommandOutput Result = executeQualityCommand(
              qualityProbeCommand(Probe), Dependencies);
          Store.dispatch(QualityActions::sampleRecorded(scoreQualitySample(
              Probe, Result, Metadata, Dependencies.Host)));
          runQualityProbes(
              Probes, Metadata, Store, Dependencies,
              Index + qualityData().Numbers.NextIndex);
        }();
}

} // namespace QualityThunksDetail

/**
 * User Story: As a release operator, I need the minimal game measure API-enforced NPC output through the CLI before declaring the run complete.
 * @fn inline void runQualityEvaluation( FMicroGameStore &Store, const FQualityRunDependencies &Dependencies)
 */
inline void runQualityEvaluation(
    FMicroGameStore &Store, const FQualityRunDependencies &Dependencies) {
  Store.dispatch(QualityActions::evaluationStarted(
      {Dependencies.Host, FDateTime::UtcNow().ToIso8601(),
       Dependencies.Baseline}));

  const CommandRunner::FCommandOutput MetadataResult =
      QualityThunksDetail::executeQualityCommand(
          qualityMetadataCommand(), Dependencies);
  const func::Maybe<FQualityModelMetadata> Metadata =
      MetadataResult.Status == qualityData().CommandStatuses.Ok
          ? readQualityModelMetadata(MetadataResult)
          : func::nothing<FQualityModelMetadata>();
  func::match(
      Metadata,
      [&Store, &Dependencies](const FQualityModelMetadata &CurrentMetadata) {
        Store.dispatch(QualityActions::metadataCaptured(CurrentMetadata));
        func::match(
            QualityThunksDetail::runSetupCommands(
                qualitySetupCommands(), Dependencies,
                qualityData().Numbers.InitialIndex),
            [&Store](const FQualitySetupFailure &Failure) {
              QualityThunksDetail::failEvaluation(
                  Store, formatQualityTemplate(
                             qualityData().Messages.SetupFailed,
                             {{qualityData().Tokens.Command,
                               Failure.Command.Command},
                              {qualityData().Tokens.Output,
                               Failure.Result.Output}}));
            },
            [&Store, &Dependencies, &CurrentMetadata]() {
              QualityThunksDetail::runQualityProbes(
                  qualityProbeDefinitions(), CurrentMetadata, Store,
                  Dependencies, qualityData().Numbers.InitialIndex);
              Store.dispatch(QualityActions::evaluationCompleted(
                  {FDateTime::UtcNow().ToIso8601()}));
            });
      },
      [&Store, &MetadataResult]() {
        QualityThunksDetail::failEvaluation(
            Store, formatQualityTemplate(
                       qualityData().Messages.MetadataFailed,
                       {{qualityData().Tokens.Output,
                         MetadataResult.Output}}));
      });
}

} // namespace MicroGame
