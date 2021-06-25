cmake_minimum_required(VERSION 3.20)
project(hyperxsearch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FindPkgConfig)

# tclap
pkg_check_modules(tclap REQUIRED IMPORTED_TARGET tclap)
  get_target_property(
  TCLAP_INC
  PkgConfig::tclap
  INTERFACE_INCLUDE_DIRECTORIES
)

# libprim
pkg_check_modules(libprim REQUIRED IMPORTED_TARGET libprim)
  get_target_property(
  LIBPRIM_INC
  PkgConfig::libprim
  INTERFACE_INCLUDE_DIRECTORIES
)

# libstrop
pkg_check_modules(libstrop REQUIRED IMPORTED_TARGET libstrop)
  get_target_property(
  LIBSTROP_INC
  PkgConfig::libstrop
  INTERFACE_INCLUDE_DIRECTORIES
)

# libgrid
pkg_check_modules(libgrid REQUIRED IMPORTED_TARGET libgrid)
  get_target_property(
  LIBGRID_INC
  PkgConfig::libgrid
  INTERFACE_INCLUDE_DIRECTORIES
)

add_executable(
  hyperxsearch
  ${PROJECT_SOURCE_DIR}/src/main.cc
  ${PROJECT_SOURCE_DIR}/src/search/RouterChannelCount.cc
  ${PROJECT_SOURCE_DIR}/src/search/CalculatorFactory.cc
  ${PROJECT_SOURCE_DIR}/src/search/Calculator.cc
  ${PROJECT_SOURCE_DIR}/src/search/Engine.cc
  ${PROJECT_SOURCE_DIR}/src/search/Calculator.h
  ${PROJECT_SOURCE_DIR}/src/search/Engine.h
  ${PROJECT_SOURCE_DIR}/src/search/RouterChannelCount.h
  ${PROJECT_SOURCE_DIR}/src/search/CalculatorFactory.h
  )

target_include_directories(
  hyperxsearch
  PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${TCLAP_INC}
  ${LIBPRIM_INC}
  ${LIBSTROP_INC}
  ${LIBGRID_INC}
  )

target_link_libraries(
  hyperxsearch
  PkgConfig::tclap
  PkgConfig::libprim
  PkgConfig::libstrop
  PkgConfig::libgrid
  )

include(GNUInstallDirs)

install(
  TARGETS
  hyperxsearch
  )

