#!/bin/sh
set -eu

SEED_FLOW="/usr/src/node-red/flows/esp32_level3_dashboard.json"
TARGET_FLOW="/data/flows.json"

if [ ! -f "$TARGET_FLOW" ] && [ -f "$SEED_FLOW" ]; then
  cp "$SEED_FLOW" "$TARGET_FLOW"
fi

exec /usr/src/node-red/entrypoint.sh "$@"
