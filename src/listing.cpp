/// \file listing.cpp
#include "listing.hpp"
#include <iomanip>
#include <iterator>
#include <boost/format.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/stable_sort.hpp>

namespace
{
  template<typename STATEMENT> auto const sort_predicate = [](STATEMENT const & lhs, STATEMENT const & rhs) { return lhs.date() < rhs.date(); };

}

template<typename COMMITTABLE> io1::Listing<COMMITTABLE>::Listing(QString name)
:name_(std::move(name))
{}

template<typename COMMITTABLE>
void io1::Listing<COMMITTABLE>::sort(void)
{
  boost::range::sort(statements_, sort_predicate<statement_type>);
  return;
}

template<typename COMMITTABLE>
void io1::Listing<COMMITTABLE>::stable_sort(void)
{
  boost::range::stable_sort(statements_, sort_predicate<statement_type>);
  return;
}

template<typename COMMITTABLE> typename io1::Listing<COMMITTABLE>::const_iterator io1::Listing<COMMITTABLE>::erase_statement(const_iterator position)
{
  assert(statements_.end() > position);
  return statements_.erase(position);
}

template<typename COMMITTABLE> typename io1::Listing<COMMITTABLE>::const_iterator io1::Listing<COMMITTABLE>::group_range(QString description, QDate date, const_range statements)
{
  assert(statements_.end() >= statements.end());
  switch (statements.size())
  {
    case 0: // return statements.end(), which is the same as statements.begin().
    case 1: return statements.begin();
    default: break;
  }

  std::vector<Entry> combined_entries;
  for (auto const & statement : statements)
  {
    if (!statement.is_composed()) combined_entries.push_back(statement.main_entry());
    else
    {
      boost::range::copy(statement.composed_entries(), std::back_inserter(combined_entries));
    }
  }

  auto const position = statements_.erase(statements.begin(), statements.end());
  return statements_.emplace(position, std::move(description), std::move(date), std::move(combined_entries));
}

template<typename COMMITTABLE> typename io1::Listing<COMMITTABLE>::const_range io1::Listing<COMMITTABLE>::gather_selection(statement_selection const & selected_statements)
{
  switch (selected_statements.size())
  {
    case 0: return boost::make_iterator_range(statements_.end(),statements_.end());
    case 1:
    {
      auto const statement = *selected_statements.begin();
      assert(statements_.end() > statement);
      return boost::make_iterator_range(statement,statement+1);
    }
    default: break;
  }

  // Moves the selected elements next to the last one so as to define a range that will then be range_grouped.

  // begin and end of the range that will be ranged_grouped.
  auto const end_range = 1+*selected_statements.rbegin();
  auto begin_range = end_range; // Will be set to the right value throughout the for loop below.

  // rotate each selected iterator to the right, and update begin_range accordingly.
  for (auto const_statement : selected_statements | boost::adaptors::reversed)
  {
    move_statement_up(const_statement,--begin_range);
  }

  // ready to call range_group.
  return boost::make_iterator_range(begin_range, end_range);
}

template<typename COMMITTABLE> void io1::Listing<COMMITTABLE>::move_statement(const_iterator statement, const_iterator position)
{
  assert(statements_.end() > statement);
  return (statement < position) ? move_statement_up(statement,position) : move_statement_down(statement,position);
}

template<typename COMMITTABLE> void io1::Listing<COMMITTABLE>::move_statement_up(const_iterator statement, const_iterator position)
{
  assert(statements_.end() > statement);
  assert(statements_.end() > position);
  assert(statement <= position); // otherwise we are actually trying to move down.

  auto const non_const_reversed_statement = std::make_reverse_iterator(remove_const(statement+1));
  auto const non_const_reversed_position = std::make_reverse_iterator(remove_const(position+1));

  std::rotate(non_const_reversed_position, non_const_reversed_statement, non_const_reversed_statement+1 );
  return;
}

