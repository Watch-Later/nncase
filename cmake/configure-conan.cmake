function(_SET_CONANOPT OUT_VAR OPT_NAME OPT_VALUE)
    if (${OPT_VALUE})
        set(PY_OPT_VALUE "True")
    else ()
        set(PY_OPT_VALUE "False")
    endif ()
    set(${OUT_VAR} "${${OUT_VAR}};${OPT_NAME}=${PY_OPT_VALUE}" PARENT_SCOPE)
endfunction()

function(_SET_CONANSETTING OUT_VAR SET_NAME SET_VALUE)
    set(${OUT_VAR} "${${OUT_VAR}};${SET_NAME}=${SET_VALUE}" PARENT_SCOPE)
endfunction()

_SET_CONANOPT(CONAN_OPTS "runtime" BUILDING_RUNTIME)
_SET_CONANOPT(CONAN_OPTS "tests" BUILD_TESTING)
_SET_CONANOPT(CONAN_OPTS "python" BUILD_PYTHON_BINDING)
_SET_CONANOPT(CONAN_OPTS "openmp" ENABLE_OPENMP)
_SET_CONANOPT(CONAN_OPTS "vulkan_runtime" ENABLE_VULKAN_RUNTIME)
_SET_CONANOPT(CONAN_OPTS "halide" ENABLE_HALIDE)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    if (BUILDING_RUNTIME)
        set (CMAKE_CXX_STANDARD 14)
    else ()
        set (CMAKE_CXX_STANDARD 20)
    endif ()
endif ()

_SET_CONANSETTING(CONAN_SETTINGS "compiler.cppstd" ${CMAKE_CXX_STANDARD})
