#include <gtest/gtest.h>

#include "pf_driver/pf/pf_product_line_class.h"
#include "pf_driver/pf/scan_parameters.h"

class PFProductLineClassTest : public ::testing::Test
{
protected:
  PFProductLineClass line_class;
};

TEST_F(PFProductLineClassTest, ClassifiesProductByLineSuffix)
{
  line_class.set_product("OMD10M-R2300-B23-V1V1D-4S");
  EXPECT_EQ(line_class.get_product_line_class(), PFProductLineClass::LineClass::QUAD);

  line_class.set_product("OMD10M-R2300-B23-V1V1D-1S");
  EXPECT_EQ(line_class.get_product_line_class(), PFProductLineClass::LineClass::SINGLE);
}

TEST_F(PFProductLineClassTest, RejectsUnknownOrNonSuffixMatches)
{
  line_class.set_product("OMD10M-R2300-B23-V1V1D");
  EXPECT_EQ(line_class.get_product_line_class(), PFProductLineClass::LineClass::UNKNOWN);

  line_class.set_product("OMD10M-R2300-B23-V1V1D-4S-extra");
  EXPECT_EQ(line_class.get_product_line_class(), PFProductLineClass::LineClass::UNKNOWN);

  line_class.set_product("4S");
  EXPECT_EQ(line_class.get_product_line_class(), PFProductLineClass::LineClass::UNKNOWN);
}

TEST_F(PFProductLineClassTest, ProvidesLineAndInclinationCounts)
{
  int count = 0;
  line_class.set_product("OMD10M-R2300-B23-V1V1D-4S");
  EXPECT_TRUE(line_class.get_line_class_line_count(count));
  EXPECT_EQ(count, 4);
  EXPECT_TRUE(line_class.get_line_class_inclination_count(count));
  EXPECT_EQ(count, 4);

  line_class.set_product("OMD10M-R2300-B23-V1V1D-1S");
  EXPECT_TRUE(line_class.get_line_class_line_count(count));
  EXPECT_EQ(count, 1);
  EXPECT_TRUE(line_class.get_line_class_inclination_count(count));
  EXPECT_EQ(count, 1);

  line_class.set_product("OMD10M-R2300-B23-V1V1D");
  EXPECT_FALSE(line_class.get_line_class_line_count(count));
  EXPECT_EQ(count, 0);
  EXPECT_FALSE(line_class.get_line_class_inclination_count(count));
  EXPECT_EQ(count, 0);
}

TEST_F(PFProductLineClassTest, AppliesDerivedCountsWhenNotReceived)
{
  ScanParameters params;
  line_class.set_product("OMD10M-R2300-B23-V1V1D-4S");

  EXPECT_TRUE(line_class.apply_line_count(params));
  EXPECT_EQ(params.layer_count, 4);
  EXPECT_TRUE(line_class.apply_inclination_count(params));
  EXPECT_EQ(params.inclination_count, 4);
}

TEST_F(PFProductLineClassTest, PreservesReceivedCounts)
{
  ScanParameters params;
  params.layer_count = 7;
  params.layer_count_received = true;
  params.inclination_count = 8;
  params.inclination_count_received = true;
  line_class.set_product("unknown-product");

  EXPECT_TRUE(line_class.apply_line_count(params));
  EXPECT_EQ(params.layer_count, 7);
  EXPECT_TRUE(line_class.apply_inclination_count(params));
  EXPECT_EQ(params.inclination_count, 8);
}

TEST_F(PFProductLineClassTest, RejectsUnknownClassWhenCountsAreNotReceived)
{
  ScanParameters params;
  line_class.set_product("unknown-product");

  EXPECT_FALSE(line_class.apply_line_count(params));
  EXPECT_EQ(params.layer_count, 0);
  EXPECT_FALSE(line_class.apply_inclination_count(params));
  EXPECT_EQ(params.inclination_count, 0);
}