# Dependencies
include(FetchContent)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY  https://github.com/CLIUtils/CLI11.git
    GIT_TAG          v2.3.2 
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         7882684a2cd69005fb57001c17a332899621e2be
)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY  https://github.com/syoyo/tinygltf.git
    GIT_TAG         v2.8.19
)

FetchContent_MakeAvailable(cli11 glm tinygltf)

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
    glm
    stb
    tinygltf
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
