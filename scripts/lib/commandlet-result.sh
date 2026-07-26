#!/usr/bin/env bash

forbocai_run_commandlet() {
  local success_marker="$1"
  shift

  local output_file
  local command_status
  local temporary_output=0
  if [[ -n "${FORBOCAI_MICRO_GAME_LOG_PATH:-}" ]]; then
    output_file="$FORBOCAI_MICRO_GAME_LOG_PATH"
    mkdir -p "$(dirname "$output_file")"
  else
    output_file="$(mktemp "${TMPDIR:-/tmp}/forbocai-commandlet.XXXXXX")"
    temporary_output=1
  fi

  set +e
  "$@" -stdout -FullStdOutLogOutput -FORCELOGFLUSH 2>&1 | tee "$output_file"
  command_status=${PIPESTATUS[0]}
  set -e

  if [[ "$command_status" -ne 0 ]]; then
    [[ "$temporary_output" -eq 1 ]] && rm -f "$output_file"
    return "$command_status"
  fi

  if [[ -n "$success_marker" ]] && ! grep -Fq "$success_marker" "$output_file"; then
    echo "Unreal exited without the required commandlet success marker: $success_marker" >&2
    [[ "$temporary_output" -eq 1 ]] && rm -f "$output_file"
    return 1
  fi

  [[ "$temporary_output" -eq 1 ]] && rm -f "$output_file"
}
