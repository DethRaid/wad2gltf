# CMakeList.txt : CMake project for wad2gltf, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (wad2gltf "wad2gltf.cpp" "wad2gltf.h" "wad_loader.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET wad2gltf PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
