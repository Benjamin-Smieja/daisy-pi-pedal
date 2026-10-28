#!/bin/bash

# Configuration
PI_USER="pi"
PI_PASS="raspberry"
PI_HOST="raspberrypi.local"
PI_DIR="/home/pi/projects/rpi-daisy-seed"
PI_BUILD_DIR="/home/pi/projects/rpi-daisy-seed/build"


echo "🚀 Syncing local files to Raspberry Pi..."

# Sync using sshpass to pipe the password into rsync's internal ssh connection
rsync -avz --progress -e "sshpass -p '$PI_PASS' ssh -o StrictHostKeyChecking=no" \
    --exclude 'build/' \
    --exclude '.git' \
    --exclude '.vscode/' \
    ./ "$PI_USER@$PI_HOST:$PI_DIR/"

echo "✨ Sync complete! Triggering remote build on the Pi..."

# Run clean and build on the Pi using sshpass
sshpass -p "$PI_PASS" ssh -o StrictHostKeyChecking=no "$PI_USER@$PI_HOST" "cd $PI_BUILD_DIR && make clean && make"

echo "🎉 Done!"
