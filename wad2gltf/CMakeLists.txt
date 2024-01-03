# Dependencies
include(FetchContent)

set(EARCUT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EARCUT_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(EARCUT_BUILD_VIZ OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY  https://github.com/CLIUtils/CLI11.git
    GIT_TAG         v2.3.2 
)

FetchContent_Declare(
    earcut
    GIT_REPOSITORY  https://github.com/mapbox/earcut.hpp.git
    GIT_TAG         bf42168d1ae117091e45db5ac3464f2d02c9d9fb
)

FetchContent_Declare(
    fastgltf
    GIT_REPOSITORY  https://github.com/spnda/fastgltf.git
    GIT_TAG         gltf_composing
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         7882684a2cd69005fb57001c17a332899621e2be
)

FetchContent_MakeAvailable(cli11 earcut fastgltf glm)

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_GetProperties(stb)
if(stb_POPULATED)
    message("STB automatically populated")

else()
    FetchContent_Populate(stb)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE 
        "${stb_SOURCE_DIR}"
    )
endif()

file(GLOB_RECURSE WAD2GLTF_SOURCE CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
        )

add_executable (wad2gltf ${WAD2GLTF_SOURCE})
target_link_libraries(wad2gltf PUBLIC
    CLI11::CLI11
    earcut_hpp::earcut_hpp
    fastgltf
    glm
    stb
)

# TODO: Add tests and install targets if needed.

#######################
# Generate VS filters #
#######################
foreach(SOURCE_FILE IN LISTS WAD2GLTF_SOURCE)
    get_filename_component(SOURCE_PATH "${SOURCE_FILE}" PATH)
    cmake_path(RELATIVE_PATH SOURCE_PATH BASE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        OUTPUT_VARIABLE SOURCE_PATH_RELATIVE)
        
    source_group("cpp/${SOURCE_PATH_RELATIVE}" FILES "${SOURCE_FILE}")
endforeach()
