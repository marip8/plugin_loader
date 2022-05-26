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
