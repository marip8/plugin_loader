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

TYPED_TEST_CASE(PluginLoaderFixture, Implementations);

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
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
