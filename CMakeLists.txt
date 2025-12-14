cmake_minimum_required(VERSION 3.10)
project(MLFQ_Scheduler VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find FLTK (make it optional)
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
    pkg_check_modules(FLTK QUIET fltk1.3 fltk)
endif()

# If pkg-config fails, try alternative approach
if(NOT FLTK_FOUND)
    find_path(FLTK_INCLUDE_DIR
        NAMES FL/Fl.H
        PATHS /usr/include /usr/local/include /opt/include
        PATH_SUFFIXES FL fltk fltk-1.3 fltk-1.4
    )
    find_library(FLTK_LIBRARY
        NAMES fltk fltk13 fltk1.3
        PATHS /usr/lib /usr/local/lib /opt/lib /usr/lib/x86_64-linux-gnu
    )

    if(FLTK_INCLUDE_DIR AND FLTK_LIBRARY)
        set(FLTK_LIBRARIES ${FLTK_LIBRARY})
        set(FLTK_INCLUDE_DIRS ${FLTK_INCLUDE_DIR})
        set(FLTK_FOUND TRUE)
    endif()
endif()

# If still not found, try direct paths (specifically for Debian/Ubuntu systems)
if(NOT FLTK_FOUND)
    find_path(FLTK_INCLUDE_DIR NAMES FL/Fl.H PATHS /usr/include)
    find_library(FLTK_LIBRARY NAMES fltk PATHS /usr/lib/x86_64-linux-gnu)

    if(FLTK_INCLUDE_DIR AND FLTK_LIBRARY)
        set(FLTK_LIBRARIES ${FLTK_LIBRARY})
        set(FLTK_INCLUDE_DIRS ${FLTK_INCLUDE_DIR})
        set(FLTK_FOUND TRUE)
    endif()
endif()

# Core source files
set(CORE_SOURCES
    src/Process.cpp
    src/Queue.cpp
    src/MLFQScheduler.cpp
    src/Visualizer.cpp
    src/WebServer.cpp
)
set(MAIN_SOURCES ${CORE_SOURCES} src/main.cpp)

if(FLTK_FOUND)
    message(STATUS "FLTK found, building with GUI support")
    set(ALL_SOURCES ${CORE_SOURCES} src/FLTKVisualizer.cpp src/main.cpp)
    add_executable(mlfq_scheduler ${ALL_SOURCES})
    target_link_libraries(mlfq_scheduler ${FLTK_LIBRARIES})
    target_include_directories(mlfq_scheduler PRIVATE ${FLTK_INCLUDE_DIRS})
    target_compile_options(mlfq_scheduler PRIVATE ${FLTK_CFLAGS_OTHER})
    add_compile_definitions(FLTK_AVAILABLE)
else()
    message(WARNING "FLTK not found, building without GUI support")
    add_executable(mlfq_scheduler ${MAIN_SOURCES})
endif()


# For Windows, add console flag
if(WIN32)
    set_target_properties(mlfq_scheduler PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Installation
install(TARGETS mlfq_scheduler DESTINATION bin)
