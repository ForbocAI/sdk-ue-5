#!/usr/bin/env bash

forbocai_append_wsl_env_entries() {
  local current="${WSLENV:-}"
  local entry

  for entry in "$@"; do
    [[ -z "$entry" ]] && continue
    case ":$current:" in
      *":$entry:"*) ;;
      *) current="${current:+$current:}$entry" ;;
    esac
  done

  printf '%s' "$current"
}

forbocai_wsl_env_entry_for() {
  local name="$1"
  local value="${!name:-}"

  case "$name" in
    API_TEST_GAME_CONTRACT|FORBOCAI_CONFIG_PATH|FORBOCAI_DATABASE_PATH|FORBOCAI_TEST_ENV_FILE|FORBOC_SDK_SOURCE_PATH|UE_ROOT)
      [[ "$value" == /* ]] && printf '%s/p\n' "$name" || printf '%s\n' "$name"
      ;;
    *)
      printf '%s\n' "$name"
      ;;
  esac
}

forbocai_collect_wsl_env_entries() {
  local name

  while IFS='=' read -r name _; do
    case "$name" in
      API_TEST_GAME_CONTRACT|FORBOCAI*|FORBOC_FP_*|FORBOC_SDK_SOURCE_PATH|UE_ROOT)
        forbocai_wsl_env_entry_for "$name"
        ;;
    esac
  done < <(env)
}

forbocai_export_wsl_environment() {
  local entries=()

  mapfile -t entries < <(forbocai_collect_wsl_env_entries)
  WSLENV="$(forbocai_append_wsl_env_entries "${entries[@]}")"
  export WSLENV
}
