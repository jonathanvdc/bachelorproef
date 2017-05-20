#!/bin/bash
set -e # Exit with nonzero exit code if anything fails

# This file is based on the tutorial here: https://gist.github.com/domenic/ec8b0fc8ab45f39403dd

TARGET_BRANCH="master"

# Pull requests and commits to other branches shouldn't try to deploy, just build to verify
if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    echo "Skipping deploy."
    exit 0
fi

# Save some useful information
REPO="https://github.com/flu-plus-plus/flu-plus-plus.github.io"
PASSWORD_REPO="https://flubot:${FLUBOT_PASSWORD}@github.com/flu-plus-plus/flu-plus-plus.github.io"
SHA=`git rev-parse --verify HEAD`

# Clone the existing gh-pages for this repo into out/
# Create a new empty branch if gh-pages doesn't exist yet (should only happen on first deploy)
echo "Cloning flu-plus-plus.github.io repository..."
git clone $REPO out
cd out
git checkout $TARGET_BRANCH || git checkout --orphan $TARGET_BRANCH

cd ..

# Try to push until we succeed.
while ! ./push-report-helper.sh; do
    # Sleep for a while.
    sleep 10
    # Try again.
    ./push-report-helper.sh
done
