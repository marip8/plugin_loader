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
#include <plugin_loader/plugin_loader.h>
#include <gtest/gtest.h>

using namespace plugin_loader;

template <typename Plugin>
class PluginLoaderFixture : public testing::Test
{
public:
  PluginLoaderFixture()
  {
    loader.search_paths.insert(PLUGIN_DIR);
    loader.search_libraries.insert(PLUGINS);
  }

  PluginLoader loader;
};

using Implementations = ::testing::Types<Printer, Shape>;

TYPED_TEST_SUITE(PluginLoaderFixture, Implementations);

TYPED_TEST(PluginLoaderFixture, LoadPlugins)
{
  std::vector<std::string> plugins;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  ASSERT_NO_THROW(plugins = this->loader.template getAllAvailablePlugins<TypeParam>());
  ASSERT_EQ(plugins.size(), 2);

  for (const std::string& plugin_name : plugins)
  {
    std::cout << "Loading plugin '" << plugin_name << "'" << std::endl;
    typename TypeParam::Ptr plugin;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_NO_THROW(plugin = this->loader.template createInstance<TypeParam>(plugin_name));
    ASSERT_NO_THROW(plugin->operator()());  // NOLINT(cppcoreguidelines-avoid-goto)
  }

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  ASSERT_THROW(this->loader.template createInstance<TypeParam>(""), plugin_loader::PluginLoaderException);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
