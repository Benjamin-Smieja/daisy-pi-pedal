#!/bin/bash
set -e

# Configuration
PI_USER="pi"
PI_PASS="raspberry"
PI_HOST="raspberrypi.local"
PI_DIR="/home/pi/projects/daisy-pi-pedal"          # mirrors the repo root
PI_SRC_DIR="$PI_DIR/rpi-controller"
PI_BUILD_DIR="$PI_SRC_DIR/build"

# Repo root = parent of this script's directory, so the script works from any cwd
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "🚀 Syncing local files to Raspberry Pi..."

# Sync the repo root (minus firmware) so ../protocol resolves the same on the Pi.
# A leading '/' anchors a pattern to the transfer root; without it, it matches at any depth.
rsync -avz --progress -e "sshpass -p '$PI_PASS' ssh -o StrictHostKeyChecking=no" \
    --exclude '/firmware/' \
    --exclude '*.pdf' \
    --exclude 'build/' \
    --exclude '.git' \
    --exclude '.vscode/' \
    "$REPO_ROOT/" "$PI_USER@$PI_HOST:$PI_DIR/"

echo "✨ Sync complete! Triggering remote build on the Pi..."

# Configure (creates build dir if needed) and build
sshpass -p "$PI_PASS" ssh -o StrictHostKeyChecking=no "$PI_USER@$PI_HOST" \
    "cmake -S $PI_SRC_DIR -B $PI_BUILD_DIR && cmake --build $PI_BUILD_DIR --clean-first"

echo "🎉 Done!"
