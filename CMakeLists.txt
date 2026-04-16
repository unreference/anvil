cmake_minimum_required(VERSION 4.2.0)
project(Anvil VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------- #
# Version header (stamped at configure time)
# --------------------------------------------------------------------------- #

# CI passes the real build number; local builds default to 0.
set(ANVIL_BUILD_NUMBER "0" CACHE STRING "Auto-incremented by CI.")

configure_file(
        ${CMAKE_SOURCE_DIR}/Include/Common/Version.hpp.in
        ${CMAKE_BINARY_DIR}/Generated/Common/Version.hpp
)

# --------------------------------------------------------------------------- #
# Sources
# --------------------------------------------------------------------------- #

set(ANVIL_SOURCES
        Source/Main.cpp
        Source/Gpu/Renderer.cpp
        Source/Gpu/SwapChain.cpp
        Source/Gpu/VulkanContext.cpp
        Source/Platform/Win32Window.cpp
        Source/Gameplay/FreeCamera.cpp
        Source/Core/Game.cpp
        Source/Gpu/Buffer.cpp
        Source/Gpu/DescriptorAllocator.cpp
        Source/Gpu/FrameState.cpp
        Source/Gpu/Texture.cpp
        Source/Gpu/StaticMeshPipeline.cpp
)

set(ANVIL_HEADERS
        Include/Common/Types.hpp
        Include/Gpu/Renderer.hpp
        Include/Gpu/SwapChain.hpp
        Include/Gpu/VulkanContext.hpp
        Include/Platform/Win32Window.hpp
        Include/Math/Vec3.hpp
        Include/Math/Vec4.hpp
        Include/Math/Mat4.hpp
        Include/Math/Transforms.hpp
        Include/Gameplay/CameraInput.hpp
        Include/Gameplay/FreeCamera.hpp
        Include/Math/Vec2.hpp
        Include/Core/Game.hpp
        Include/Gpu/Buffer.hpp
        Include/Gpu/DescriptorAllocator.hpp
        Include/Gpu/FrameState.hpp
        Include/Gpu/Texture.hpp
        Include/Gpu/Checkerboard.hpp
        Include/Gpu/MaterialState.hpp
        Source/Gpu/MaterialState.cpp
        Include/Gpu/StaticVertex.hpp
        Include/Gpu/Primitives.hpp
        Include/Gpu/StaticMeshPipeline.hpp
)

add_executable(Anvil ${ANVIL_SOURCES} ${ANVIL_HEADERS})

target_include_directories(Anvil PRIVATE
        ${CMAKE_SOURCE_DIR}/Include
        ${CMAKE_BINARY_DIR}/Generated
)

# --------------------------------------------------------------------------- #
# Vulkan
# --------------------------------------------------------------------------- #

find_package(Vulkan REQUIRED)
target_link_libraries(Anvil PRIVATE Vulkan::Vulkan)

# --------------------------------------------------------------------------- #
# Shader compilation (GLSL -> SPIR-V -> embedded C++ header)
# --------------------------------------------------------------------------- #

if (NOT Vulkan_GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Install the Vulkan SDK.")
endif ()

function(anvil_compile_shader SHADER_FILE)
    get_filename_component(NAME_WE ${SHADER_FILE} NAME_WE)
    get_filename_component(EXT ${SHADER_FILE} LAST_EXT)

    # ".vert" -> "Vert" for PascalCase array name.
    string(SUBSTRING "${EXT}" 1 -1 STAGE)
    string(SUBSTRING "${STAGE}" 0 1 STAGE_FIRST)
    string(TOUPPER "${STAGE_FIRST}" STAGE_FIRST)
    string(SUBSTRING "${STAGE}" 1 -1 STAGE_REST)
    set(ARRAY_NAME "${NAME_WE}${STAGE_FIRST}${STAGE_REST}")

    set(INPUT_GLSL ${CMAKE_SOURCE_DIR}/Resources/Shaders/${SHADER_FILE})
    set(OUTPUT_SPV ${CMAKE_BINARY_DIR}/Resources/Shaders/${SHADER_FILE}.spv)
    set(OUTPUT_HPP ${CMAKE_BINARY_DIR}/Generated/Resources/Shaders/${ARRAY_NAME}.hpp)

    # Ensure output directories exist.
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Resources/Shaders)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Generated/Resources/Shaders)

    add_custom_command(
            OUTPUT ${OUTPUT_SPV}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${OUTPUT_SPV} ${INPUT_GLSL}
            DEPENDS ${INPUT_GLSL}
            COMMENT "Compiling ${SHADER_FILE}"
    )

    add_custom_command(
            OUTPUT ${OUTPUT_HPP}
            COMMAND ${CMAKE_COMMAND}
            -DINPUT_SPV=${OUTPUT_SPV}
            -DOUTPUT_HPP=${OUTPUT_HPP}
            -DARRAY_NAME=${ARRAY_NAME}
            -P ${CMAKE_SOURCE_DIR}/CMake/CompileShader.cmake
            DEPENDS ${OUTPUT_SPV}
            COMMENT "Embedding ${SHADER_FILE} -> ${ARRAY_NAME}.hpp"
    )

    target_sources(Anvil PRIVATE ${OUTPUT_HPP})
endfunction()

# Shaders
anvil_compile_shader(Basic.vert)
anvil_compile_shader(Basic.frag)

# --------------------------------------------------------------------------- #
# Platform defines
# --------------------------------------------------------------------------- #

target_compile_definitions(Anvil PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        VK_USE_PLATFORM_WIN32_KHR
        UNICODE
        _UNICODE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

# --------------------------------------------------------------------------- #
# Compiler flags (MSVC)
# --------------------------------------------------------------------------- #

if (MSVC)
    # Shared across both configs.
    target_compile_options(Anvil PRIVATE
            /W4           # High warning level.
            /WX           # Warnings as errors.
            /permissive-  # Strict conformance mode.
            /sdl          # Security Development Lifecycle checks.
            /GR-          # No RTTI.
            /EHsc         # Standard C++ exception handling.
            /utf-8        # Source and execution charset.
    )

    # Debug
    target_compile_options(Anvil PRIVATE
            $<$<CONFIG:Debug>:
            /Zi         # Full debug info.
            /Od         # No optimization.
            /RTC1       # Runtime error checks (stack frame + uninitialized).
            >
    )

    # Release
    target_compile_options(Anvil PRIVATE
            $<$<CONFIG:Release>:
            /Zi         # Debug info for profiling and crash dumps.
            /O2         # Maximize speed.
            /Oi         # Enable intrinsic functions.
            /GL         # Whole program optimization.
            /Gy         # Function-level linking.
            >
    )

    # Release linker flags.
    target_link_options(Anvil PRIVATE
            $<$<CONFIG:Release>:
            /LTCG       # Link-time code generation.
            /OPT:REF    # Eliminate unreferenced code.
            /OPT:ICF    # Identical COMDAT folding.
            >
    )

    # Console subsystem.
    target_link_options(Anvil PRIVATE /SUBSYSTEM:CONSOLE)
endif ()
