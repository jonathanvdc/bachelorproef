#!/usr/bin/env bash
set -o pipefail

# Run the tests.
make test_all_no_x
# Pretty-print the results.
mono gtest-report-tools/gtest-report-print.exe build/installed/bin/gtester_all_no_x.xml