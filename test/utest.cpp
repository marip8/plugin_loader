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

TEST(PluginLoader, supportMethods)  // NOLINT
{
  std::set<std::string> s = parseEnvironmentVariableList("UNITTESTENV");
  EXPECT_TRUE(s.empty());

  std::string env_var = "UNITTESTENV=a:b:c";
  putenv(env_var.data());
  s = parseEnvironmentVariableList("UNITTESTENV");
  std::vector<std::string> v(s.begin(), s.end());
  EXPECT_EQ(v[0], "a");
  EXPECT_EQ(v[1], "b");
  EXPECT_EQ(v[2], "c");

  const std::string lib_name = std::string(PLUGINS);
  const std::string lib_dir = std::string(PLUGIN_DIR);
  const std::string symbol_name = "ConsolePrinter";

  {
    std::vector<std::string> sections = getAllAvailableSections(lib_name, lib_dir);
    EXPECT_EQ(sections.size(), 2);
    EXPECT_TRUE(std::find(sections.begin(), sections.end(), "printer") != sections.end());
    EXPECT_TRUE(std::find(sections.begin(), sections.end(), "shape") != sections.end());

    sections = getAllAvailableSections(lib_name, lib_dir, true);
    EXPECT_TRUE(sections.size() > 2);
  }

  {
    std::vector<boost::filesystem::path> sl = { boost::filesystem::path(decorate(lib_name, lib_dir)) };
    std::vector<std::string> symbols = getAllAvailableClasses("printer", sl);
    EXPECT_EQ(symbols.size(), 2);
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "ConsolePrinter") != symbols.end());
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "HelloWorldPrinter") != symbols.end());
  }

  {
    std::vector<std::string> symbols = getAllAvailableClasses("printer", lib_name, lib_dir);
    EXPECT_EQ(symbols.size(), 2);
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "ConsolePrinter") != symbols.end());
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "HelloWorldPrinter") != symbols.end());
  }

  {
    std::vector<std::string> symbols = getAllAvailableClasses("shape", lib_name, lib_dir);
    EXPECT_EQ(symbols.size(), 2);
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "Square") != symbols.end());
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "Triangle") != symbols.end());
  }

  {
    EXPECT_TRUE(isClassAvailable("ConsolePrinter", lib_name, lib_dir));
    EXPECT_FALSE(isClassAvailable("does_not_exist", lib_name, lib_dir));
    EXPECT_FALSE(isClassAvailable("does_not_exist", lib_name));
    // NOLINTNEXTLINE
    ASSERT_THROW(isClassAvailable(symbol_name, lib_name, "does_not_exist"), plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(isClassAvailable(symbol_name, "does_not_exist", lib_dir), plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(isClassAvailable(symbol_name, "does_not_exist"), plugin_loader::PluginLoaderException);
  }

  //  {
  //    // NOLINTNEXTLINE
  //    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, lib_name, "does_not_exist"),
  //                 plugin_loader::PluginLoaderException);
  //    // NOLINTNEXTLINE
  //    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, "does_not_exist", lib_dir),
  //                 plugin_loader::PluginLoaderException);
  //    // NOLINTNEXTLINE
  //    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>("does_not_exist", lib_name, lib_dir),
  //                 plugin_loader::PluginLoaderException);
  //    // NOLINTNEXTLINE
  //    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, "does_not_exist"),
  //                 plugin_loader::PluginLoaderException);
  //    // NOLINTNEXTLINE
  //    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>("does_not_exist", lib_name),
  //                 plugin_loader::PluginLoaderException);
  //  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
