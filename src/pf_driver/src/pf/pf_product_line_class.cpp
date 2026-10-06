#include "pf_driver/pf/pf_product_line_class.h"

#include "pf_driver/pf/scan_parameters.h"

PFProductLineClass::PFProductLineClass(const std::string& product)
{
  set_product(product);
}

void PFProductLineClass::set_product(const std::string& product)
{
  if (product.size() >= 3 && product.compare(product.size() - 3, 3, "-4S") == 0)
  {
    line_class_ = LineClass::QUAD;
  }
  else if (product.size() >= 3 && product.compare(product.size() - 3, 3, "-1S") == 0)
  {
    line_class_ = LineClass::SINGLE;
  }
  else
  {
    line_class_ = LineClass::UNKNOWN;
  }
}

PFProductLineClass::LineClass PFProductLineClass::get_product_line_class() const
{
  return line_class_;
}

bool PFProductLineClass::get_line_class_line_count(int& count) const
{
  switch (line_class_)
  {
    case LineClass::SINGLE:
      count = 1;
      return true;
    case LineClass::QUAD:
      count = 4;
      return true;
    default:
      count = 0;
      return false;
  }
}

bool PFProductLineClass::get_line_class_inclination_count(int& count) const
{
  return get_line_class_line_count(count);
}

bool PFProductLineClass::apply_line_count(ScanParameters& params) const
{
  return params.layer_count_received || get_line_class_line_count(params.layer_count);
}

bool PFProductLineClass::apply_inclination_count(ScanParameters& params) const
{
  return params.inclination_count_received || get_line_class_inclination_count(params.inclination_count);
}