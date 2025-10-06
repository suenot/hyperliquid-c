#!/bin/bash

# Script to generate coverage badge from lcov output
# Usage: ./generate-coverage-badge.sh <coverage_percentage>

set -e

COVERAGE_PERCENT="$1"

if [ -z "$COVERAGE_PERCENT" ]; then
    echo "Usage: $0 <coverage_percentage>"
    echo "Example: $0 85.3"
    exit 1
fi

# Validate input
if ! [[ "$COVERAGE_PERCENT" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    echo "Error: Invalid coverage percentage format"
    exit 1
fi

# Determine badge color
if (( $(echo "$COVERAGE_PERCENT >= 90" | bc -l 2>/dev/null || echo "0") )); then
    COLOR="brightgreen"
elif (( $(echo "$COVERAGE_PERCENT >= 75" | bc -l 2>/dev/null || echo "0") )); then
    COLOR="yellow"
elif (( $(echo "$COVERAGE_PERCENT >= 60" | bc -l 2>/dev/null || echo "0") )); then
    COLOR="orange"
else
    COLOR="red"
fi

# Generate badge URL
BADGE_URL="https://img.shields.io/badge/coverage-${COVERAGE_PERCENT}%25-${COLOR}.svg"

echo "Coverage: ${COVERAGE_PERCENT}%"
echo "Badge Color: $COLOR"
echo "Badge URL: $BADGE_URL"

# Output for GitHub Actions
echo "COVERAGE_PERCENTAGE=$COVERAGE_PERCENT" >> $GITHUB_ENV
echo "COVERAGE_BADGE_URL=$BADGE_URL" >> $GITHUB_ENV
echo "COVERAGE_COLOR=$COLOR" >> $GITHUB_ENV
