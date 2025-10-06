#!/bin/bash

# Hyperliquid C SDK - Readiness Check
# This script verifies that the project is ready for publication

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     HYPERLIQUID C SDK - READINESS CHECK                    ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

CHECKS_PASSED=0
CHECKS_FAILED=0

check_file() {
    local file=$1
    local description=$2
    
    if [ -f "$file" ]; then
        echo -e "${GREEN}✅${NC} $description: $file"
        CHECKS_PASSED=$((CHECKS_PASSED + 1))
    else
        echo -e "${RED}❌${NC} $description: $file (NOT FOUND)"
        CHECKS_FAILED=$((CHECKS_FAILED + 1))
    fi
}

check_dir() {
    local dir=$1
    local description=$2
    
    if [ -d "$dir" ]; then
        local count=$(find "$dir" -type f | wc -l)
        echo -e "${GREEN}✅${NC} $description: $dir ($count files)"
        CHECKS_PASSED=$((CHECKS_PASSED + 1))
    else
        echo -e "${RED}❌${NC} $description: $dir (NOT FOUND)"
        CHECKS_FAILED=$((CHECKS_FAILED + 1))
    fi
}

echo "📋 Checking Documentation..."
echo "────────────────────────────────────────────────────────────────"
check_file "README.md" "Main README"
check_file "README_GITHUB.md" "GitHub README"
check_file "LICENSE" "License file"
check_file "QUICKSTART.md" "Quick start guide"
check_file "ARCHITECTURE.md" "Architecture docs"
check_file "STATUS.md" "Status document"
check_file "CONTRIBUTING.md" "Contributing guide"
check_file "CURRENT_STATUS.md" "Current status"
check_file "PHASE2_COMPLETE.md" "Phase 2 report"
check_file ".gitignore" "Git ignore file"
echo ""

echo "📁 Checking Directory Structure..."
echo "────────────────────────────────────────────────────────────────"
check_dir "include" "Header files"
check_dir "src" "Source files"
check_dir "tests" "Test files"
check_dir "examples" "Example code"
check_dir "docs" "Documentation"
echo ""

echo "📜 Checking Header Files..."
echo "────────────────────────────────────────────────────────────────"
check_file "include/hyperliquid.h" "Public API"
check_file "include/hl_crypto_internal.h" "Crypto internals"
check_file "include/hl_msgpack.h" "MessagePack API"
check_file "include/hl_http.h" "HTTP API"
check_file "include/hl_internal.h" "Internal API"
check_file "include/hl_logger.h" "Logger"
echo ""

echo "📝 Checking Source Files..."
echo "────────────────────────────────────────────────────────────────"
check_file "src/client.c" "Client wrapper"
check_file "src/trading.c" "Trading logic"
check_file "src/crypto/keccak.c" "Keccak-256"
check_file "src/crypto/eip712.c" "EIP-712"
check_file "src/msgpack/serialize.c" "MessagePack"
check_file "src/http/client.c" "HTTP client"
echo ""

echo "🧪 Checking Tests..."
echo "────────────────────────────────────────────────────────────────"
check_file "tests/test_minimal.c" "Minimal test"
echo ""

echo "📖 Checking Examples..."
echo "────────────────────────────────────────────────────────────────"
check_file "examples/simple_trade.c" "Simple trade example"
echo ""

echo "🔨 Checking Build System..."
echo "────────────────────────────────────────────────────────────────"
check_file "Makefile" "Production Makefile"
check_file "Makefile.test" "Test Makefile"
echo ""

echo "🧪 Running Tests..."
echo "────────────────────────────────────────────────────────────────"
if make -f Makefile.test test_minimal > /dev/null 2>&1; then
    echo -e "${GREEN}✅${NC} Core module tests: PASSING"
    CHECKS_PASSED=$((CHECKS_PASSED + 1))
else
    echo -e "${RED}❌${NC} Core module tests: FAILING"
    CHECKS_FAILED=$((CHECKS_FAILED + 1))
fi
echo ""

echo "📊 Project Statistics..."
echo "────────────────────────────────────────────────────────────────"
TOTAL_FILES=$(find . -type f ! -path './.git/*' ! -path './build/*' | wc -l | tr -d ' ')
TOTAL_SIZE=$(du -sh . | awk '{print $1}')
C_FILES=$(find . -name "*.c" ! -path './build/*' | wc -l | tr -d ' ')
H_FILES=$(find . -name "*.h" ! -path './build/*' | wc -l | tr -d ' ')
MD_FILES=$(find . -name "*.md" ! -path './build/*' | wc -l | tr -d ' ')

echo "Total files:      $TOTAL_FILES"
echo "Total size:       $TOTAL_SIZE"
echo "C source files:   $C_FILES"
echo "Header files:     $H_FILES"
echo "Markdown files:   $MD_FILES"
echo ""

echo "════════════════════════════════════════════════════════════════"
echo ""
echo "📊 FINAL RESULTS:"
echo ""
echo -e "${GREEN}✅ Checks passed:${NC} $CHECKS_PASSED"
echo -e "${RED}❌ Checks failed:${NC} $CHECKS_FAILED"
echo ""

if [ $CHECKS_FAILED -eq 0 ]; then
    echo "════════════════════════════════════════════════════════════════"
    echo -e "${GREEN}🎉 PROJECT IS READY FOR PUBLICATION! 🎉${NC}"
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    echo "Next steps:"
    echo "1. Initialize git: git init"
    echo "2. Add files: git add ."
    echo "3. Commit: git commit -m 'Initial commit - Hyperliquid C SDK v0.9.0'"
    echo "4. Add remote: git remote add origin https://github.com/suenot/hyperliquid-c.git"
    echo "5. Push: git push -u origin master"
    echo ""
    exit 0
else
    echo "════════════════════════════════════════════════════════════════"
    echo -e "${YELLOW}⚠️  SOME CHECKS FAILED${NC}"
    echo "════════════════════════════════════════════════════════════════"
    echo ""
    echo "Please fix the issues above before publishing."
    echo ""
    exit 1
fi
