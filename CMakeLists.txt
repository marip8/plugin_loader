cmake_minimum_required(VERSION 3.10.0)

# Extract package name and version from package.xml
find_package(ros_industrial_cmake_boilerplate REQUIRED)
extract_package_metadata(pkg)
project(${pkg_extracted_name} VERSION ${pkg_extracted_version} LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS filesystem)

option(ENABLE_CLANG_TIDY "Enables compilation with clang-tidy" OFF)
option(BUILD_TESTING "Enables compilation of unit tests" OFF)
option(ENABLE_RUN_TESTING "Enables running of unit tests as a part of the build" OFF)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                     "$<INSTALL_INTERFACE:include>")
target_link_libraries(${PROJECT_NAME} INTERFACE Boost::boost Boost::filesystem ${CMAKE_DL_LIBS})
target_cxx_version(${PROJECT_NAME} INTERFACE VERSION 17)
target_clang_tidy(${PROJECT_NAME} ENABLE ${ENABLE_CLANG_TIDY})

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})

# Unit tests
if(BUILD_TESTING)
  enable_testing()
  add_run_tests_target(ENABLE ${ENABLE_RUN_TESTING})
  add_subdirectory(test)
endif()

# Package configuration
configure_package(NAMESPACE ${PROJECT_NAME} TARGETS ${PROJECT_NAME})
