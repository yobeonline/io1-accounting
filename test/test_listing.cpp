/// \file test_listing.cpp
#include "gtest/gtest.h"
#include "listing.hpp"

namespace io1 {

  class TestListing: public ::testing::Test
	{
	public:
    template <typename committed_tag> void TestInteraction(void) const;
    template <typename committed_tag> void TestGatherSelection(void) const;
    template <typename committed_tag> void TestGroupRange(void) const;
    template <typename committed_tag> void TestSplit(void) const;
    template <typename committed_tag> void TestMoveStatement(void) const;
    template <typename committed_tag> void TestReadWrite(void) const;
    void TestCommittable(void) const;
	};
	
  TEST_F(TestListing, TestInteraction) { return TestInteraction<non_committable_tag>(); };
  TEST_F(TestListing, TestGatherSelection) { return TestGatherSelection<non_committable_tag>(); };
  TEST_F(TestListing, TestGroupRange) { return TestGroupRange<non_committable_tag>(); };
  TEST_F(TestListing, TestSplit) { return TestSplit<non_committable_tag>(); };
  TEST_F(TestListing, TestMoveStatement) { return TestMoveStatement<non_committable_tag>(); };
  TEST_F(TestListing, TestReadWrite) { return TestReadWrite<non_committable_tag>(); };
  TEST_F(TestListing, TestCommittableInteraction) { return TestInteraction<committable_tag>(); };
  TEST_F(TestListing, TestCommittableGatherSelection) { return TestGatherSelection<committable_tag>(); };
  TEST_F(TestListing, TestCommittableGroupRange) { return TestGroupRange<committable_tag>(); };
  TEST_F(TestListing, TestCommittableSplit) { return TestSplit<committable_tag>(); };
  TEST_F(TestListing, TestCommittableMoveStatement) { return TestMoveStatement<committable_tag>(); };
  TEST_F(TestListing, TestCommittableReadWrite) { return TestReadWrite<committable_tag>(); };
  TEST_F(TestListing, TestCommittable) { return TestCommittable(); };
}

void io1::TestListing::TestCommittable(void) const
{
  Listing<committable_tag> l{"test"};
  auto const & s = *l.add_statement(12_USD, "sample credit.");
  ASSERT_FALSE(s.is_committed());

  s.set_committed();
  ASSERT_TRUE(s.is_committed());

  return;
}

template <typename COMMITTED_TAG> void io1::TestListing::TestReadWrite(void) const
{
  Listing<COMMITTED_TAG> l{"This is a test listing"};
  l.add_statement(12.12_USD,"Sample line",QDate{1979,07,28});
  l.add_statement(-120.98_USD, "Another line", QDate{ 1982,2,18 });
  
  std::stringstream s;
  s << l;
  
  Listing<COMMITTED_TAG> l_read;
  s >> l_read;

  ASSERT_EQ(l,l_read);
  return;
}

// Tests that the exponent used by the class is a multiple of ten.
template <typename COMMITTED_TAG> void io1::TestListing::TestInteraction(void) const
{
  Listing<COMMITTED_TAG> l{"test"};
  ASSERT_STREQ("test",l.name().toStdString().c_str());

  l.set_name("another test");
  ASSERT_STREQ("another test", l.name().toStdString().c_str());

  auto const & s = *l.add_statement(12_USD,"sample credit.");
  ASSERT_STREQ("sample credit.", s.description().toStdString().c_str());
  ASSERT_EQ(12_USD, s.amount());
  ASSERT_TRUE(s.composed_entries().empty());

  auto const & s1 = *l.add_statement(24_USD, "another sample credit.");
  ASSERT_STREQ("another sample credit.", s1.description().toStdString().c_str());
  ASSERT_EQ(24_USD, s1.amount());
  ASSERT_TRUE(s1.composed_entries().empty());

  std::vector<Entry> entries;
  entries.emplace_back(10_USD,"first deposit.");
  entries.emplace_back(20_USD, "second deposit.");
  entries.emplace_back(30_USD, "third deposit.");

  auto const & s2 = *l.add_statement("deposits",QDate::currentDate(),entries);
  ASSERT_STREQ("deposits",s2.description().toStdString().c_str());
  ASSERT_EQ(60_USD,s2.amount());
  ASSERT_EQ(3,s2.composed_entries().size());

  auto const & s3 = *l.add_statement(100_USD,"yet another line");
  ASSERT_STREQ("yet another line", s3.description().toStdString().c_str());
  ASSERT_EQ(100_USD, s3.amount());
  ASSERT_TRUE(s3.composed_entries().empty());

  l.group_range("grouped operation",QDate::currentDate(),{++l.statements().begin(),--l.statements().end()});

  typename Listing<COMMITTED_TAG>::statement_selection selection;
  selection.insert(l.statements().begin());
  selection.insert(--l.statements().end());
  l.group_range("grouped selection", QDate::currentDate(),l.gather_selection(selection));

  std::vector<typename COMMITTED_TAG::statement_type> statements;
  statements.emplace_back(10_USD, "first deposit.");
  statements.emplace_back(20_USD, "second deposit.");
  statements.emplace_back(30_USD, "third deposit.");

  Listing<COMMITTED_TAG> l1("new test",statements);
  auto statements_range = l1.statements();
  ASSERT_EQ(3, boost::size(statements_range));
  ASSERT_EQ(statements[0], *statements_range.begin());
  ASSERT_EQ(statements[1], *(statements_range.begin()+1));
  ASSERT_EQ(statements[2], *(statements_range.begin()+2));

  return;
}

