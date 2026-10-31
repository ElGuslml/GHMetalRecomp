#!/bin/bash
# Fetches KosmicKrisp (LunarG's Vulkan driver on Metal, from Mesa; Apple Silicon, macOS 26 or later) for the macOS app:
# the LunarG Vulkan SDK's archive is downloaded and only the driver is taken out of it, as data. The SDK's installer is
# never run. make_dmg.sh bundles what this leaves in out/kosmickrisp (src/macos_vulkan_driver.cpp selects it).
#
#   installer/macos/fetch_kosmickrisp.sh [SDK version]     # default: the version below
#
# Needs: curl, unzip, python3, 7z (brew install p7zip).

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VERSION="${1:-1.4.363.0}"
OUT="$ROOT/out/kosmickrisp"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

command -v 7z >/dev/null || { echo "7z is missing: brew install p7zip" >&2; exit 1; }

echo "== Downloading the Vulkan SDK $VERSION for macOS"
curl -fsSL -o "$WORK/sdk.zip" "https://sdk.lunarg.com/sdk/download/$VERSION/mac/vulkansdk-macos-$VERSION.zip"
unzip -q "$WORK/sdk.zip" -d "$WORK/sdk"
DAT="$(find "$WORK/sdk" -path '*Contents/Resources/installer.dat' | head -1)"
[[ -f "$DAT" ]] || { echo "installer.dat not found in the SDK archive" >&2; exit 1; }

echo "== Taking KosmicKrisp out of the SDK's payload"
# The Qt installer's payload holds 7-Zip archives: each is cut out at its signature, its length from its header.
python3 -I - "$DAT" "$WORK/parts" <<'EOF'
import os, re, struct, sys
data = open(sys.argv[1], "rb").read()
os.makedirs(sys.argv[2], exist_ok=True)
for i, match in enumerate(re.finditer(re.escape(b"7z\xbc\xaf\x27\x1c"), data)):
    start = match.start()
    next_offset, next_size = struct.unpack_from("<QQ", data, start + 12)
    with open(os.path.join(sys.argv[2], f"part{i}.7z"), "wb") as f:
        f.write(data[start:start + 32 + next_offset + next_size])
EOF
found=""
for part in "$WORK"/parts/*.7z; do
  # The listing first, then the search: with pipefail, grep -q stopping early would fail 7z's write (SIGPIPE).
  listing="$(7z l "$part" 2>/dev/null || true)"
  if grep -q 'macOS/lib/libvulkan_kosmickrisp.dylib' <<<"$listing"; then
    7z x -y -o"$WORK/kk" "$part" 'macOS/lib/libvulkan_kosmickrisp.dylib' \
      'macOS/share/vulkan/icd.d/libkosmickrisp_icd.json' -r >/dev/null
    found=1
  fi
done
[[ -n "$found" ]] || { echo "KosmicKrisp is not in this SDK" >&2; exit 1; }

rm -rf "$OUT"
mkdir -p "$OUT/lib" "$OUT/share/vulkan/icd.d"
cp "$WORK/kk/macOS/lib/libvulkan_kosmickrisp.dylib" "$OUT/lib/"
cp "$WORK/kk/macOS/share/vulkan/icd.d/libkosmickrisp_icd.json" "$OUT/share/vulkan/icd.d/"
codesign --verify "$OUT/lib/libvulkan_kosmickrisp.dylib"
echo "$VERSION" >"$OUT/VERSION"
echo "== KosmicKrisp from the Vulkan SDK $VERSION in $OUT"
