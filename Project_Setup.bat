@echo off
@Rem Christopher Edmunds
@Rem Project_Setup
@REM Rem This file is for setting up VCPKG and ImGui submodules
@REM Rem This script will clone ImGui and copy the needed imgui/backends files to the imgui directory


IF NOT EXIST .\imgui\NUL (echo "Cloning imgui submodule" && git clone https://github.com/ocornut/imgui.git)

IF NOT EXIST .\imgui\NUL (echo "ERROR::IMGUI: Error cloning submodule")

echo "Fetching recent updates from imgui"
cd imgui && git fetch && cd ..

set back=imgui\backends\

@Rem Add all needed files from backend to this list and run the script to update. 
set required_files=%back%imgui_impl_glfw.h %back%imgui_impl_glfw.cpp %back%imgui_impl_opengl3.h %back%imgui_impl_opengl3.cpp %back%imgui_impl_opengl3_loader.h imgui\misc\cpp\imgui_stdlib.h imgui\misc\cpp\imgui_stdlib.cpp


echo "Copying required files from imgui/backends/ to imgui/"

(for %%f in (%required_files%) do (
    echo "copying %%f to imgui\"
    copy "%%f" "imgui\"
))

echo "Copying Done!"

IF NOT EXIST .\vcpkg\NUL (echo "Cloning vcpkg submodule" && git clone https://github.com/microsoft/vcpkg.git && .\vcpkg\bootstrap-vcpkg.bat)

IF NOT EXIST .\vcpkg\NUL (echo "ERROR::VCPKG: Error cloning submodule")

echo "Setup Complete"