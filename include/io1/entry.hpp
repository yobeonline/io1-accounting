/// \file entry.hpp
#pragma once
#ifndef IO1_ENTRY_HPP
#define IO1_ENTRY_HPP

#include "money.hpp"
#include <QDate>
#include <QString>
#include <iosfwd>

namespace io1
{
  /// A class that models the details of a statement.
  ///
  /// An entry is similar to a statement except that many entries may be required to account for a single statement.
  /// For example, a statement might consist of a withdraw of $20.00 with a $1.00 tax and hence appear
  /// as a $21.00 statement composed of a $20.00 entry and $1.00 entry.
  class Entry
  {
  public:
    Entry(void) =default; /// Constructs an uninitialized entry.
    explicit Entry(Money amount); /// Constructs an entry with the given amount, dated today.
    explicit Entry(Money amount, QString const & description); /// Constructs an entry with the given amount and description, dated today.
    explicit Entry(Money amount, QString const & description, QDate date); /// Constructs an entry with the given amount, description and date.

  public:
    QDate const & date(void) const { return date_; }; /// Returns the date of the entry.
    QString const & description(void) const { return description_; }; /// Returns the description of the entry. The returned string is trimmed.
    Money amount(void) const { return amount_; }; /// Returns the amount of the entry.

  public:
    std::ostream & write(std::ostream & stream) const; /// Formats the entry into a std::ostream using UTF8. It can be re-read with the read function.
    static Entry read(std::istream & stream); /// Reads an entry from a UTF8 std::istream.
    bool equals(Entry const & rhs) const; /// Returns true if rhs equals the object.

  private:
    Money amount_; /// The amount of the entry.
    QDate date_; /// the date of the entry.
    QString description_; /// the trimmed description of the entry.
  };

  /// Free function to format an entry into a std::ostream.
  inline std::ostream & operator<<(std::ostream & stream, Entry const & entry) { return entry.write(stream); };

  /// Free function to read an entry from a std::istream.
  inline std::istream & operator>>(std::istream & stream, Entry & entry) { entry = Entry::read(stream); return stream; };

  /// Free function to compare two entries.
  inline bool operator==(Entry const & lhs, Entry const & rhs) { return lhs.equals(rhs); };
}

#endif
