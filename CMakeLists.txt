cmake_minimum_required(VERSION 3.10)
project(AlchemistPotionLab CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lib
)

set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Texture.cpp
    src/SloshPhysics.cpp
    src/LabEnvironment.cpp
    src/PotionSystem.cpp
    src/ParticleSystem.cpp
    src/UIRenderer.cpp
    src/Scene.cpp
)

set(HEADERS
    include/math3d.h
    include/BitmapFont.h
    include/Shader.h
    include/Camera.h
    include/Mesh.h
    include/Texture.h
    include/SloshPhysics.h
    include/LabEnvironment.h
    include/PotionSystem.h
    include/ParticleSystem.h
    include/UIRenderer.h
    include/Scene.h
)

add_executable(AlchemistPotionLab ${SOURCES} ${HEADERS})

target_link_libraries(AlchemistPotionLab
    freeglut
    glew32
    opengl32
)

# Post-build copy DLLs and shaders to output directory
add_custom_command(TARGET AlchemistPotionLab POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/freeglut.dll"
        "$<TARGET_FILE_DIR:AlchemistPotionLab>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/bin/glew32.dll"
        "$<TARGET_FILE_DIR:AlchemistPotionLab>/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:AlchemistPotionLab>/shaders"
)
