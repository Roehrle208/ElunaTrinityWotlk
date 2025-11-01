#!/bin/bash

set -e

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
PATCH_DIR="$REPO_ROOT/repack/patches"
LOG_FILE="$REPO_ROOT/repack/patch.log"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

GREEN="\e[32m"
RED="\e[31m"
YELLOW="\e[33m"
RESET="\e[0m"

dry_run=false

log() {
  echo -e "$1"
  echo -e "[$TIMESTAMP] ${1//${RESET}/}" >> "$LOG_FILE"
}

usage() {
  echo "Usage:"
  echo "  $0 make [--dry-run]     → Erzeugt beide Patch-Dateien"
  echo "  $0 apply [--dry-run]    → Wendet beide Patch-Dateien an"
  echo "  $0 clean [--dry-run]    → Entfernt angewendete Patches"
  echo "  $0 help                 → Zeigt diese Hilfe an"
  exit 1
}

make_patches() {
  log "${YELLOW}📦 Erzeuge core.patch...${RESET}"
  $dry_run || git diff master...feature/core-adjustments > "$PATCH_DIR/core.patch"

  log "${YELLOW}📦 Erzeuge bot-engine.patch...${RESET}"
  $dry_run || git diff feature/core-adjustments...feature/bot-engine > "$PATCH_DIR/bot-engine.patch"

  log "${GREEN}✅ Patches gespeichert unter: $PATCH_DIR${RESET}"
}

apply_patches() {
  log "${YELLOW}🩹 Wende core.patch an...${RESET}"
  if $dry_run; then
    git apply --check "$PATCH_DIR/core.patch"
  else
    git apply "$PATCH_DIR/core.patch"
  fi

  log "${YELLOW}🩹 Wende bot-engine.patch an...${RESET}"
  if $dry_run; then
    git apply --check "$PATCH_DIR/bot-engine.patch"
  else
    git apply "$PATCH_DIR/bot-engine.patch"
  fi

  log "${GREEN}✅ Beide Patches wurden verarbeitet.${RESET}"
}

clean_patches() {
  log "${YELLOW}🧼 Entferne core.patch...${RESET}"
  if $dry_run; then
    git apply -R --check "$PATCH_DIR/core.patch"
  else
    git apply -R "$PATCH_DIR/core.patch" || log "${RED}⚠️ core.patch konnte nicht rückgängig gemacht werden.${RESET}"
  fi

  log "${YELLOW}🧼 Entferne bot-engine.patch...${RESET}"
  if $dry_run; then
    git apply -R --check "$PATCH_DIR/bot-engine.patch"
  else
    git apply -R "$PATCH_DIR/bot-engine.patch" || log "${RED}⚠️ bot-engine.patch konnte nicht rückgängig gemacht werden.${RESET}"
  fi

  log "${GREEN}✅ Patch-Versuche abgeschlossen.${RESET}"
}

# Argumente parsen
cmd="$1"
shift
while [[ "$1" == --* ]]; do
  case "$1" in
    --dry-run) dry_run=true ;;
    *) usage ;;
  esac
  shift
done

case "$cmd" in
  make) make_patches ;;
  apply) apply_patches ;;
  clean) clean_patches ;;
  help | *) usage ;;
esac
