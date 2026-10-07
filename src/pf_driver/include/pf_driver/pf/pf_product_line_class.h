#pragma once

#include <string>

struct ScanParameters;

class PFProductLineClass
{
public:
  enum class LineClass
  {
    SINGLE = 1,
    QUAD = 4,
    UNKNOWN = 0
  };

  PFProductLineClass() = default;
  explicit PFProductLineClass(const std::string& product);

  void set_product(const std::string& product);
  LineClass get_product_line_class() const;
  bool get_line_class_line_count(int& count) const;
  bool get_line_class_inclination_count(int& count) const;
  bool apply_line_count(ScanParameters& params) const;
  bool apply_inclination_count(ScanParameters& params) const;

private:
  LineClass line_class_ {LineClass::UNKNOWN};
};