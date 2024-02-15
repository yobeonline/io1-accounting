/// \file test_entry.cpp
#include "gtest/gtest.h"
#include "io1/entry.hpp"
#include <boost/exception/get_error_info.hpp>
#include "accounting_exception.hpp"

#include <sstream>

namespace io1 {

  class TestEntry : public ::testing::Test
  {
  public:
    void TestFailedConstruction(void) const;
    void TestReadWrite(void) const;
  };

  TEST_F(TestEntry, TestFailedConstruction) { return TestFailedConstruction(); };
  TEST_F(TestEntry, TestReadWrite) { return TestReadWrite(); };
}

void io1::TestEntry::TestFailedConstruction(void) const
{
  ASSERT_THROW({
    try
    {
      Entry(-12.3456_USD, "A long description with whitespaces to check just about everything", QDate{ 2018,7,28 });
    }
    catch(InvalidAmountFormat const & e)
    {
      ASSERT_TRUE(boost::get_error_info<InvalidAmountFormat::errinfo_amount>(e));
      throw;
    }
  }, InvalidAmountFormat);
  ASSERT_THROW(Entry(12_USD, "A dummy description.", QDate{}), InvalidDate);
}

void io1::TestEntry::TestReadWrite(void) const
{
  Entry e1{-12.34_USD, "A long description with whitespaces to check just about everything",QDate{2018,7,28}};
  Entry e2{24.12_USD, "Another long description with whitespaces to check just about everything", QDate{2019, 7, 28}};
  Entry e3{0_USD, "Yet another long description with whitespaces to check just about everything", QDate{2020, 7, 28}};
  Entry e4{42.5_USD, "A last long description with whitespaces to check just about everything", QDate{2021, 7, 28}};

  std::stringstream stream;
  stream << e1 << e2 << e3 << e4;

  Entry e1_read, e2_read, e3_read, e4_read;
  stream >> e1_read >> e2_read >> e3_read >> e4_read;

  ASSERT_EQ(e1, e1_read);
  ASSERT_EQ(e2, e2_read);
  ASSERT_EQ(e3, e3_read);
  ASSERT_EQ(e4, e4_read);

  stream << "2018/20/20 $12.5 An entry with an invalid date.";
  Entry e5_read;

  ASSERT_THROW({
    try
    {
      stream >> e5_read;
    }
    catch (InvalidDateFormat const & e)
    {
      ASSERT_TRUE(boost::get_error_info<InvalidDateFormat::errinfo_date_string>(e));
      throw;
    }
  }, InvalidDateFormat);

  return;
}