template<typename COMMITTABLE> void io1::Listing<COMMITTABLE>::move_statement_down(const_iterator statement, const_iterator position)
{
  assert(statements_.end() > statement);
  assert(statements_.end() > position);
  assert(statement >= position); // otherwise we are actually moving up.

  auto const non_const_statement = remove_const(statement);
  std::rotate(remove_const(position), non_const_statement, non_const_statement+1);
  return;
}

template<typename COMMITTABLE> typename io1::Listing<COMMITTABLE>::const_range io1::Listing<COMMITTABLE>::split_statement(const_iterator statement)
{
  assert(statements_.end() > statement);
  if (!statement->is_composed()) return boost::make_iterator_range(statement,statement+1);

  auto const entries = statement->composed_entries();
  auto const nb_entries = entries.size();

  // we simply alter the first statement inplace.
  alter_statement(statement++,*entries.begin());

  // the others need to be inserted right after.
  std::vector<statement_type> new_statements;
  for (auto const & entry: entries | boost::adaptors::sliced(1,entries.size()))
    new_statements.emplace_back(entry);

  auto const begin_range = --statements_.insert(statement,std::make_move_iterator(new_statements.begin()),std::make_move_iterator(new_statements.end()));

  return boost::make_iterator_range(begin_range,begin_range+nb_entries);
}

template<typename COMMITTABLE> void io1::Listing<COMMITTABLE>::swap_statements(const_iterator position1, const_iterator position2)
{
  assert(statements_.end() > position1);
  assert(statements_.end() > position2);

  auto & statement1 = const_cast<statement_type &>(*position1);
  auto & statement2 = const_cast<statement_type &>(*position2);

  return std::swap(statement1, statement2);
}

template<typename COMMITTABLE> io1::Listing<COMMITTABLE> io1::Listing<COMMITTABLE>::read(std::istream & stream)
{
  std::string title;
  std::getline(stream >> std::ws, title);

  io1::Listing<COMMITTABLE> listing{ QString::fromStdString(title) };

  while (stream)
  {
    statement_type statement;
    stream >> std::ws >> statement;
    listing.statements_.push_back(std::move(statement));
  }

  return listing;
}

template<typename COMMITTABLE> std::ostream & io1::Listing<COMMITTABLE>::write(std::ostream & stream) const
{
  stream << boost::format("\n%1%\n\n") % name_.toStdString();
  for (auto const & statement: statements())
    stream << statement;

  return stream;
}

template<typename COMMITTABLE> std::ostream & io1::operator<<(std::ostream & stream, Listing<COMMITTABLE> const & listing)
{
  return listing.write(stream);
}

template<typename COMMITTABLE> std::istream & io1::operator>>(std::istream & stream, Listing<COMMITTABLE> & listing)
{
  listing = Listing<COMMITTABLE>::read(stream);
  assert(!stream);
  return stream;
}

template<typename COMMITTABLE> bool io1::Listing<COMMITTABLE>::equals(Listing const & rhs) const
{
  return (statements_ == rhs.statements_);
}

template<typename COMMITTABLE> typename io1::Listing<COMMITTABLE>::vector_type::iterator io1::Listing<COMMITTABLE>::remove_const(const_iterator statement)
{
  return statements_.erase(statement,statement);
}

namespace io1
{
  template class Listing<committable_tag>;
  template class Listing<non_committable_tag>;

  template std::ostream & operator<<(std::ostream & stream, Listing<committable_tag> const & listing);
  template std::ostream & operator<<(std::ostream & stream, Listing<non_committable_tag> const & listing);

  template std::istream & operator>>(std::istream & stream, Listing<committable_tag> & listing);
  template std::istream & operator>>(std::istream & stream, Listing<non_committable_tag> & listing);

  template bool operator==(Listing<committable_tag> const & lhs, Listing<committable_tag> const & rhs);
  template bool operator==(Listing<non_committable_tag> const & lhs, Listing<non_committable_tag> const & rhs);

}
