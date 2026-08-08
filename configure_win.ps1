# configure_win.ps1
#
# To successfully run this script:
# - Ensure that Qt (5 or 6) is installed and in the PATH
# - Ensure that the environment is set for the appropriate Visual Studio configuration
#   (ex. Use the appropriate Start Menu "x86/x64 Native Tools Command Prompt for VS 20XX" shortcut)
#
# install-qt-action v4 exports QT_ROOT_DIR for all Qt versions, and Qt5_DIR only
# for Qt5. Prefer the former so this works for both.
$QtPrefixPath = "$env:QT_ROOT_DIR"
if ([string]::IsNullOrWhitespace($QtPrefixPath))
{
    $QtPrefixPath = "$env:QT5_DIR"
}
Write-Output "Qt prefix path: $QtPrefixPath"

if ((Get-Command "qmake" -ErrorAction SilentlyContinue) -eq $null)
{
   throw "Unable to find qmake.exe in your PATH"
}

If(!(Test-Path 'env:VCToolsInstallDir'))
{
	Write-Warning "Did not find expected environment variables for Visual Studio configuration - this script may fail"
}

# Create the build directory
If(!(Test-Path build))
{
    md -Name build > $null
}
pushd build

$build_subdir="unknown"
# If $env:Platform is set, use it as a subdirectory
if (Test-Path 'env:Platform')
{
    $build_subdir="$env:Platform"
}
Write-Output "Using build subdir: $build_subdir"
If(!(Test-Path "$build_subdir"))
{
    md -Name "$build_subdir" > $null
}
pushd "$build_subdir"

# Compile libQGLViewer (using qmake)
Write-Output "Compile libQGLViewer";
If(!(Test-Path libQGLViewer))
{
    md -Name libQGLViewer > $null
}
pushd libQGLViewer
# libQGLViewer's QGLViewer/config.h includes <GL/glu.h> without pulling in
# <windows.h> first (unlike its own VRender/Types.h and VRender/Primitive.h,
# which do guard it). glu.h needs APIENTRY / WINGDIAPI from the Windows SDK
# headers, so without them MSVC reports a cascade of
#   glu.h(64,25): error C2146: syntax error: missing ';' before 'gluErrorString'
# Under Qt5 this was masked because <QOpenGLWidget> transitively included
# windows.h - Qt6 no longer does.
#
# Force-include windows.h for this sub-build only. NOMINMAX is required because
# several VRender sources call unqualified max(), which the windows.h macro
# would otherwise break. WMIT's own sources are unaffected - QtGLView.h includes
# <GL/glew.h> first, and GLEW defines APIENTRY itself.
#
# Present in every released libQGLViewer including 3.0.0, so bumping the
# submodule does not help (fixing config.h upstream would remove the need).
qmake -t vclib ..\..\..\3rdparty\libQGLViewer\QGLViewer\QGLViewer.pro -spec win32-msvc "QMAKE_CXXFLAGS+=/FIwindows.h" "DEFINES+=NOMINMAX" "DEFINES+=WIN32_LEAN_AND_MEAN"
# Get the latest-installed Windows 10 SDK
$Win10SDKVersions = (Get-ChildItem -Path 'HKLM:\Software\Wow6432Node\Microsoft\Windows Kits\Installed Roots' -Name)
$greatest_sdk_version="0.0.0.0"
foreach ($subkey in $Win10SDKVersions)
{
    If([System.Version]"$subkey" -gt [System.Version]"$greatest_sdk_version")
    {
        $greatest_sdk_version=$subkey
    }
}
Write-Output "Latest available Win10SDKVersion: $greatest_sdk_version"
Write-Output "msbuild QGLViewer.vcxproj /p:Configuration=Release ""/p:WindowsTargetPlatformVersion=$greatest_sdk_version"""
msbuild QGLViewer.vcxproj /p:Configuration=Release "/p:WindowsTargetPlatformVersion=$greatest_sdk_version"
popd
$env:QGLVIEWERROOT = "$(($pwd).path)\..\..\3rdparty\libQGLViewer"
Write-Output "QGLVIEWERROOT: $env:QGLVIEWERROOT"

# Use CMake to configure Visual Studio project
Write-Output "Generate Visual Studio project (using CMake)";
$target_architecture="Win32" # default (for x86)
if (Test-Path 'env:Platform')
{
	if ("$env:Platform" -eq "x86")
	{
		$target_architecture="Win32"
	}
	else
	{
		$target_architecture="$env:Platform"
	}
}
$target_generator="Visual Studio 17 2022"
if (Test-Path 'env:CmakeGeneratorToUse')
{
	$target_generator="$env:CmakeGeneratorToUse"
}
cmake -G "$target_generator" -A "$target_architecture" -DCMAKE_PREFIX_PATH="$QtPrefixPath" ../../
popd
popd
