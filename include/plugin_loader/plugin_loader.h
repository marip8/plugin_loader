#pragma once

#include <memory>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>

namespace plugin_loader
{
/**
 * @brief Class for dynamically loading plugins
 */
class PluginLoader
{
public:
  /**
   * @brief Loads a shared instance of a plugin of a specified type
   */
  template <typename PluginBase>
  std::shared_ptr<PluginBase> createInstance(const std::string& plugin_name) const;

  /**
   * @brief Lists all available plugins of a specified base type
   * @details This method requires that each plugin interface definition define a static string member called `section`.
   * This string is used to denote symbols (i.e. plugin classes) in a library, such that all symbols a given section
   * name can be found by the plugin loader. It is useful to specify a unique section name to each plugin interface
   * class in order to find all implementations of that plugin interface in the libraries containing plugins.
   */
  template <typename PluginBase>
  std::vector<std::string> getAllAvailablePlugins() const;

  /** @brief Indicate is system folders may be search if plugin is not found in any of the paths */
  bool search_system_folders{ true };

  /** @brief A list of paths to search for plugins */
  std::set<std::string> search_paths;

  /** @brief A list of library names hat contain plugins
   * @details The library names should not contain the prefix (i.e., lib/) or suffix (i.e., .so)
   */
  std::set<std::string> search_libraries;

  /** @brief The environment variable containing plugin search paths */
  std::string search_paths_env;

  /**
   * @brief The environment variable containing plugins
   * @details The plugins are stored in the following format: "library_name;library_name1;library_name2"
   */
  std::string search_libraries_env;

private:
  template <typename PluginBase>
  boost::shared_ptr<PluginBase> createInstanceBoost(const std::string& plugin_name) const;
};

class PluginLoaderException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

}  // namespace plugin_loader
