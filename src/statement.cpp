/// \file statement.cpp
#include "io1/statement.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

#include <boost/format.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/throw_exception.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/numeric.hpp>

#include "blank.hpp"

namespace
{
  auto const composed_char = '-';
  auto const committed_char = '#';
}

// Constructor from an amount and a description.
io1::Statement::Statement(Money amount, QString const & description)
:Statement(Entry{std::move(amount),description})
{}

// Constructor from an amount, a description and a date.
io1::Statement::Statement(Money amount, QString const & description, QDate date)
:Statement(Entry{std::move(amount),description,std::move(date)})
{}

// Constructor from an entry.
io1::Statement::Statement(Entry entry)
:entries_(1,std::move(entry))
{
  assert(1 == entries_.size());
}

// Returns a range over the composed entries.
io1::Statement::const_range io1::Statement::composed_entries(void) const
{
  assert(!entries_.empty()); // there should be at least the main entry.
  return boost::make_iterator_range(++entries_.begin(), entries_.end());
}

// Returns the number of composed entries.
std::size_t io1::Statement::entry_count(void) const
{
  return is_composed() ? entries_.size() -1 : 0;
}

// Formats a statement in a stream.
std::ostream & io1::Statement::write_impl(std::ostream & stream, char const * prefix) const
{
  assert(prefix);

  stream << prefix << entries_.front();
  auto const padding = std::string(std::strlen(prefix),' ');

  for (auto const & composed_entry : composed_entries())
  {
    stream << padding << composed_char << ' ' << composed_entry;
  }

  return stream;
}

// Reads a generic statement (either Statement or CommittableStatement) from a stream.
template<typename STATEMENT> STATEMENT io1::Statement::read_impl(std::istream & stream)
{
  auto const main_entry = Entry::read(stream);

  auto amount = main_entry.amount();

  std::vector<Entry> entries;
  while (composed_char == (stream >> blank).peek())
  {
    stream.ignore(); // consume the composed char
    entries.push_back(Entry::read(stream));
    amount -= entries.back().amount();
  }

  if (entries.empty()) return STATEMENT{ std::move(main_entry) };

  if (0_USD != amount) BOOST_THROW_EXCEPTION(AmountMismatch{} << AmountMismatch::errinfo_main_entry{ std::move(main_entry) } << AmountMismatch::errinfo_entry_list{ std::move(entries) });

  return STATEMENT{ main_entry.description(),main_entry.date(),std::move(entries) };
}

// Reads a statement from a stream.
io1::Statement io1::Statement::read(std::istream & stream)
{
  return read_impl<Statement>(stream);
}


// Constructs an AmountMismatch exception.
void io1::AmountMismatch::format_message(void) const
{
  message_ = "Amount mismatch.";

  if (auto const main_entry = boost::get_error_info<errinfo_main_entry>(*this))
  {
    message_ += str(boost::format("\nThe expected total amount is %1$#g.") % main_entry->amount());
  }

  if (auto const entry_list = boost::get_error_info<errinfo_entry_list>(*this))
  {
    auto const amount_list = *entry_list | boost::adaptors::transformed([](Entry const & e) { return e.amount(); });
    auto const total_amount = boost::accumulate(amount_list,0_USD);
    message_ += str(boost::format("\nThe actual total amount is %1$#g.") % total_amount);
  }
  
  return;
};

// Returns true if rhs is the same as the object.
bool io1::Statement::equals(Statement const & rhs) const
{
  return (entries_ == rhs.entries_);
}

// Formats a committable statement in a stream.
std::ostream & io1::CommittableStatement::write(std::ostream & stream) const
{
  char const prefix[3] = { committed_char, ' ', '\0' };
  return write_impl(stream, is_committed() ? prefix : "  ");
}

// Reads a committable statement from a stream.
io1::CommittableStatement io1::CommittableStatement::read(std::istream & stream)
{
  stream >> std::ws;
  
  bool committed = false;
  if (committed_char == stream.peek())
  {
    committed = true;
    stream.ignore(); // consume the committed char
  }

  auto statement = Statement::read_impl<CommittableStatement>(stream);
  statement.set_committed(committed);
  
  return statement;
}

// Returns true if lhs and rhs are equal.
bool io1::operator==(CommittableStatement const & lhs, CommittableStatement const & rhs)
{
  return (lhs.is_committed() == rhs.is_committed()) && lhs.equals(rhs);
}
