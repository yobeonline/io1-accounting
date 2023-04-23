/// \file statement.hpp
#pragma once
#ifndef IO1_STATEMENT_HPP
#define IO1_STATEMENT_HPP

#include <iosfwd>

#include <boost/container/small_vector.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/numeric.hpp>

#include "money.hpp"
#include "entry.hpp"
#include "exception.hpp"

#include <vector>

namespace io1
{
  /// A class that models a statement in an account listing.
  ///
  /// A statement usually matches a single bank operation but may also match several.
  /// For example, a deposit of three cheques is a single statement in a listing and is yet composed of three entries.
  class Statement
  {
  private:
    using small_vector_t = boost::container::small_vector<Entry, 1>; // most vector instances will only have one element, so we small-size optimize for that.
    // Whenever the statement is composed of N>1 entries, the vector is laid out the following way
    // [0] -> The main entry, ie. a single entry view of the statement.
    // [1]..[N] -> The composed entries.
    // otherwise the main entry is the single entry.

  public:
    using const_iterator = small_vector_t::const_iterator;
    using const_range = boost::iterator_range<const_iterator>;

  public:
    Statement(void) =default; /// Creates a statement with undefined content (class invariants are not established). Objects built from it can only be assigned.
    explicit Statement(Entry entry); /// Creates a statement from the given entry.
    explicit Statement(Money amount, QString const & description); /// Creates a statement with the given amount and description, dated today.
    explicit Statement(Money amount, QString const & description, QDate date); /// Creates a statement with the given amount, description and date.
    template<typename RANGE> explicit Statement(QString const & description, QDate date, RANGE range); /// Creates a statement from a range of entries with the given description and date.

  public:
    QDate const & date(void) const { return main_entry().date(); }; /// Returns the date of the statement.
    QString const & description(void) const { return main_entry().description(); }; /// Returns the trimmed description of the statement.
    Money amount(void) const { return main_entry().amount(); }; /// Returns the total amount of the statement.

  public:
    Entry const & main_entry(void) const { return entries_.front(); }; /// Returns a single entry view of the statement.
    const_range composed_entries(void) const; /// Returns the range of composed entries. Returns an empty range if is_composed() returns false.
    bool is_composed(void) const { return 1 < entries_.size(); }; /// Returns true if the statement is more than a single entry.
    std::size_t entry_count(void) const; /// Returns the number of composed entries. Returns zero if is_composed() returns false.

  public:
    std::ostream & write(std::ostream & stream) const { return write_impl(stream); }; /// Writes the statement into a std::ostream using UTF8.
    static Statement read(std::istream & stream); /// Reads a statement from a UTF8 std::istream.
    bool equals(Statement const & rhs) const; /// Returns true if rhs equals the object.

  protected:
    std::ostream & write_impl(std::ostream & stream, char const * prefix ="") const; /// Writes the statement into a std::ostream using UTF8 and prepending a prefix to each line.
    template<typename STATEMENT> static STATEMENT read_impl(std::istream & stream); /// Reads a generic statement from a stream using UTF8.

  private:
    small_vector_t entries_; // The main entry followed by the composed ones if any.
  };

  /// Exception thrown when a composed statement read from a stream has amounts that don't match.
  /// The amount of the main entry doesn't match the total amount of the composite entries.
  struct AmountMismatch : virtual Exception
  {
    using errinfo_entry_list = boost::error_info<struct tag_entry_list, std::vector<Entry>>;
    using errinfo_main_entry = boost::error_info<struct tag_main_entry, Entry>;
    void format_message(void) const override;
  };

  /// Free function to format a statement into a std::ostream.
  inline std::ostream & operator<<(std::ostream & stream, Statement const & statement) { return statement.write(stream); };

  /// Free function to read a statement from a std::istream.
  inline std::istream & operator>>(std::istream & stream, Statement & statement) { statement = Statement::read(stream); return stream; };

  /// Free function to compare two statements.
  inline bool operator==(Statement const & lhs, Statement const & rhs) { return lhs.equals(rhs); };

  /// A statement with a commit state.
  class CommittableStatement: public Statement
  {
  public:
    template <class... ARGS> explicit CommittableStatement(ARGS && ... args):Statement(std::forward<ARGS>(args)...) {}; /// Forwards construction to Statement.

    bool is_committed() const { return is_committed_; }; /// Returns the commit state of the statement.
    void set_committed(bool committed=true) const { is_committed_ = committed; }; /// changes the commit state of the statement.

    std::ostream & write(std::ostream & stream) const; /// Formats the statement into a std::ostream using UTF8.
    static CommittableStatement read(std::istream & stream); /// Reads a statement from a UTF8 std::istream.

  private:
    mutable bool is_committed_{ false }; // the boolean thet holds the commit state.
  };

  /// Free function to format a committable statement into a std::ostream.
  inline std::ostream & operator<<(std::ostream & stream, CommittableStatement const & statement) { return statement.write(stream); };

  /// Free function to read a committable statement from a std::istream.
  inline std::istream & operator>>(std::istream & stream, CommittableStatement & statement) { statement = CommittableStatement::read(stream); return stream; };

  /// Free function to compare two committable statements.
  bool operator==(CommittableStatement const & lhs, CommittableStatement const & rhs);

}

// Constructor from a range of entries.
template<typename RANGE> io1::Statement::Statement(QString const & description, QDate date, RANGE range)
{
  auto const size = range.size();
  switch (size)
  {
    case 0: throw std::range_error("Cannot construct a composed Statement object out of an empty range.");
    case 1: entries_.emplace_back(*std::make_move_iterator(range.begin())); break;
    default:
    {
      entries_.reserve(1+ size);

      auto const total_amount = boost::accumulate(range, 0_USD, [](Money amount, Entry const & entry) { return amount += entry.amount(); });
      entries_.emplace_back(total_amount, description, std::move(date));
      entries_.insert(entries_.end(), std::make_move_iterator(range.begin()), std::make_move_iterator(range.end()));
    }
  }
  assert(1 <= entries_.size());
}

#endif
