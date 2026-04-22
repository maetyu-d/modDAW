#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENGINE_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${ENGINE_ROOT}/.." && pwd)"
MODDAW_HOST_PORT="${MODDAW_HOST_PORT:-57130}"
MODDAW_ENGINE_PORT="${MODDAW_ENGINE_PORT:-57131}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --host-port)
      MODDAW_HOST_PORT="$2"
      shift 2
      ;;
    --engine-port)
      MODDAW_ENGINE_PORT="$2"
      shift 2
      ;;
    *)
      shift
      ;;
  esac
done

export MODDAW_HOST_PORT
export MODDAW_ENGINE_PORT

find_sclang() {
  local candidates=()

  if [[ -n "${SCLANG_BIN:-}" ]]; then
    candidates+=("${SCLANG_BIN}")
  fi

  candidates+=(
    "${REPO_ROOT}/third_party/supercollider/build/lang/sclang"
    "${REPO_ROOT}/third_party/supercollider/build/install/bin/sclang"
    "${REPO_ROOT}/third_party/supercollider/cmake-build-debug/lang/sclang"
    "${REPO_ROOT}/third_party/supercollider/cmake-build-release/lang/sclang"
    "/Applications/SuperCollider.app/Contents/MacOS/sclang"
    "/opt/homebrew/bin/sclang"
    "/usr/local/bin/sclang"
  )

  for candidate in "${candidates[@]}"; do
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done

  if command -v sclang >/dev/null 2>&1; then
    command -v sclang
    return 0
  fi

  return 1
}

if ! SCLANG_PATH="$(find_sclang)"; then
  cat <<EOF >&2
Could not locate sclang.
Checked:
  - \$SCLANG_BIN
  - local SuperCollider builds under third_party/supercollider
  - /Applications/SuperCollider.app/Contents/MacOS/sclang
  - /opt/homebrew/bin/sclang
  - /usr/local/bin/sclang
  - PATH
EOF
  exit 1
fi

if [[ "$(uname -m)" == "arm64" && "${SCLANG_PATH}" == "/Applications/SuperCollider.app/Contents/MacOS/sclang" ]]; then
  exec arch -x86_64 "${SCLANG_PATH}" "${ENGINE_ROOT}/boot.scd"
fi

exec "${SCLANG_PATH}" "${ENGINE_ROOT}/boot.scd"
