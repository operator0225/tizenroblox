#!/bin/bash
# TizenRoblox — Sober Flatpak Extractor
# Extracts ARM64 Sober binaries from an org.vinegarhq.Sober.flatpak bundle.
#
# Usage:
#   ./scripts/extract_sober.sh /path/to/org.vinegarhq.Sober.flatpak
#
# Requires: ostree, flatpak-builder (or flatpak)
# Output:   sober_bundle/{bin,libs}/

set -e
cd "$(dirname "$0")/.."

FLATPAK_FILE="${1:-}"
BUNDLE_DIR="sober_bundle"
REPO_DIR="/tmp/sober_ostree_repo"
CHECKOUT_DIR="/tmp/sober_checkout"

if [ -z "${FLATPAK_FILE}" ]; then
    echo "Usage: $0 <path-to-org.vinegarhq.Sober.flatpak>"
    echo ""
    echo "Download Sober ARM64 Flatpak from:"
    echo "  https://github.com/vinegarhq/sober/releases (aarch64)"
    exit 1
fi

if [ ! -f "${FLATPAK_FILE}" ]; then
    echo "ERROR: ${FLATPAK_FILE} not found"
    exit 1
fi

# ── Install dependencies ──────────────────────────────────────────────────────
for cmd in ostree flatpak; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "[extract] Installing $cmd..."
        apt-get install -y -qq "$cmd" 2>/dev/null || \
            echo "WARNING: Could not install $cmd — install manually"
    fi
done

# ── Extract OSTree bundle ─────────────────────────────────────────────────────
echo "[extract] Initializing OSTree repository..."
rm -rf "${REPO_DIR}" "${CHECKOUT_DIR}"
mkdir -p "${REPO_DIR}"
ostree --repo="${REPO_DIR}" init --mode=archive

echo "[extract] Importing Flatpak bundle..."
flatpak build-import-bundle "${REPO_DIR}" "${FLATPAK_FILE}"

echo "[extract] Finding Sober ref..."
SOBER_REF=$(ostree --repo="${REPO_DIR}" refs 2>/dev/null | grep -i "sober\|vinegar" | head -1)
if [ -z "${SOBER_REF}" ]; then
    echo "ERROR: Could not find Sober ref in bundle. Available refs:"
    ostree --repo="${REPO_DIR}" refs
    exit 1
fi
echo "[extract] Found ref: ${SOBER_REF}"

echo "[extract] Checking out files..."
ostree --repo="${REPO_DIR}" checkout --union "${SOBER_REF}" "${CHECKOUT_DIR}"

# ── Locate binaries ───────────────────────────────────────────────────────────
echo "[extract] Locating binaries..."
SOBER_BIN=$(find "${CHECKOUT_DIR}" -name "sober" -type f ! -name "sober_services" | head -1)
SOBER_SERVICES=$(find "${CHECKOUT_DIR}" -name "sober_services" -type f | head -1)
LIBLOADER=$(find "${CHECKOUT_DIR}" -name "libloader.so" | head -1)
LIBBADCPU=$(find "${CHECKOUT_DIR}" -name "libbadcpu.so" | head -1)
LIBMIMALLOC=$(find "${CHECKOUT_DIR}" -name "libmimalloc.so*" -not -l | head -1)

for f in "${SOBER_BIN}" "${LIBLOADER}" "${LIBBADCPU}"; do
    if [ -z "$f" ]; then
        echo "ERROR: Required binary not found in bundle. Contents:"
        ls -la "${CHECKOUT_DIR}/"
        exit 1
    fi
done

# ── Copy to sober_bundle/ ─────────────────────────────────────────────────────
echo "[extract] Copying to ${BUNDLE_DIR}/..."
mkdir -p "${BUNDLE_DIR}/bin" "${BUNDLE_DIR}/libs"

cp "${SOBER_BIN}" "${BUNDLE_DIR}/bin/sober"
chmod +x "${BUNDLE_DIR}/bin/sober"

[ -n "${SOBER_SERVICES}" ] && cp "${SOBER_SERVICES}" "${BUNDLE_DIR}/bin/sober_services" 2>/dev/null || true
cp "${LIBLOADER}"  "${BUNDLE_DIR}/libs/libloader.so"
cp "${LIBBADCPU}"  "${BUNDLE_DIR}/libs/libbadcpu.so"
[ -n "${LIBMIMALLOC}" ] && cp "${LIBMIMALLOC}" "${BUNDLE_DIR}/libs/libmimalloc.so.3" || true

# ── Cleanup ───────────────────────────────────────────────────────────────────
rm -rf "${REPO_DIR}" "${CHECKOUT_DIR}"

# ── Verify ───────────────────────────────────────────────────────────────────
echo ""
echo "=== Extracted binaries ==="
ls -lh "${BUNDLE_DIR}/bin/" "${BUNDLE_DIR}/libs/"
echo ""
echo "=== Architecture check ==="
file "${BUNDLE_DIR}/bin/sober"

ARCH=$(file "${BUNDLE_DIR}/bin/sober" | grep -o "ARM aarch64\|x86-64\|ARM,")
if echo "${ARCH}" | grep -q "aarch64"; then
    echo "✅ ARM64 binary confirmed"
else
    echo "WARNING: Expected ARM64 (aarch64), got: ${ARCH}"
fi

echo ""
echo "✅ Extraction complete! Now run: bash scripts/build.sh"
