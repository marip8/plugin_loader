#pragma once

#include <memory>
#include <string>

namespace plugin_loader
{
/**
 * @brief Plugin interface implementation for testing
 */
class Printer
{
public:
  using Ptr = std::shared_ptr<Printer>;
  virtual void operator()() const = 0;

private:
  friend class PluginLoader;
  static const std::string section;
};

/**
 * @brief Plugin interface implementation for testing
 */
class Shape
{
public:
  using Ptr = std::shared_ptr<Shape>;
  virtual void operator()() const = 0;

private:
  friend class PluginLoader;
  static const std::string section;
};

}  // namespace plugin_loader

#include <plugin_loader/macros.h>
#define EXPORT_PRINTER_PLUGIN(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, printer)
#define EXPORT_SHAPE_PLUGIN(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, shape)
