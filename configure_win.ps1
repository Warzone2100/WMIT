# configure_win.ps1
#
# To successfully run this script:
# - Ensure that Qt5 is installed and in the PATH
# - Ensure that the environment is set for the appropriate Visual Studio configuration
#   (ex. Use the appropriate Start Menu "x86/x64 Native Tools Command Prompt for VS 20XX" shortcut)
#
Write-Output "QT5_DIR: $env:QT5_DIR"

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
qmake -t vclib ..\..\..\3rdparty\libQGLViewer\QGLViewer\QGLViewer.pro -spec win32-msvc
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
$target_generator="Visual Studio 15 2017"
if (Test-Path 'env:CmakeGeneratorToUse')
{
	$target_generator="$env:CmakeGeneratorToUse"
}
cmake -G "$target_generator" -A "$target_architecture" -DCMAKE_PREFIX_PATH="$env:QT5_DIR" ../../
popd
popd
