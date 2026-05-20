#!/usr/bin/env bash

set -e

EXPECTATION_TABLE=$1
REPO_NAME=$2

COMMIT="$(git rev-parse HEAD)"
BRANCHES_W_COMMIT="$(git branch -r --contains $COMMIT)"

EXPECTED_BRANCH=$(sed -n "s#^$REPO_NAME ##p" <(cat $EXPECTATION_TABLE))

if [ "$EXPECTED_BRANCH" == "" ]; then
    echo "  This module does not have an expected branch declared in \"check-submodules.sh\"."
    echo "  Please declare the correct branch."
    exit 1
fi

if grep "$EXPECTED_BRANCH" <(echo $BRANCHES_W_COMMIT) > /dev/null; then
    echo "  Correct branch ($EXPECTED_BRANCH)"
else
    echo "  Incorrect branch (expected $EXPECTED_BRANCH)"
    exit 1
fi
