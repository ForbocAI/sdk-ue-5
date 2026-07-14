#!/usr/bin/env bash

forbocai_load_test_environment() {
  local key_requirement="${1:-required}"
  local env_file="${FORBOCAI_TEST_ENV_FILE:-${XDG_CONFIG_HOME:-$HOME/.config}/forbocai/test-api.env}"
  local restore_allexport=0

  if [[ -z "${FORBOCAI_API_KEY:-}" && -f "$env_file" ]]; then
    case "$-" in
      *a*) restore_allexport=1 ;;
    esac
    set -a
    # shellcheck disable=SC1090
    source "$env_file"
    [[ "$restore_allexport" -eq 1 ]] || set +a
  fi

  if [[ "$key_requirement" == "required" && -z "${FORBOCAI_API_KEY:-}" ]]; then
    echo "FORBOCAI_API_KEY is required for live verification." >&2
    echo "Set it in $env_file or FORBOCAI_TEST_ENV_FILE." >&2
    return 1
  fi
}