// Tests the gather_selection function
template <typename COMMITTED_TAG> void io1::TestListing::TestGatherSelection(void) const
{
  Listing<COMMITTED_TAG> l("Testing gathering of a selection of statements.");
  l.add_statement(0_USD, "");
  l.add_statement(1_USD, "");
  l.add_statement(2_USD, "");
  l.add_statement(3_USD, "");
  l.add_statement(4_USD, "");
  l.add_statement(5_USD, "");
  l.add_statement(6_USD, "");
  l.add_statement(7_USD, "");
  l.add_statement(8_USD, "");
  l.add_statement(9_USD, "");
  l.add_statement(10_USD, "");
  auto const item = [&l](size_t index){ return l.statements().begin()+index; };

  auto l_ref = l;
  auto const item_ref = [&l_ref](size_t index) { return l_ref.statements().begin() + index; };

  // gathering an empty selection does nothing and returns an empty range.
  ASSERT_TRUE(l.gather_selection(typename Listing<COMMITTED_TAG>::statement_selection{}).empty());
  ASSERT_EQ(l_ref,l);
  
  // gathering a single element does nothing as well but returns a range that contains it.
  auto const range1 = l.gather_selection({ item(2) });
  ASSERT_EQ(1, range1.size());
  ASSERT_EQ(*item_ref(2), *range1.begin());
  ASSERT_EQ(l_ref, l);

  // gathering a selection of contiguous elements does nothing and returns a range that contains them.
  auto const range2 = l.gather_selection({ item(2),item(4),item(3) });
  ASSERT_EQ(3,range2.size());
  ASSERT_EQ(*item_ref(2), *range2.begin());
  ASSERT_EQ(*item_ref(3), *(1+range2.begin()));
  ASSERT_EQ(*item_ref(4), *(2+range2.begin()));
  ASSERT_EQ(l_ref, l);

  // gathering a selection of non contiguous elements moves them together and returns a range that contains them.
  auto const range3 = l.gather_selection( {item(2), item(4), item(6), item(8), item(10) });
  ASSERT_EQ(5, range3.size());
  ASSERT_EQ(*item_ref(2), *range3.begin());
  ASSERT_EQ(*item_ref(4), *(1+range3.begin()));
  ASSERT_EQ(*item_ref(6), *(2+range3.begin()));
  ASSERT_EQ(*item_ref(8), *(3+range3.begin()));
  ASSERT_EQ(*item_ref(10), *(4+range3.begin()));
  ASSERT_EQ(*item_ref(1), *item(1));
  ASSERT_EQ(*item_ref(3), *item(2));
  ASSERT_EQ(*item_ref(5), *item(3));
  ASSERT_EQ(*item_ref(7), *item(4));
  ASSERT_EQ(*item_ref(9), *item(5));
  ASSERT_EQ(*item_ref(2), *item(6));
  ASSERT_EQ(*item_ref(4), *item(7));
  ASSERT_EQ(*item_ref(6), *item(8));
  ASSERT_EQ(*item_ref(8), *item(9));
  ASSERT_EQ(*item_ref(10), *item(10));

  return;
}

// Tests the group_range function
template <typename COMMITTED_TAG> void io1::TestListing::TestGroupRange(void) const
{
  Listing<COMMITTED_TAG> l("Testing grouping of ranges of statements.");
  l.add_statement(0_USD, "");
  l.add_statement(1_USD, "");
  l.add_statement(2_USD, "");
  l.add_statement(3_USD, "");
  l.add_statement(4_USD, "");
  l.add_statement(5_USD, "");
  l.add_statement(6_USD, "");
  l.add_statement(7_USD, "");
  l.add_statement(8_USD, "");
  l.add_statement(9_USD, "");
  l.add_statement(10_USD, "");
  auto const item = [&l](size_t index) { return l.statements().begin() + index; };

  auto l_ref = l;
  auto const item_ref = [&l_ref](size_t index) { return l_ref.statements().begin() + index; };

  // trying to group an empty range does nothing and returns the end iterator of the range.
  auto const it1 = l.group_range("grouping an empty range.",QDate::currentDate(),{item(4),item(4)});
  ASSERT_EQ(item(4), it1);
  ASSERT_EQ(l_ref, l);

  // same thing with an empty range over the past-the-end iterator.
  auto const it2 = l.group_range("grouping an empty range, past-the-end.", QDate::currentDate(), { item(11),item(11) });
  ASSERT_EQ(item(11), it2);
  ASSERT_EQ(l_ref, l);

  // trying to group a single statement does nothing and returns an iterator to this element.
  auto const it3 = l.group_range("grouping a single statement.", QDate::currentDate(), { item(4),item(5) });
  ASSERT_EQ(item(4), it3);
  ASSERT_EQ(l_ref, l);

  // trying to group a range of statements does group them into one statement, effectively removing them from the listing.
  auto const it4 = l.group_range("group of four statements.", QDate::currentDate(), { item(4),item(8) });
  for (size_t i=0; 4>i; ++i) ASSERT_EQ(*item_ref(i), *item(i));
  // number four is the group, it will be tested further down.
  for (size_t i = 5; 8>i; ++i) ASSERT_EQ(*item_ref(i+3), *item(i));

  // the grouped statement should be composed of item_ref 4, 5, 6 and 7 in this order
  ASSERT_EQ(4,it4->composed_entries().size());
  size_t grouped_entry_index = 4;
  Money grouped_amount = 0_USD;
  for (auto const & e : it4->composed_entries())
  {
    ASSERT_EQ(item_ref(grouped_entry_index++)->main_entry(), e);
    grouped_amount += e.amount();
  }
  ASSERT_EQ(grouped_amount,it4->amount());
  ASSERT_STREQ("group of four statements.",it4->description().toStdString().c_str());

  return;
}

