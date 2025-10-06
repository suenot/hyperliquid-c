#!/bin/bash

# Script to check code modularity (files larger than 1000 lines)
# Exits with code 0 if all files are properly modularized, 1 otherwise

set -e

echo "## 📊 Code Modularity Check"
echo "Checking for files larger than 1000 lines..."
echo

# Initialize counters
TOTAL_FILES=0
TOTAL_LINES=0
FILES_OVER_1000=0
MAX_LINES=0
MAX_FILE=""

# Check each source file
echo "### File Analysis:"
printf "%-50s %8s %s\n" "File" "Lines" "Status"
printf "%-50s %8s %s\n" "----" "-----" "------"

for file in $(find src include -name "*.c" -o -name "*.h" | sort); do
    if [ -f "$file" ]; then
        lines=$(wc -l < "$file")
        TOTAL_FILES=$((TOTAL_FILES + 1))
        TOTAL_LINES=$((TOTAL_LINES + lines))

        if [ "$lines" -gt "$MAX_LINES" ]; then
            MAX_LINES=$lines
            MAX_FILE=$file
        fi

        if [ "$lines" -gt 1000 ]; then
            printf "%-50s %8d 🚨 TOO LARGE\n" "$file" "$lines"
            FILES_OVER_1000=$((FILES_OVER_1000 + 1))
        else
            printf "%-50s %8d ✅ OK\n" "$file" "$lines"
        fi
    fi
done

echo
echo "### 📈 Statistics:"
echo "- Total source files: $TOTAL_FILES"
echo "- Total lines of code: $TOTAL_LINES"

if [ "$TOTAL_FILES" -gt 0 ]; then
    AVG_LINES=$((TOTAL_LINES / TOTAL_FILES))
    echo "- Average lines per file: $AVG_LINES"
fi

echo "- Largest file: $MAX_FILE ($MAX_LINES lines)"
echo "- Files > 1000 lines: $FILES_OVER_1000"

# Determine grade
if [ "$FILES_OVER_1000" -eq 0 ]; then
    GRADE="A+"
    COLOR="brightgreen"
    STATUS="✅ PERFECT MODULARITY"
    EXIT_CODE=0
elif [ "$FILES_OVER_1000" -le 2 ]; then
    GRADE="A"
    COLOR="green"
    STATUS="✅ GOOD MODULARITY"
    EXIT_CODE=0
elif [ "$FILES_OVER_1000" -le 5 ]; then
    GRADE="B"
    COLOR="yellow"
    STATUS="⚠️ NEEDS IMPROVEMENT"
    EXIT_CODE=0
else
    GRADE="C"
    COLOR="red"
    STATUS="🚨 CRITICAL ISSUES"
    EXIT_CODE=1
fi

echo
echo "## 🎯 Modularity Grade: $GRADE"
echo "$STATUS"

# Output for GitHub Actions
echo "MODULARITY_GRADE=$GRADE" >> $GITHUB_ENV
echo "MODULARITY_COLOR=$COLOR" >> $GITHUB_ENV
echo "FILES_OVER_1000=$FILES_OVER_1000" >> $GITHUB_ENV
echo "MAX_FILE=$MAX_FILE" >> $GITHUB_ENV
echo "MAX_LINES=$MAX_LINES" >> $GITHUB_ENV

exit $EXIT_CODE
