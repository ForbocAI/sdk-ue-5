#!/usr/bin/env bash

forbocai_prepare_plugin_host() {
  local root="$1"
  local host="$root/micro-game-cli/.forbocai-plugin-host"
  local plugin="$host/ForbocAI_SDK"
  local directory

  mkdir -p "$host"
  if [[ -L "$plugin" ]]; then
    rm "$plugin"
  fi
  if [[ -e "$plugin" && ! -d "$plugin" ]]; then
    echo "Plugin host path already exists and is not a directory: $plugin" >&2
    return 1
  fi
  mkdir -p "$plugin"

  cp "$root/ForbocAI_SDK.uplugin" "$plugin/ForbocAI_SDK.uplugin"
  for directory in Config Content Resources Source ThirdParty; do
    [[ -d "$root/$directory" ]] || continue
    if [[ -L "$plugin/$directory" ]]; then
      ln -sfn "$root/$directory" "$plugin/$directory"
      continue
    fi
    if [[ -e "$plugin/$directory" ]]; then
      echo "Plugin host directory already exists and is not a symlink: $plugin/$directory" >&2
      return 1
    fi
    ln -s "$root/$directory" "$plugin/$directory"
  done
}
