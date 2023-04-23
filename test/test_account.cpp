/// \file test_account.cpp
#include "gtest/gtest.h"
#include "account.hpp"

namespace io1 {

  class TestAccount : public ::testing::Test
  {
  public:
    void TestInteraction(void) const;
  };

  TEST_F(TestAccount, TestInteraction) { return TestInteraction(); };
}

void io1::TestAccount::TestInteraction(void) const
{
  Account a;
  a.set_name("test");
  a.set_description("This is a test account.");
  
  auto & listing = a.current_listing();
  
  listing.add_statement(1000000.85_USD, "first deposit");
  listing.add_statement(20_USD, "second deposit");
  auto const withdraw = listing.add_statement(-5_USD, "withdrawal");

  std::cout << a;

  withdraw->set_committed();

  std::cout << a;
  return;
}
