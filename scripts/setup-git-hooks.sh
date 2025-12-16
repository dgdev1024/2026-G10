#!/bin/bash
set -euo pipefail

echo "Setting repository git hooks path to .githooks"
git config core.hooksPath .githooks
chmod +x .githooks/pre-commit
echo "Done. Future commits will use the hook in .githooks/pre-commit"
