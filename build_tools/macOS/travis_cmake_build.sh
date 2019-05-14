#!/bin/sh

# Run configure_macOS.sh
./configure_macOS.sh

# Include Xcode helpers
. build_tools/macOS/_xcodebuild_helpers.sh

# Build Xcode project, and package "WMIT.zip"
execute_xcodebuild_command  \
 -project build/wmit.xcodeproj \
 -target "package" \
 -configuration "Release" \
 -destination "platform=macOS" \
 -PBXBuildsContinueAfterErrors=NO
result=${?}
if [ $result -ne 0 ]; then
	echo "ERROR: xcodebuild failed"
	exit ${result}
fi

# Verify "WMIT.zip" was created
BUILT_WMIT_ZIP="build/WMIT.zip"
if [ ! -f "${BUILT_WMIT_ZIP}" ]; then
	echo "ERROR: Something went wrong, and \"${BUILT_WMIT_ZIP}\" does not exist"
	exit 1
fi

# Extract & verify the .zip contents
TMP_PKG_EXTRACT_DIR="build/tmp/_wzextract"
rm -rf "${TMP_PKG_EXTRACT_DIR}"
if [ ! -d "${TMP_PKG_EXTRACT_DIR}" ]; then
	mkdir -p "${TMP_PKG_EXTRACT_DIR}"
fi
unzip -qq "${BUILT_WMIT_ZIP}" -d "${TMP_PKG_EXTRACT_DIR}"
cd "${TMP_PKG_EXTRACT_DIR}"
if [ ! -d "WMIT.app" ]; then
	echo "ERROR: \"WMIT.app\" was not extracted from \"${BUILT_WMIT_ZIP}\""
	exit 1
fi
# For debugging purposes, output some information about the generated "WMIT.app" (inside the .zip)
echo "Generated \"WMIT.app\""
generated_infoplist_location="WMIT.app/Contents/Info.plist"
generated_versionnumber=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" "${generated_infoplist_location}")
echo "  -> Version Number (CFBundleShortVersionString): ${generated_versionnumber}"
generated_buildnumber=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "${generated_infoplist_location}")
echo "  -> Build Number (CFBundleVersion): ${generated_buildnumber}"
generated_minimumsystemversion=$(/usr/libexec/PlistBuddy -c "Print LSMinimumSystemVersion" "${generated_infoplist_location}")
echo "  -> Minimum macOS (LSMinimumSystemVersion): ${generated_minimumsystemversion}"
codesign_verify_result=$(codesign --verify --deep --strict --verbose=2 "WMIT.app" 2>&1)
echo "  -> codesign --verify --deep --strict --verbose=2 \"WMIT.app\""
if [ -n "${codesign_verify_result}" ]; then
	while read -r line; do
		echo "     $line"
	done <<< "$codesign_verify_result"
else
	echo "     (No output?)"
fi
cd - > /dev/null
