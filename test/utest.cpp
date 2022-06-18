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
#include <plugin_loader/plugin_loader.hpp>
#include <gtest/gtest.h>

using namespace plugin_loader;

namespace plugin_loader
{
struct ConsolePrinter;
struct HelloWorldPrinter;
struct Square;
struct Triangle;
}  // namespace plugin_loader

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
    std::vector<std::string> sections = getAvailableSections(lib_name, lib_dir);
    EXPECT_EQ(sections.size(), 2);
    EXPECT_TRUE(std::find(sections.begin(), sections.end(), "printer") != sections.end());
    EXPECT_TRUE(std::find(sections.begin(), sections.end(), "shape") != sections.end());

    sections = getAvailableSections(lib_name, lib_dir, true);
    EXPECT_TRUE(sections.size() > 2);
  }

  {
    std::vector<std::string> symbols = getAvailableSymbols("printer", lib_name, lib_dir);
    EXPECT_EQ(symbols.size(), 2);
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "ConsolePrinter") != symbols.end());
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "HelloWorldPrinter") != symbols.end());
  }

  {
    std::vector<std::string> symbols = getAvailableSymbols("shape", lib_name, lib_dir);
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

  {
    // NOLINTNEXTLINE
    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, lib_name, "does_not_exist"),
                 plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, "does_not_exist", lib_dir),
                 plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>("does_not_exist", lib_name, lib_dir),
                 plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>(symbol_name, "does_not_exist"),
                 plugin_loader::PluginLoaderException);
    // NOLINTNEXTLINE
    ASSERT_THROW(s_createSharedInstance<ConsolePrinter>("does_not_exist", lib_name),
                 plugin_loader::PluginLoaderException);
  }
}

// TEST(PluginLoader, PluginLoader)  // NOLINT
//{
//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_paths.insert(std::string(TEST_PLUGIN_DIR));
//    plugin_loader.search_libraries.insert("tesseract_common_test_plugin_multiply");

//    EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
//    auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//    EXPECT_TRUE(plugin != nullptr);
//    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);

//    std::vector<std::string> sections = plugin_loader.getAvailableSections();
//    EXPECT_EQ(sections.size(), 1);
//    EXPECT_EQ(sections.at(0), "TestBase");

//    sections = plugin_loader.getAvailableSections(true);
//    EXPECT_TRUE(sections.size() > 1);

//    std::vector<std::string> symbols = plugin_loader.getAvailablePlugins<TestPluginBase>();
//    EXPECT_EQ(symbols.size(), 1);
//    EXPECT_EQ(symbols.at(0), "plugin");

//    symbols = plugin_loader.getAvailablePlugins("TestBase");
//    EXPECT_EQ(symbols.size(), 1);
//    EXPECT_EQ(symbols.at(0), "plugin");
//  }

//// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
//#if BOOST_VERSION > 106800
//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_libraries.insert("tesseract_common_test_plugin_multiply");

//    EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
//    auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//    EXPECT_TRUE(plugin != nullptr);
//    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
//  }
//#endif

//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_system_folders = false;
//    plugin_loader.search_paths.insert("does_not_exist");
//    plugin_loader.search_libraries.insert("tesseract_common_test_plugin_multiply");

//    {
//      EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
//      auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//      EXPECT_TRUE(plugin == nullptr);
//    }
//  }

//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_system_folders = false;
//    plugin_loader.search_libraries.insert("tesseract_common_test_plugin_multiply");

//    {
//      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
//      auto plugin = plugin_loader.instantiate<TestPluginBase>("does_not_exist");
//      EXPECT_TRUE(plugin == nullptr);
//    }

//    plugin_loader.search_system_folders = true;

//    {
//      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
//      auto plugin = plugin_loader.instantiate<TestPluginBase>("does_not_exist");
//      EXPECT_TRUE(plugin == nullptr);
//    }
//  }

//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_system_folders = false;
//    plugin_loader.search_libraries.insert("does_not_exist");

//    {
//      EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
//      auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//      EXPECT_TRUE(plugin == nullptr);
//    }

//    plugin_loader.search_system_folders = true;

//    {
//      EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
//      auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//      EXPECT_TRUE(plugin == nullptr);
//    }
//  }

//  {
//    PluginLoader plugin_loader;
//    EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
//    auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//    EXPECT_TRUE(plugin == nullptr);
//  }

//  {
//    PluginLoader plugin_loader;
//    plugin_loader.search_system_folders = false;
//    EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
//    auto plugin = plugin_loader.instantiate<TestPluginBase>("plugin");
//    EXPECT_TRUE(plugin == nullptr);
//  }
//}

TYPED_TEST(PluginLoaderFixture, LoadPlugins)
{
  std::vector<std::string> plugins;
  ASSERT_NO_THROW(plugins = this->loader.template getAllAvailablePlugins<TypeParam>());
  ASSERT_EQ(plugins.size(), 2);

  for (const std::string& plugin_name : plugins)
  {
    std::cout << "Loading plugin '" << plugin_name << "'" << std::endl;
    typename TypeParam::Ptr plugin;
    ASSERT_NO_THROW(plugin = this->loader.template createInstance<TypeParam>(plugin_name));
    ASSERT_NO_THROW(plugin->operator()());
  }

  ASSERT_THROW(this->loader.template createInstance<TypeParam>(""), plugin_loader::PluginLoaderException);
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
