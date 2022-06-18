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

#include <plugin_loader/plugin_loader.h>
#include <plugin_loader/plugin_loader_utils.h>

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

}  // namespace plugin_loader
