#!/bin/bash
# Builds GHMetalRecomp for macOS (the game and GH Metallica Launcher) and packages it as "Guitar Hero Metallica.app" in a
# disk image (.dmg).
#
#   installer/macos/make_dmg.sh            # Apple Silicon (mac-arm64-release)
#   ARCH=amd64 installer/macos/make_dmg.sh # Intel (mac-amd64-release)
#
# With assets/default.xex present the build recompiles the game. Without it, the shell is built
# (GHMETALLICA_SHELL, CMakeLists.txt): it starts and says where the game files go, but cannot play.
# Pass SHELL_BUILD=1 to build the shell even when default.xex is present.
#
# The app keeps nothing of the game inside: its settings, logs, user data and the game files (assets/) go to
# ~/Library/Application Support/GHMetalRecomp (src/startup.cpp, DataFolder). It is signed ad hoc (no Developer ID):
# on another Mac, open it the first time with right click > Open, or remove the quarantine attribute.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ARCH="${ARCH:-arm64}"
PRESET="mac-${ARCH}-release"
BUILD="$ROOT/out/build/$PRESET"
APP_NAME="Guitar Hero Metallica"
VERSION="$(sed -n 's/^#define RECOMP_VERSION_STR "\(.*\)"/\1/p' "$ROOT/res/version.h")"

shell=OFF
if [[ "${SHELL_BUILD:-0}" == 1 || ! -f "$ROOT/assets/default.xex" ]]; then
  shell=ON
fi
suffix=""
[[ $shell == ON ]] && suffix="-shell"

echo "== Building $PRESET (GHMETALLICA_SHELL=$shell)"
cmake --preset "$PRESET" -DGHMETALLICA_SHELL=$shell >/dev/null
cmake --build --preset "$PRESET" --target ghmetallica ghmetallica_launcher

STAGE="$ROOT/out/package/$PRESET"
APP="$STAGE/$APP_NAME.app"
rm -rf "$STAGE"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"

echo "== Assembling $APP_NAME.app"
# GH Metallica Launcher is the app's executable (options, first run from the ISO, profiles, updates; Play starts the
# game), the game's next to it, and the SDK's libraries next to that (@executable_path rpath).
LAUNCHER="GH Metallica Launcher"
cp "$BUILD/$LAUNCHER" "$APP/Contents/MacOS/"
cp "$BUILD/GHMetallicaRecomp" "$APP/Contents/MacOS/"
for lib in "$BUILD"/*.dylib; do
  cp "$lib" "$APP/Contents/MacOS/"
done
# The Vulkan loader and MoltenVK where the SDK looks for them in a bundle (Contents/Resources/vulkan:
# lib/ and share/vulkan/icd.d/, the ICD's library_path relative to it).
cp -R "$BUILD/vulkan" "$APP/Contents/Resources/vulkan"
# KosmicKrisp next to MoltenVK, the default driver when bundled (src/macos_vulkan_driver.cpp): from
# installer/macos/fetch_kosmickrisp.sh, which leaves it in out/kosmickrisp.
if [[ -f "$ROOT/out/kosmickrisp/lib/libvulkan_kosmickrisp.dylib" ]]; then
  cp "$ROOT/out/kosmickrisp/lib/libvulkan_kosmickrisp.dylib" "$APP/Contents/Resources/vulkan/lib/"
  cp "$ROOT/out/kosmickrisp/share/vulkan/icd.d/libkosmickrisp_icd.json" \
    "$APP/Contents/Resources/vulkan/share/vulkan/icd.d/"
  echo "   KosmicKrisp $(cat "$ROOT/out/kosmickrisp/VERSION") bundled"
else
  echo "   KosmicKrisp not bundled (installer/macos/fetch_kosmickrisp.sh): MoltenVK only"
fi

# The icon: res/icon.ico's 256 px image, scaled for the iconset.
ICONSET="$STAGE/AppIcon.iconset"
mkdir -p "$ICONSET"
sips -s format png "$ROOT/res/icon.ico" --out "$STAGE/icon.png" >/dev/null
for size in 16 32 64 128 256; do
  sips -z $size $size "$STAGE/icon.png" --out "$ICONSET/icon_${size}x${size}.png" >/dev/null
done
for size in 16 32 128; do
  sips -z $((size * 2)) $((size * 2)) "$STAGE/icon.png" --out "$ICONSET/icon_${size}x${size}@2x.png" >/dev/null
done
iconutil -c icns "$ICONSET" -o "$APP/Contents/Resources/AppIcon.icns"
rm -rf "$ICONSET" "$STAGE/icon.png"

sed -e "s/@VERSION@/$VERSION/g" -e "s/@APP_NAME@/$APP_NAME/g" -e "s/@EXECUTABLE@/$LAUNCHER/g" \
  "$ROOT/installer/macos/Info.plist.in" >"$APP/Contents/Info.plist"

echo "== Signing (ad hoc)"
find "$APP/Contents" -name '*.dylib' -exec codesign --force --sign - --timestamp=none {} \;
codesign --force --sign - --timestamp=none "$APP/Contents/MacOS/GHMetallicaRecomp"
codesign --force --sign - --timestamp=none "$APP"
codesign --verify --strict --verbose=1 "$APP"

echo "== Disk image"
DMG="$ROOT/out/package/GHMetalRecomp-$VERSION-macos-$ARCH$suffix.dmg"
DMG_ROOT="$STAGE/dmg"
mkdir -p "$DMG_ROOT"
mv "$APP" "$DMG_ROOT/"
ln -s /Applications "$DMG_ROOT/Applications"
rm -f "$DMG"
hdiutil create -volname "$APP_NAME" -srcfolder "$DMG_ROOT" -fs HFS+ -format UDZO -ov "$DMG" >/dev/null
# No second copy of the app left behind: LaunchServices registers every bundle it sees with this identifier and
# may start that one instead of (or as well as) the installed app.
rm -rf "$STAGE"
echo "$DMG"
