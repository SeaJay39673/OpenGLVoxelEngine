cmake_minimum_required(VERSION 3.10.0)
project(OpenWorld VERSION 0.1.0 LANGUAGES C CXX)

file (GLOB IMGUI_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
)

add_executable(OpenWorld main.cpp ${IMGUI_SOURCES})

set_property(TARGET OpenWorld PROPERTY CXX_STANDARD 17)

set(RESOURCE_PATH "")
if(WIN32)
  set(RESOURCE_PATH "$<IF:$<CONFIG:Debug>,Debug,Release>")
  set(VCPKG_HOST_TRIPLET x64-windows-static)
  set(VCPKG_TARGET_TRIPLET x64-windows-static)
endif()

#############################################################
# Add Resources to build directory so program can access them
#############################################################
add_custom_target(copy_assets
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Resources ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_PATH}/Resources
)
add_dependencies(OpenWorld copy_assets)

############################################
# Inlucde VCPKG and find 3rd party libraries
############################################
include("./vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(OpenGL REQUIRED)
target_link_libraries(OpenWorld PRIVATE OpenGL::GL)

find_package(glad CONFIG REQUIRED)
target_link_libraries(OpenWorld PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OpenWorld PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(OpenWorld PRIVATE glm::glm)

find_package(Stb REQUIRED)
target_include_directories(OpenWorld PRIVATE ${Stb_INCLUDE_DIRS})

################################################################
# Set install paths so the installer can find required resources
################################################################

install(TARGETS OpenWorld RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include DESTINATION bin)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

