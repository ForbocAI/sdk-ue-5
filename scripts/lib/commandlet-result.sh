#!/usr/bin/env bash

forbocai_run_commandlet() {
  local success_marker="$1"
  shift

  local output_file
  local command_status
  output_file="$(mktemp "${TMPDIR:-/tmp}/forbocai-commandlet.XXXXXX")"

  set +e
  "$@" -stdout -FullStdOutLogOutput -FORCELOGFLUSH 2>&1 | tee "$output_file"
  command_status=${PIPESTATUS[0]}
  set -e

  if [[ "$command_status" -ne 0 ]]; then
    rm -f "$output_file"
    return "$command_status"
  fi

  if [[ -n "$success_marker" ]] && ! grep -Fq "$success_marker" "$output_file"; then
    echo "Unreal exited without the required commandlet success marker: $success_marker" >&2
    rm -f "$output_file"
    return 1
  fi

  rm -f "$output_file"
}
