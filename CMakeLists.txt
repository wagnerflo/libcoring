cmake_minimum_required( VERSION 3.15 )
project( libcovent )

include( GNUInstallDirs )
include( CMakePackageConfigHelpers )

find_package( PkgConfig REQUIRED )
find_package ( headerfu
  COMPONENTS iofu structfu
  REQUIRED
  PATHS ext/headerfu )

pkg_search_module( LIBURING REQUIRED liburing )

set ( EVLOOPS uring )

foreach( EVLOOP ${EVLOOPS} )
  configure_file(
    include/covent/evloops/EVLOOP.hh.in
    include/covent/evloops/${EVLOOP}.hh
  )
  set(
    EVLOOPS_INCLUDE
    "${EVLOOPS_INCLUDE}#include <covent/evloops/${EVLOOP}.hh>\n"
  )
endforeach()

configure_file(
  include/covent/evloops/evloops.hh.in
  include/covent/evloops/evloops.hh
)

add_library(
  covent SHARED
  src/base.cc
  src/event_loop.cc
  src/exceptions.cc
  src/uring/awaiters.cc
  src/uring/evloop.cc
)

target_compile_features( covent PUBLIC cxx_std_20 )
target_compile_options( covent PUBLIC
  ${LIBURING_CFLAGS_OTHER}
  -fcoroutines
)

target_include_directories(
  covent PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${LIBURING_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries( covent PUBLIC
  ${LIBURING_LIBRARIES}
)

target_link_libraries( covent PRIVATE
  headerfu::iofu
  headerfu::structfu
)

install(
  TARGETS covent
  EXPORT covent_targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES include/covent.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT covent_targets
  FILE covent-targets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/covent
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/covent-config.cmake.in
  ${CMAKE_CURRENT_LIST_DIR}/covent-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/covent
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/covent-config-version.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion
)

install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/covent-config.cmake
        ${CMAKE_CURRENT_LIST_DIR}/covent-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/covent
)

export(
  EXPORT covent_targets
  FILE "${CMAKE_CURRENT_LIST_DIR}/covent-targets.cmake"
)
