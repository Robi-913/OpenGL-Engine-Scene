cmake_minimum_required(VERSION 3.28)
project(OpenGLProjectCorev1)

# Setează standardul C++
set(CMAKE_CXX_STANDARD 23)

# Include directoarele pentru header files (GL, GLFW, GLM)
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLdevlibs/include")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLdevlibs/")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLProjectCorev1/shaders")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLProjectCorev1/models")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLProjectCorev1/objects")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLProjectCorev1/skybox")
include_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLProjectCorev1/scena")



# Setează directorul de link în funcție de tipul de build (Debug/Release)
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    link_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLdevlibs/lib/Debug")
else()
    link_directories("C:/Users/Robert/Documents/facultate/anul3/pg/OpenGLdevlibs/lib/Release")
endif()

# Creează executabilul pentru proiectul tău
add_executable(OpenGLProjectCorev1 main.cpp
        Shader.cpp
        Shader.hpp
        Camera.cpp
        Camera.hpp
        Mesh.cpp
        Mesh.hpp
        Model3D.cpp
        Model3D.hpp
        stb_image.cpp
        stb_image.h
        tiny_obj_loader.cpp
        tiny_obj_loader.h
        Window.cpp
        Window.h
        SkyBox.cpp
        SkyBox.hpp
        )
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)


# Leagă librăriile necesare, în funcție de tipul de build (Debug/Release)
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(OpenGLProjectCorev1 libglew32d.lib glfw3.lib opengl32.lib)
else()
    target_link_libraries(OpenGLProjectCorev1 libglew32.lib glfw3.lib opengl32.lib)
endif()

set(ENV{__NV_PRIME_RENDER_OFFLOAD} 1)
set(ENV{__GLX_VENDOR_LIBRARY_NAME} nvidia)


