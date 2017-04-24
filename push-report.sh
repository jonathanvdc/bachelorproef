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

# Copy our gtest report to out/gtest-reports/CONTRIBUTOR-BRANCH-COMPILER.xml
echo "Copying gtest report to flu-plus-plus.github.io/gtest-reports/ ..."
REPO_OWNER_NAME=$(echo $TRAVIS_REPO_SLUG | cut -d '/' -f1)
mkdir -p out/gtest-reports
cp build/installed/bin/gtester_all.xml out/gtest-reports/${REPO_OWNER_NAME}-${TRAVIS_BRANCH}-${CC_COMPILER_NAME}.xml

# Create a table comparison of all reports.
echo "Regenerating comparison table..."
mono gtest-report-tools/gtest-report-html.exe out/gtest-reports/*.xml --css=gtest-report-tools/resources/simple-css.css \
    > out/gtest-reports/comparison.html

# Now let's go have some fun with the cloned repo
cd out

echo "Configuring user..."
git config user.name "FluBot"
git config user.email "flu-plus-plus-flubot@outlook.com"

# If there are no changes to the compiled out (e.g. this is a README update) then just bail.
if [ -z `git diff --exit-code` ]; then
    echo "No changes to the output on this push; exiting."
    exit 0
fi

# Commit the "changes", i.e. the new version.
# The delta will show diffs between new and old versions.
echo "Committing changes..."
git add .
git commit -m "Deploy to GitHub Pages: ${SHA}"

# Now that we're all set up, we can push.
echo "Pushing commit..."
git push $PASSWORD_REPO $TARGET_BRANCH
echo "Pushed commit"