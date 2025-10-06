#!/bin/bash
# Fix includes in all copied files

echo "Fixing includes in copied files..."

# Fix crypto files
for f in src/crypto/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "crypto_utils.h"|#include "hl_crypto_internal.h"|g' "$f"
    sed -i '' 's|#include "logger.h"|#include "hl_logger.h"|g' "$f"
    sed -i '' 's|#include "sha3.h"|#include "hl_crypto_internal.h"|g' "$f"
done

# Fix msgpack files
for f in src/msgpack/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "hyperliquid_action.h"|#include "hl_msgpack.h"|g' "$f"
    sed -i '' 's|#include "crypto_utils.h"|#include "hl_crypto_internal.h"|g' "$f"
done

# Fix http files
for f in src/http/*.c; do
    echo "  Processing $f..."
    sed -i '' 's|#include "http_client.h"|#include "hl_http.h"|g' "$f"
    sed -i '' 's|#include "logger.h"|#include "hl_logger.h"|g' "$f"
    sed -i '' 's|#include "memory.h"|// memory.h not needed|g' "$f"
done

echo "✅ Includes fixed!"
