cmake_minimum_required(VERSION 3.25)

option(IO1_WITH_TESTS
       "Add a target to build and run unit tests. Requires doctest." ON)

if(IO1_WITH_TESTS)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(
  io1-accounting
  VERSION 0.1.0
  DESCRIPTION
    "A set of classes to support personnal accouting applications."
  HOMEPAGE_URL "https://github.com/yobeonline/io1-accounting"
  LANGUAGES CXX)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} STATIC
		include/io1/statement.hpp
		src/statement.cpp
		include/io1/entry.hpp
		src/entry.cpp
		include/io1/listing.hpp
		src/listing.cpp
		include/io1/account.hpp
		src/account.cpp
		include/io1/archived_listing.hpp
		src/archived_listing.cpp
		include/io1/accounting_exception.hpp
		src/accounting_exception.cpp
		include/io1/date_formatter.hpp
		src/date_formatter.cpp
)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

find_package(io1 REQUIRED COMPONENTS money)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PUBLIC io1::money Boost::filesystem Boost::boost)

add_library(io1::accounting ALIAS ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/io1 DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/config.cmake.in" "${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/io1/cmake)

install(
  EXPORT ${PROJECT_NAME}
  NAMESPACE io1::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/io1/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/io1/cmake)

if(IO1_WITH_TESTS)
  include(CTest)

  find_package(doctest CONFIG REQUIRED)

  add_executable(test_${PROJECT_NAME} test/test_listing.cpp test/test_account.cpp test/test_entry.cpp test/test_statement.cpp)
  target_link_libraries(test_${PROJECT_NAME} PRIVATE io1::accounting
                                                     doctest::doctest)

  add_test(
    NAME test_${PROJECT_NAME}
    COMMAND test_${PROJECT_NAME} --reporters=junit
            --out=junit_test_${PROJECT_NAME}.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
