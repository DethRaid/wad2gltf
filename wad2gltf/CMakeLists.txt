# Dependencies
include(FetchContent)

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

FetchContent_MakeAvailable(glm tinygltf)

# Add source to this project's executable.
add_executable (wad2gltf "wad2gltf.cpp" "wad2gltf.h" "wad_loader.cpp" "map_reader.cpp" "gltf_export.cpp" )
target_link_libraries(wad2gltf PUBLIC
    glm
    tinygltf
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET wad2gltf PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
