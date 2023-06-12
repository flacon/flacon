#!/bin/bash
# v 0.1

APP_NAME=Flacon
MAKE_DMG=1
CERT_IDENTITY="Developer ID Application: Alex Sokolov (635H9TYSZJ)"

#--------------------

set -Eeuo pipefail
#set -x

TAR=$(find . -name "${APP_NAME}-*.tar" | sort | tail -n 1)
if [[ -z ${TAR} ]]; then
    echo "Tar file not found" >&2
    exit 1
fi

BUNDLE_NAME="${APP_NAME}.app"
BUNDLE_PATH="./${BUNDLE_NAME}"

rm -rf ${BUNDLE_PATH}
tar xf ${TAR}

VERSION=$(/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" ${BUNDLE_PATH}/Contents/Info.plist)
DMG_NAME="./${APP_NAME}-${VERSION}.dmg"

echo "***********************************"
echo "* App     ${APP_NAME}"
echo "* Version ${VERSION}"
echo "* DMG     ${MAKE_DMG}"
echo "***********************************"

echo "Sign and verify ..............................."
codesign --force --deep --verify --sign  "${CERT_IDENTITY}" "${BUNDLE_PATH}"
codesign --all-architectures -v --strict --deep --verbose=1 "${BUNDLE_PATH}"
spctl --assess --type execute "${BUNDLE_PATH}"
echo "  OK"
echo ""

echo "Verify signature .............................."
codesign --all-architectures -v --strict --deep --verbose=1 "${BUNDLE_PATH}"
spctl --assess --type execute "${BUNDLE_PATH}"
echo "  OK"
echo ""

if [[ MAKE_DMG ]]; then
    dmgbuild -s dmg_settings.json "${BUNDLE_PATH}" "${DMG_NAME}"
fi