// Tests the split function
template <typename COMMITTED_TAG> void io1::TestListing::TestSplit(void) const
{
  Listing<COMMITTED_TAG> l("Testing splitting a statement.");
  l.add_statement(0_USD, "");
  l.add_statement(1_USD, "");
  l.add_statement(2_USD, "");
  l.add_statement(3_USD, "");
  l.add_statement(4_USD, "");
  l.add_statement(5_USD, "");
  l.add_statement(6_USD, "");
  l.add_statement(7_USD, "");
  l.add_statement(8_USD, "");
  l.add_statement(9_USD, "");
  l.add_statement(10_USD, "");
  auto const item = [&l](size_t index) { return l.statements().begin() + index; };
  l.group_range("grouped_range", QDate::currentDate(), {item(1), item(11)});

  auto l_ref = l;
  auto const item_ref = [&l_ref](size_t index) { return l_ref.statements().begin() + index; };

  // splitting a statement that is not composed does nothing and returns a range that contains the statement.
  auto const range1 = l.split_statement(item(0));
  ASSERT_EQ(l_ref,l);
  ASSERT_EQ(1, range1.size());
  ASSERT_EQ(item(0),range1.begin());

  // splitting a composed statement adds as many statements as its composed entries and returns a range over them.
  auto const range2 = l.split_statement(item(1));
  ASSERT_EQ(10,range2.size());
  for (size_t i=0; 10>i; ++i) ASSERT_EQ(*(item_ref(1)->composed_entries().begin()+i), (i+range2.begin())->main_entry());

  return;
}

template <typename COMMITTED_TAG> void io1::TestListing::TestMoveStatement(void) const
{
  Listing<COMMITTED_TAG> l("Testing moving a statement around.");
  l.add_statement(0_USD, "");
  l.add_statement(1_USD, "");
  l.add_statement(2_USD, "");
  l.add_statement(3_USD, "");
  l.add_statement(4_USD, "");
  l.add_statement(5_USD, "");
  l.add_statement(6_USD, "");
  l.add_statement(7_USD, "");
  l.add_statement(8_USD, "");
  l.add_statement(9_USD, "");
  l.add_statement(10_USD, "");
  auto const item = [&l](size_t index) { return l.statements().begin() + index; };

  auto l_ref = l;
  auto const item_ref = [&l_ref](size_t index) { return l_ref.statements().begin() + index; };

  l.move_statement_down(item(6),item(6));
  ASSERT_EQ(l_ref,l);

  l.move_statement_up(item(6),item(6));
  ASSERT_EQ(l_ref,l);

  l.move_statement_up(item(6),item(7));
  for (size_t i=0; i<6; ++i) ASSERT_EQ(*item_ref(i),*item(i));
  ASSERT_EQ(*item_ref(7), *item(6));
  ASSERT_EQ(*item_ref(6), *item(7));
  for (size_t i=8; i<11; ++i) ASSERT_EQ(*item_ref(i), *item(i));

  l.move_statement_down(item(7), item(6));
  ASSERT_EQ(l_ref,l);

  l.move_statement(item(5), item(8));
  for (size_t i = 0; i < 5; ++i) ASSERT_EQ(*item_ref(i), *item(i));
  ASSERT_EQ(*item_ref(6), *item(5));
  ASSERT_EQ(*item_ref(7), *item(6));
  ASSERT_EQ(*item_ref(8), *item(7));
  ASSERT_EQ(*item_ref(5), *item(8));
  for (size_t i = 9; i < 11; ++i) ASSERT_EQ(*item_ref(i), *item(i));

  l.move_statement(item(8), item(5));
  ASSERT_EQ(l_ref,l);
  return;
}
