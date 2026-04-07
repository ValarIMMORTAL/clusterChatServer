#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORK_DIR="${WORK_DIR:-$ROOT_DIR/.cache/bootstrap-src}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
MUDUO_REF="${MUDUO_REF:-v2.0.2}"
HIREDIS_REF="${HIREDIS_REF:-1.3.0}"
BUILD_PROJECT="${BUILD_PROJECT:-1}"
NPROC="${NPROC:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"

APT_PACKAGES=(
    build-essential
    cmake
    git
    pkg-config
    libboost-dev
    libmysqlclient-dev
    redis-server
)

run_as_root() {
    if [[ "$(id -u)" -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        echo "This step requires root privileges: $*" >&2
        exit 1
    fi
}

clone_or_update_repo() {
    local repo_url="$1"
    local ref="$2"
    local dest="$3"

    if [[ ! -d "$dest/.git" ]]; then
        git clone "$repo_url" "$dest"
    fi

    git -C "$dest" fetch --tags origin
    git -C "$dest" checkout "$ref"
}

echo "[1/5] Installing Ubuntu packages"
run_as_root apt-get update
run_as_root apt-get install -y "${APT_PACKAGES[@]}"

mkdir -p "$WORK_DIR"

echo "[2/5] Fetching muduo"
clone_or_update_repo "https://github.com/chenshuo/muduo.git" "$MUDUO_REF" "$WORK_DIR/muduo"

echo "[3/5] Building and installing muduo to $INSTALL_PREFIX"
(
    cd "$WORK_DIR/muduo"
    run_as_root env INSTALL_DIR="$INSTALL_PREFIX" ./build.sh -j"$NPROC" install
)

echo "[4/5] Fetching hiredis"
clone_or_update_repo "https://github.com/redis/hiredis.git" "$HIREDIS_REF" "$WORK_DIR/hiredis"

echo "[5/5] Building and installing hiredis to $INSTALL_PREFIX"
cmake -S "$WORK_DIR/hiredis" -B "$WORK_DIR/hiredis/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
cmake --build "$WORK_DIR/hiredis/build" -j"$NPROC"
run_as_root cmake --install "$WORK_DIR/hiredis/build"

if [[ "$INSTALL_PREFIX" == "/usr/local" ]]; then
    run_as_root ldconfig
fi

if [[ "$BUILD_PROJECT" == "1" ]]; then
    echo "[extra] Rebuilding clusterChatServer"
    cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build"
    cmake --build "$ROOT_DIR/build" -j"$NPROC"
fi

cat <<EOF
Bootstrap completed.

Next steps:
1. Restore the MySQL database backup named clusterChat.
2. Start Redis if it is not already running.
3. Run:
   ./bin/clusterChatServer 127.0.0.1 8080
   ./bin/chatClient 127.0.0.1 8080
EOF
