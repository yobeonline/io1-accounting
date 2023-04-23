/// \file test_statement.cpp
#include "gtest/gtest.h"
#include "statement.hpp"
#include "date_formatter.hpp"
#include <boost/exception/get_error_info.hpp>

#include <sstream>

namespace io1 {

  class TestStatement : public ::testing::Test
  {
  public:
    void TestReadWrite(void) const;
    void TestReadWriteComittable(void) const;
    void TestFailedReadWriteComposed(void) const;
  };

  TEST_F(TestStatement, TestReadWrite) { return TestReadWrite(); };
  TEST_F(TestStatement, TestReadWriteComittable) { return TestReadWriteComittable(); };
  TEST_F(TestStatement, TestFailedReadWriteComposed) { return TestFailedReadWriteComposed(); };
}

void io1::TestStatement::TestReadWrite(void) const
{
  Statement s1{-12.34_USD, "A long description with whitespaces to check just about everything", QDate{2018, 7, 28}};

  std::vector<Entry> entries;
  entries.emplace_back(12_USD, "First entry of the statement", QDate{2018, 7, 28});
  entries.emplace_back(-12_USD, "Second entry of the statement", QDate{2018, 7, 29});
  entries.emplace_back(8_USD, "third entry of the statement", QDate{2018, 7, 30});
  Statement s2{"Another long description with whitespaces to check just about everything", QDate{2019, 7, 31}, std::move(entries)};

  Statement s3{3456_USD, "Yet another long description with whitespaces to check just about everything", QDate{2020, 7, 28}};

  entries = std::vector<Entry>{};
  entries.emplace_back(12_USD, "The only entry of the statement, because why not?", QDate{2020, 7, 28});
  Statement s4{"A not quite final long description with whitespaces to check just about everything", QDate{2019, 7, 31}, std::move(entries)};

  entries = std::vector<Entry>{};
  entries.emplace_back(-12_USD, "Another first entry of the statement", QDate{2018, 7, 29});
  entries.emplace_back(8_USD, "Another second entry of the statement", QDate{2018, 7, 30});
  Statement s5{"A final long description with whitespaces to check just about everything", QDate{2021, 7, 31}, std::move(entries)};

  std::stringstream stream;
  stream << s1 << s2 << s3 << s4 << s5;

  Statement s1_read, s2_read, s3_read, s4_read, s5_read;
  stream >> s1_read >> s2_read >> s3_read >> s4_read >> s5_read;

  ASSERT_EQ(s1, s1_read);
  ASSERT_EQ(s2, s2_read);
  ASSERT_EQ(s3, s3_read);
  ASSERT_EQ(s4, s4_read);
  ASSERT_EQ(s5, s5_read);

  return;
}

void io1::TestStatement::TestReadWriteComittable(void) const
{
  CommittableStatement s1{ -12.34_USD, "A long description with whitespaces to check just about everything", QDate{ 2018, 7, 28 } };

  std::vector<Entry> entries;
  entries.emplace_back(12_USD, "First entry of the statement", QDate{ 2018, 7, 28 });
  entries.emplace_back(-12_USD, "Second entry of the statement", QDate{ 2018, 7, 29 });
  entries.emplace_back(8_USD, "third entry of the statement", QDate{ 2018, 7, 30 });
  CommittableStatement s2{ "Another long description with whitespaces to check just about everything", QDate{ 2019, 7, 31 }, std::move(entries) };

  CommittableStatement s3{ 3456_USD, "Yet another long description with whitespaces to check just about everything", QDate{ 2020, 7, 28 } };
  s3.set_committed();

  entries = std::vector<Entry>{};
  entries.emplace_back(12_USD, "The only entry of the statement, because why not?", QDate{ 2020, 7, 28 });
  CommittableStatement s4{ "A not quite final long description with whitespaces to check just about everything", QDate{ 2019, 7, 31 }, std::move(entries) };
  s4.set_committed();

  entries = std::vector<Entry>{};
  entries.emplace_back(-12_USD, "Another first entry of the statement", QDate{ 2018, 7, 29 });
  entries.emplace_back(8_USD, "Another second entry of the statement", QDate{ 2018, 7, 30 });
  CommittableStatement s5{ "A final long description with whitespaces to check just about everything", QDate{ 2021, 7, 31 }, std::move(entries) };

  std::stringstream stream;
  stream << s1 << s2 << s3 << s4 << s5;

  CommittableStatement s1_read, s2_read, s3_read, s4_read, s5_read;
  stream >> s1_read >> s2_read >> s3_read >> s4_read >> s5_read;

  ASSERT_EQ(s1, s1_read);
  ASSERT_EQ(s2, s2_read);
  ASSERT_EQ(s3, s3_read);
  ASSERT_EQ(s4, s4_read);
  ASSERT_EQ(s5, s5_read);

  return;
}

void io1::TestStatement::TestFailedReadWriteComposed(void) const
{
  std::stringstream stream;
  stream << date_formatter(QDate{2018, 7, 31}) << ' ' << 12_USD << ' ' << "A long description with whitespaces to check just about everything\n";
  stream << "- " << date_formatter(QDate{2018, 7, 28}) << ' ' << 12_USD << ' ' << "First entry of the statement\n";
  stream << "- " << date_formatter(QDate{2018, 7, 29}) << ' ' << -12_USD << ' ' << "Second entry of the statement\n";
  stream << "- " << date_formatter(QDate{2018, 7, 30}) << ' ' << 8_USD << ' ' << "Third entry of the statement\n";

  Statement s_read;

  ASSERT_THROW({
  try
  {stream >> s_read; }
  catch (AmountMismatch const & e)
  {
    ASSERT_TRUE(boost::get_error_info<AmountMismatch::errinfo_main_entry>(e));
    ASSERT_TRUE(boost::get_error_info<AmountMismatch::errinfo_main_entry>(e));
    throw;
  }}
  ,AmountMismatch);

  return;
}
