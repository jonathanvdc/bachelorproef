#!/bin/bash
set -e # Exit with nonzero exit code if anything fails

# This file is based on the tutorial here: https://gist.github.com/domenic/ec8b0fc8ab45f39403dd

# Copy our gtest report to out/gtest-reports/CONTRIBUTOR-BRANCH-COMPILER.xml
echo "Copying gtest report to flu-plus-plus.github.io/gtest-reports/ ..."
REPO_OWNER_NAME=$(echo $TRAVIS_REPO_SLUG | cut -d '/' -f1)
mkdir -p out/gtest-reports
cp build/installed/bin/gtester_all.xml out/gtest-reports/${REPO_OWNER_NAME}-${TRAVIS_BRANCH}-${CONFIG_IDENTIFIER}.xml

# Create a table comparison of all reports.
echo "Regenerating comparison table..."
mono gtest-report-tools/gtest-report-html.exe out/gtest-reports/*.xml --css=gtest-report-tools/resources/simple-style.css \
    > out/gtest-reports/comparison.html

# Now let's go have some fun with the cloned repo
cd out

echo "Configuring user..."
git config user.name "FluBot"
git config user.email "flu-plus-plus-flubot@outlook.com"

git add .

# If there are no changes to the compiled out (e.g. this is a README update) then just bail.
if git diff --cached --exit-code; then
    echo "No changes to the output on this push; exiting."
    exit 0
fi

# Commit the "changes", i.e. the new version.
# The delta will show diffs between new and old versions.
echo "Committing changes..."
git commit -m "Deploy comparison table to github.io: ${SHA}"

# Now that we're all set up, we can push.
echo "Pushing commit..."
if git push $PASSWORD_REPO $TARGET_BRANCH; then
    echo "Pushed commit"
    cd ..
else
    # Nuke the commit we just made.
    echo "Push failed. Nuking commit..."
    git reset HEAD^
    # Do a git pull.
    git pull $PASSWORD_REPO $TARGET_BRANCH
    cd ..
    exit 1
fi