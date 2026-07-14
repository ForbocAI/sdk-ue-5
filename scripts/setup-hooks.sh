#!/bin/sh
# Run once after cloning to activate git hooks and (optionally) link the runtime repo.
git config core.hooksPath .githooks
chmod +x .githooks/post-commit .githooks/post-merge \
  scripts/git/sync-demo-submodule.sh
echo "Hooks enabled."

if [ -n "$1" ]; then
  git config forboc.demoPath "$1"
  echo "Demo path set to: $1"
else
  echo ""
  echo "To auto-sync the demo-ue-5 checkout after each SDK commit/pull, run:"
  echo "  git config forboc.demoPath /path/to/demo-ue-5"
fi
