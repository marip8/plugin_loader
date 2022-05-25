cmake_minimum_required(VERSION 3.10.0)

# Extract package name and version from package.xml
find_package(ros_industrial_cmake_boilerplate REQUIRED)
extract_package_metadata(pkg)
project(${pkg_extracted_name} VERSION ${pkg_extracted_version} LANGUAGES CXX)

find_package(Boost REQUIRED filesystem)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                     "$<INSTALL_INTERFACE:include>")
target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE Boost::boost
            Boost::filesystem
            ${CMAKE_DL_LIBS})
target_cxx_version(${PROJECT_NAME} INTERFACE VERSION 17)
target_clang_tidy(${PROJECT_NAME} ENABLE ${PLUGIN_LOADER_ENABLE_CLANG_TIDY})

# Install the library
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})

# Unit tests
if(${PLUGIN_LOADER_ENABLE_TESTING})
  enable_testing()
  add_run_tests_target(ENABLE ${PLUGIN_LOADER_ENABLE_RUN_TESTING})
  add_subdirectory(test)
endif()

# Package configuration
configure_package(NAMESPACE ${PROJECT_NAME})
