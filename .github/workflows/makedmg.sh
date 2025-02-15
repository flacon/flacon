#!/bin/bash
# v 0.1

APP_NAME=Flacon
CERT_IDENTITY="Developer ID Application: Alex Sokolov (635H9TYSZJ)"
KEYCHAIN_PROFILE="NotaryTool"

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
DMG_NAME="./${APP_NAME}_${VERSION}.dmg"

echo "***********************************"
echo "* App     ${APP_NAME}"
echo "* Version ${VERSION}"
echo "* DMG     ${DMG_NAME}"
echo "***********************************"

#rm -rf "${BUNDLE_PATH}/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Autoupdate.app"

echo "Remove quarantine flag ........................"
chmod  -R u+w "${BUNDLE_PATH}"
xattr -r -d com.apple.quarantine "${BUNDLE_PATH}"
echo "  OK"

echo "Sign .........................................."
codesign --force --options runtime --deep --verify --sign  "${CERT_IDENTITY}" "${BUNDLE_PATH}/Contents/Frameworks/Sparkle.framework/Versions/A/Resources/Autoupdate.app"
codesign --force --options runtime --deep --verify --sign  "${CERT_IDENTITY}" "${BUNDLE_PATH}"
echo "  OK"
echo ""

echo "Verify signature .............................."
codesign --all-architectures -v --strict --deep --verbose=1 "${BUNDLE_PATH}"
spctl --assess --type execute "${BUNDLE_PATH}"
echo "  OK"
echo ""

echo "Create DMG file .............................."
dmgbuild -s dmg_settings.json "${BUNDLE_PATH}" "${DMG_NAME}"
echo "  OK"

echo " Notarize ....................................."
xcrun notarytool submit --wait --keychain-profile "${KEYCHAIN_PROFILE}" "${DMG_NAME}" | tee notarytool-submit.log
echo "  OK"
echo ""

echo "Verify notarization ..........................."
spctl -a -vvv -t install "${BUNDLE_PATH}"
