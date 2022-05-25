#include "plugin.h"

#include <iostream>

namespace plugin_loader
{
struct ConsolePrinter : public Printer
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: ConsolePrinter" << std::endl;
  }
};

struct HelloWorldPrinter : public Printer
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Hello World" << std::endl;
  }
};

struct Square : public Shape
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Square" << std::endl;
  }
};

struct Triangle : public Shape
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Triangle" << std::endl;
  }
};

}  // namespace plugin_loader

EXPORT_PRINTER_PLUGIN(plugin_loader::ConsolePrinter, ConsolePrinter)
EXPORT_PRINTER_PLUGIN(plugin_loader::HelloWorldPrinter, HelloWorldPrinter)
EXPORT_SHAPE_PLUGIN(plugin_loader::Square, Square)
EXPORT_SHAPE_PLUGIN(plugin_loader::Triangle, Triangle)
