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
#include <plugin_loader/plugin_loader.h>

#include <boost/algorithm/string.hpp>
#include <boost/config.hpp>
#include <boost/dll/import.hpp>
#include <boost/dll/alias.hpp>
#include <boost/dll/import_class.hpp>
#include <boost/dll/shared_library.hpp>
#include <iostream>

namespace
{
inline std::string decorate(const std::string& library_name, const std::string& library_directory = "")
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

inline std::set<std::string> parseEnvironmentVariableList(const std::string& env_variable)
{
  std::set<std::string> list;
  char* env_var = std::getenv(env_variable.c_str());
  if (env_var == nullptr)  // Environment variable not found
    return list;

  std::string evn_str = std::string(env_var);
  boost::split(list, evn_str, boost::is_any_of(":"), boost::token_compress_on);
  return list;
}

inline std::set<std::string> getAllSearchPaths(const std::string& search_paths_env,
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

// aka tesseract_common::getAllSearchLibraries
inline std::set<std::string> getAllLibraryNames(const std::string& search_libraries_env,
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

// aka tesseract_common::ClassLoader::isClassAvailable
inline boost::dll::shared_library loadLibrary(const std::string& library_name,
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

inline bool isClassAvailable(const std::string& symbol_name, const std::string& library_name,
                             const std::string& library_directory = "")
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);
  return lib.has(symbol_name);
}

inline std::vector<std::string> getAllAvailableClasses(const std::string& section,
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

inline std::vector<std::string> getAllAvailableClasses(const std::string& section, const std::string& library_name,
                                                    const std::string& library_directory = "")
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(lib.location());

  // Getting symbols exported from he provided section
  return inf.symbols(section);
}

inline std::vector<std::string> getAllAvailableSections(const std::string& library_name,
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

}  // namespace

namespace plugin_loader
{
template <typename PluginBase>
static boost::shared_ptr<PluginBase> s_createSharedInstance(const std::string& symbol_name,
                                                            const std::string& library_name,
                                                            const std::string& library_directory = "")
{
  boost::dll::shared_library lib = loadLibrary(library_name, library_directory);

  // Check if library has symbol
  if (!lib.has(symbol_name))
    throw PluginLoaderException("Failed to find symbol '" + symbol_name +
                                "' in library: " + decorate(library_name, library_directory));

    // Load the class
#if BOOST_VERSION >= 107600
  return boost::dll::import_symbol<PluginBase>(lib, symbol_name);
#else
  return boost::dll::import<PluginBase>(lib, symbol_name);
#endif
}

template <typename PluginBase>
boost::shared_ptr<PluginBase> PluginLoader::createInstanceBoost(const std::string& plugin_name) const
{
  // Check for environment variable for plugin definitions
  std::set<std::string> plugins_local = getAllLibraryNames(search_libraries_env, search_libraries);
  if (plugins_local.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable for search paths
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : plugins_local)
    {
      try
      {
        return s_createSharedInstance<PluginBase>(plugin_name, library, path);
      }
      catch (const std::exception&)
      {
        continue;
      }
    }
  }

  // If not found in any of the provided search paths then search system folders if allowed
  if (search_system_folders)
  {
    for (const auto& library : search_libraries)
    {
      try
      {
        return s_createSharedInstance<PluginBase>(plugin_name, library);
      }
      catch (const std::exception&)
      {
        continue;
      }
    }
  }

  std::stringstream msg;
  msg << "Failed to instantiate plugin '" << plugin_name << "'\n";

  if (search_system_folders)
    msg << std::endl << "Search Paths (Search System Folders: True):" << std::endl;
  else
    msg << std::endl << "Search Paths (Search System Folders: False):" << std::endl;

  for (const auto& path : search_paths_local)
    msg << "    - " + path << std::endl;

  msg << "Search Libraries:" << std::endl;
  for (const auto& library : search_libraries)
    msg << "    - " << decorate(library) << std::endl;

  throw PluginLoaderException(msg.str());
}

template <typename PluginBase>
std::shared_ptr<PluginBase> PluginLoader::createInstance(const std::string& plugin_name) const
{
  boost::shared_ptr<PluginBase> plugin = createInstanceBoost<PluginBase>(plugin_name);
  return std::shared_ptr<PluginBase>(plugin.get(), [plugin](PluginBase*) mutable { plugin.reset(); });
}

template <typename PluginBase>
std::vector<std::string> PluginLoader::getAllAvailablePlugins() const
{

  // Check for environment variable for plugin definitions
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  if (search_paths_local.empty())
  {
    if (!search_system_folders)
      throw PluginLoaderException("No plugin search paths were provided!");

    // Insert an empty string into the search paths set to look in system folders
    search_paths_local.insert(std::string{});
  }

  std::vector<std::string> plugins;
  for (const std::string& library_directory : search_paths_local)
  {
    for (const std::string& library_name : library_names)
    {
      try
      {
        boost::dll::shared_library lib = loadLibrary(library_name, library_directory);
        boost::dll::library_info inf(lib.location());

        // Getting symbols exported from plugin section
        std::vector<std::string> exports = inf.symbols(PluginBase::section);
        plugins.insert(plugins.end(), exports.begin(), exports.end());
      }
      catch (const std::exception&)
      {
        continue;
      }
    }
  }

  return plugins;
}

std::vector<std::string> PluginLoader::getAvailableSections(bool include_hidden) const
{
  std::vector<std::string> sections;

  // Check for environment variable for plugin definitions
  std::set<std::string> library_names = getAllLibraryNames(search_libraries_env, search_libraries);
  if (library_names.empty())
    throw PluginLoaderException("No plugin libraries were provided!");

  // Check for environment variable to override default library
  std::set<std::string> search_paths_local = getAllSearchPaths(search_paths_env, search_paths);
  for (const auto& path : search_paths_local)
  {
    for (const auto& library : library_names)
    {
      std::vector<std::string> lib_sections = getAllAvailableSections(library, path, include_hidden);
      sections.insert(sections.end(), lib_sections.begin(), lib_sections.end());
    }
  }

  return sections;
}

int PluginLoader::count() const
{
  return static_cast<int>(getAllLibraryNames(search_libraries_env, search_libraries).size());
}

}  // namespace plugin_loader
