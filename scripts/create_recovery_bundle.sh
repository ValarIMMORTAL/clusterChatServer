#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
timestamp="$(date +%Y%m%d_%H%M%S)"
out_dir="${1:-$repo_root/recovery_bundle_$timestamp}"

mkdir -p "$out_dir"

capture_version() {
    local cmd="$1"
    local label="$2"

    if command -v "$cmd" >/dev/null 2>&1; then
        {
            echo "[$label]"
            "$cmd" --version 2>/dev/null | head -n 1 || true
            echo
        } >> "$out_dir/environment-manifest.txt"
    fi
}

git -C "$repo_root" bundle create "$out_dir/clusterChatServer.bundle" --all
git -C "$repo_root" status --short --branch > "$out_dir/git-status.txt"
git -C "$repo_root" remote -v > "$out_dir/git-remote.txt"
git -C "$repo_root" diff > "$out_dir/working-tree.patch"
git -C "$repo_root" diff --cached > "$out_dir/staged.patch"
git -C "$repo_root" log --oneline --decorate -n 20 > "$out_dir/recent-history.txt"

{
    echo "generated_at=$(date -Is)"
    echo "repo_root=$repo_root"
    echo
    echo "[usr-local includes]"
    find /usr/local/include -maxdepth 1 -mindepth 1 \( -name muduo -o -name hiredis \) -print 2>/dev/null || true
    echo
    echo "[usr-local libs]"
    find /usr/local/lib -maxdepth 1 \( -name 'libmuduo*' -o -name 'libhiredis*' \) -print 2>/dev/null | sort || true
    echo
} > "$out_dir/environment-manifest.txt"

capture_version cmake "cmake"
capture_version g++ "g++"
capture_version make "make"
capture_version mysql "mysql"
capture_version redis-server "redis-server"

thirdparty_paths=()

for path in /usr/local/include/muduo /usr/local/include/hiredis; do
    if [[ -e "$path" ]]; then
        thirdparty_paths+=("$path")
    fi
done

while IFS= read -r path; do
    [[ -n "$path" ]] && thirdparty_paths+=("$path")
done < <(find /usr/local/lib -maxdepth 1 \( -name 'libmuduo*' -o -name 'libhiredis*' \) -print 2>/dev/null | sort)

if ((${#thirdparty_paths[@]} > 0)); then
    tar -czf "$out_dir/usr-local-thirdparty.tar.gz" "${thirdparty_paths[@]}"
fi

tar -czf "$out_dir/project-files.tar.gz" \
    --exclude=.git \
    --exclude=build \
    --exclude=bin \
    --exclude=.cache \
    --exclude='recovery_bundle_*' \
    -C "$repo_root" .

printf 'Recovery bundle created at %s\n' "$out_dir"
