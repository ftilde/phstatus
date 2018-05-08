SET(APPLICATION_NAME phstatus)
SET(APPLICATION_VERSION 0.1.0)
SET(DEFAULT_CONFIG_FILE_NAME ${APPLICATION_NAME}.yaml)


project(${APPLICATION_NAME} CXX)

#Might be able to set min version lower
cmake_minimum_required(VERSION 3.4)
cmake_policy(SET CMP0037 OLD)


add_definitions(-Wall)

SET(CORE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
SET(CORE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
FILE(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

###################################################################################
# Plugins
###################################################################################
SET(plugin_base_dir plugins)
# Prepare plugin information insertion
SET(PLUGIN_INCLUDE_LIST "")
SET(PLUGIN_LOAD_LIST "")

FILE(GLOB plugin_list ${plugin_base_dir}/*/*.cmake)
FOREACH(plugin_file ${plugin_list})
  GET_FILENAME_COMPONENT(plugin_dir ${plugin_file} DIRECTORY)
  GET_FILENAME_COMPONENT(plugin_name ${plugin_dir} NAME)
  STRING(TOUPPER ${plugin_name} plugin_name_upper)
  message(STATUS "Found plugin ${plugin_name}")

  SET(PLUGIN_${plugin_name_upper} ON CACHE BOOL "Builtin plugin: ${plugin_name}")
  IF(PLUGIN_${plugin_name_upper})
    message(STATUS " -> enabled")
    INCLUDE(${plugin_file})

    # Collect plugin load code
    SET(PLUGIN_INCLUDE_LIST "${PLUGIN_INCLUDE_LIST}#define PLUGIN_NAME ${plugin_name}\n")
    FOREACH(plugin_header ${PLUGIN_HEADERS})
      SET(header_path ${plugin_dir}/${plugin_header})
      SET(PLUGIN_INCLUDE_LIST "${PLUGIN_INCLUDE_LIST}#include \"${header_path}\"\n")
    ENDFOREACH(plugin_header ${PLUGIN_HEADERS})
    SET(PLUGIN_INCLUDE_LIST "${PLUGIN_INCLUDE_LIST}#undef PLUGIN_NAME\n")
    SET(PLUGIN_LOAD_LIST "${PLUGIN_LOAD_LIST}TRY_LOAD_PLUGIN(${plugin_name})\n")

    # Collect plugin source files
    FOREACH(plugin_source ${PLUGIN_SOURCES})
      SET(source_path ${plugin_dir}/${plugin_source})
      SET_SOURCE_FILES_PROPERTIES(${source_path} PROPERTIES COMPILE_DEFINITIONS "PLUGIN_NAME=${plugin_name};BUILTIN_PLUGIN")
      LIST(APPEND STATUS_PLUGIN_SOURCES ${source_path})
    ENDFOREACH(plugin_source ${PLUGIN_SOURCES})
  ELSE()
    message(STATUS " -> disabled")
  ENDIF()
ENDFOREACH(plugin_file ${plugin_list})

# Finalize plugin information insertion
SET(DEFAULT_BINARY_PLUGIN_PATH "" CACHE PATH "Default path for binary plugins")
SET(DEFAULT_SHELL_PLUGIN_PATH "${CMAKE_CURRENT_LIST_DIR}/shellplugins" CACHE PATH "Default path for shell plugins")
CONFIGURE_FILE(${CORE_SOURCE_DIR}/pluginloader.cpp.in ${CORE_SOURCE_DIR}/pluginloader.cpp)

# Pass build info to header files
CONFIGURE_FILE(${CORE_INCLUDE_DIR}/build_config.h.in ${CORE_INCLUDE_DIR}/build_config.h)

# Find core source files
FILE(GLOB CORE_SOURCE_FILES ${CORE_SOURCE_DIR}/*.cpp)

# Set include path for core include files
include_directories(${CORE_INCLUDE_DIR})

include(FindPkgConfig)

SET(LINK_LIBRARIES_LIST "")

###################################################################################
# yaml-cpp
###################################################################################
FIND_PACKAGE(YAML-CPP REQUIRED)
IF(NOT YAML-CPP_FOUND)
  message(FATAL_ERROR "yaml-cpp not found")
ELSE()
  message(STATUS "yaml-cpp found: ${YAML_CPP_LIBRARIES}")
ENDIF()
INCLUDE_DIRECTORIES(${YAML_CPP_INCLUDE_DIR})
LINK_DIRECTORIES(${YAML_CPP_LIBRARY_DIRS})
LIST(APPEND LINK_LIBRARIES_LIST ${YAML_CPP_LIBRARIES})

###################################################################################
# pthreads
###################################################################################
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
LIST(APPEND LINK_LIBRARIES_LIST Threads::Threads)

###################################################################################
# boost
###################################################################################
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem program_options)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
LIST(APPEND LINK_LIBRARIES_LIST ${Boost_LIBRARIES})

# TODO make these conditional
###################################################################################
# curl
###################################################################################
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config not found.")
else()
  pkg_check_modules(LIBCURL REQUIRED libcurl)
  if(LIBCURL_FOUND)
    message(STATUS "libcurl found")
    include_directories(${LIBCURL_INCLUDE_DIRS})
    link_directories(${LIBCURL_LIBRARY_DIRS})
    LIST(APPEND LINK_LIBRARIES_LIST ${LIBCURL_LIBRARIES})
  else()
    message(FATAL_ERROR "libcurl not found")
  endif()
endif()

###################################################################################
# pulseaudio
###################################################################################
FIND_PACKAGE(PulseAudio REQUIRED)
IF(NOT PULSEAUDIO_FOUND)
  message(FATAL_ERROR "pulseaudio not found")
ELSE()
  message(STATUS "pulseaudio found: ${PULSEAUDIO_LIBRARY}")
ENDIF()
INCLUDE_DIRECTORIES(${PULSEAUDIO_INCLUDE_DIR})
LINK_DIRECTORIES(${PULSEAUDIO_LIBRARY_DIRS})
LIST(APPEND LINK_LIBRARIES_LIST ${PULSEAUDIO_LIBRARY})

###################################################################################
# pulseaudio
###################################################################################
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config not found.")
else()
  pkg_check_modules(UPOWER-GLIB REQUIRED upower-glib)
  if(LIBCURL_FOUND)
    message(STATUS "upower-glib found")
    include_directories(${UPOWER-GLIB_INCLUDE_DIRS})
    link_directories(${UPOWER-GLIB_LIBRARY_DIRS})
    LIST(APPEND LINK_LIBRARIES_LIST ${UPOWER-GLIB_LIBRARIES})
  else()
    message(FATAL_ERROR "upower-glib not found")
  endif()
endif()

###################################################################################
# i3ipc_glib
###################################################################################
# TODO: Move this to an cmake module?
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config not found.")
else()
  pkg_check_modules(I3IPC_GLIB REQUIRED i3ipc-glib-1.0)
  if(I3IPC_GLIB_FOUND)
    message(STATUS "i3ipc-glib found")
    include_directories(${I3IPC_GLIB_INCLUDE_DIRS})
    link_directories(${I3IPC_GLIB_LIBRARY_DIRS})
    message(STATUS "i3ipc lib dirs: ${I3IPC_GLIB_LIBRARY_DIRS}")
    LIST(APPEND LINK_LIBRARIES_LIST ${I3IPC_GLIB_LIBRARIES})
    message(STATUS "i3ipc libs: ${I3IPC_GLIB_LIBRARIES}")
  else()
    message(FATAL_ERROR "i3ipc-glib not found")
  endif()
endif()


# finally add the executable (AFTER setting link_directories!
add_executable(${APPLICATION_NAME} ${CORE_SOURCE_FILES} ${STATUS_PLUGIN_SOURCES} bars/lemonbar.cpp)
set_property(TARGET ${APPLICATION_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${APPLICATION_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# and NOW set the target_link_libraries
target_link_libraries(${APPLICATION_NAME} ${LINK_LIBRARIES_LIST})
