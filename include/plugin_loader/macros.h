#pragma once

#include <boost/dll/alias.hpp>

/** @brief Exports a class with an alias name under the "section" namespace */
#define EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, SECTION)                                                          \
  extern "C" BOOST_SYMBOL_EXPORT DERIVED_CLASS ALIAS;                                                                  \
  BOOST_DLL_SECTION(SECTION, read) BOOST_DLL_SELECTANY DERIVED_CLASS ALIAS;
