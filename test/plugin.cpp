#include "plugin.h"
#include <plugin_loader/plugin_loader.hpp>

namespace plugin_loader
{
// Define the section name for loading plugins of base class `Printer`
// This should match the section name specified in the plugin export macro for this class
const std::string Printer::section = "printer";
// Explicit template instantiate the plugin loader functions for the `Printer` class
template std::vector<std::string> PluginLoader::getAllAvailablePlugins<Printer>() const;
template Printer::Ptr PluginLoader::createInstance(const std::string&) const;

// Define the section name for loading plugins of base class `Shape`
// This should match the section name specified in the plugin export macro for this class
const std::string Shape::section = "shape";
// Explicit template instantiate the plugin loader functions for the `Shape` class
template std::vector<std::string> PluginLoader::getAllAvailablePlugins<Shape>() const;
template Shape::Ptr PluginLoader::createInstance(const std::string&) const;
}  // namespace plugin_loader
