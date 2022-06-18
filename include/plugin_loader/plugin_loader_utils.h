/*
 * Software License Agreement (Apache License)
 *
 * Copyright (c) 2022, Southwest Research Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <boost/config.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/dll/import.hpp>
#include <boost/dll/alias.hpp>
#include <boost/dll/import_class.hpp>
#include <iostream>
#include <string>
#include <set>
#include <vector>

namespace plugin_loader
{
class PluginLoaderException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

/**
 * @brief Give library name without prefix and suffix it will return the library name with the prefix and suffix
 *
 * * For instance, for a library_name like "boost" it returns :
 * - path/to/libboost.so on posix platforms
 * - path/to/libboost.dylib on OSX
 * - path/to/boost.dll on Windows
 *
 * @todo When support is dropped for 18.04 switch to using boost::dll::shared_library::decorate
 * @param library_name The library name without prefix and suffix
 * @param library_directory The library directory, if empty it just returns the decorated library name
 * @return The library name or path with prefix and suffix
 */
static inline std::string decorate(const std::string& library_name, const std::string& library_directory = "")
{
  boost::filesystem::path sl;
  if (library_directory.empty())
    sl = boost::filesystem::path(library_name);
  else
    sl = boost::filesystem::path(library_directory) / library_name;

  boost::filesystem::path actual_path =
      (std::strncmp(sl.filename().string().c_str(), "lib", 3) != 0 ?
           boost::filesystem::path((sl.has_parent_path() ? sl.parent_path() / L"lib" : L"lib").native() +
                                   sl.filename().native()) :
           sl);

  actual_path += boost::dll::shared_library::suffix();
  return actual_path.string();
}

/**
 * @brief Extract list form environment variable
 * @param env_variable The environment variable name to extract list from
 * @return A list extracted from variable name
 */
static inline std::set<std::string> parseEnvironmentVariableList(const std::string& env_variable)
{
  std::set<std::string> list;
  char* env_var = std::getenv(env_variable.c_str());
  if (env_var == nullptr)  // Environment variable not found
    return list;

  std::string evn_str = std::string(env_var);
  boost::split(list, evn_str, boost::is_any_of(":"), boost::token_compress_on);
  return list;
}

/**
 * @brief Get all available search paths
 * @param search_libraries_env The environment variable containing plugin search paths
 * @param existing_search_libraries A list of existing search paths
 * @return A list of search paths
 */
static inline std::set<std::string> getAllSearchPaths(const std::string& search_paths_env,
                                               const std::set<std::string>& existing_search_paths)
{
  // Check for environment variable to override default library
  if (!search_paths_env.empty())
  {
    std::set<std::string> search_paths = parseEnvironmentVariableList(search_paths_env);
    search_paths.insert(existing_search_paths.begin(), existing_search_paths.end());
    return search_paths;
  }

  return existing_search_paths;
}

/**
 * @brief Get all available library names
 * @param search_libraries_env The environment variable containing plugin library names
 * @param existing_search_libraries A list of existing library names without the prefix or suffix that contain plugins
 * @return A list of library names without the prefix or suffix that contain plugins
 */
static inline std::set<std::string> getAllLibraryNames(const std::string& search_libraries_env,
                                                const std::set<std::string>& existing_search_libraries)
{
  // Check for environment variable to override default library
  if (!search_libraries_env.empty())
  {
    std::set<std::string> search_libraries = parseEnvironmentVariableList(search_libraries_env);
    search_libraries.insert(existing_search_libraries.begin(), existing_search_libraries.end());
    return search_libraries;
  }

  return existing_search_libraries;
}

/**
 * @brief Attempt to load library give library name and directory
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return A shared library
 */
static inline boost::dll::shared_library loadLibrary(const std::string& library_name,
                                              const std::string& library_directory = "")
{
  boost::system::error_code ec;
  boost::dll::shared_library lib;
  if (library_directory.empty())
  {
    boost::filesystem::path sl(library_name);
    boost::dll::load_mode::type mode =
        boost::dll::load_mode::append_decorations | boost::dll::load_mode::search_system_folders;
    lib = boost::dll::shared_library(sl, ec, mode);
  }
  else
  {
    boost::filesystem::path sl = boost::filesystem::path(library_directory) / library_name;
    lib = boost::dll::shared_library(sl, ec, boost::dll::load_mode::append_decorations);
  }

  // Check if it failed to find or load library
  if (ec)
    throw plugin_loader::PluginLoaderException(
        "Failed to find or load library: " + decorate(library_name, library_directory) +
        " with error: " + ec.message());

  return lib;
}

/**
 * @brief Check if the symbol is available in the library_name searching system folders for library
 * @details The symbol name is the alias provide when calling TESSERACT_ADD_PLUGIN
 * @param symbol_name The symbol to create a shared instance of
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return True if the symbol exists, otherwise false
 */
static inline bool isClassAvailable(const std::string& symbol_name, const std::string& library_name,
                             const std::string& library_directory = "")
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);
  return lib.has(symbol_name);
}

/**
 * @brief Get a list of available symbols under the provided section
 * @param section The section to search for available symbols
 * @param libraries The library to search for symbols
 * @return A list of symbols if they exist.
 */
static inline std::vector<std::string> getAllAvailableClasses(const std::string& section,
                                                       const std::vector<boost::dll::fs::path>& libraries)
{
  std::vector<std::string> classes;
  for (const auto& library : libraries)
  {
    boost::dll::library_info inf(library);
    std::vector<std::string> exports = inf.symbols(section);
    classes.insert(classes.end(), exports.begin(), exports.end());
  }

  return classes;
}

/**
 * @brief Get a list of available symbols under the provided section
 * @param section The section to search for available symbols
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return A list of symbols if they exist.
 */
static inline std::vector<std::string> getAllAvailableClasses(const std::string& section, const std::string& library_name,
                                                       const std::string& library_directory = "")
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(lib.location());

  // Getting symbols exported from he provided section
  return inf.symbols(section);
}

/**
 * @brief Get a list of available sections
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return A list of sections if they exist.
 */
static inline std::vector<std::string> getAllAvailableSections(const std::string& library_name,
                                                               const std::string& library_directory,
                                                               bool include_hidden = false)
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(lib.location());

  // Getting section from library
  std::vector<std::string> sections = inf.sections();

  auto search_fn = [include_hidden](const std::string& section) {
    if (section.empty())
      return true;

    if (include_hidden)
      return false;

    return (section.substr(0, 1) == ".");
  };

  sections.erase(std::remove_if(sections.begin(), sections.end(), search_fn), sections.end());
  return sections;
}

}  // namespace plugin_loader
