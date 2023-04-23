/// \file entry.cpp
#include "entry.hpp"
#include <iostream>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception_ptr.hpp>
#include "date_formatter.hpp"
#include "accounting_exception.hpp"
#include "blank.hpp"

// Constructs an entry with the given amount, dated today.
io1::Entry::Entry(Money amount)
:Entry(std::move(amount),QString{})
{}

// Constructs an entry with the given amount and description, dated today.
io1::Entry::Entry(Money amount, QString const & description)
:Entry(std::move(amount),description,QDate::currentDate())
{}

// Constructs an entry with the given amount, description and date.
io1::Entry::Entry(Money amount, QString const & description, QDate date)
:amount_(std::move(amount))
,description_(description.trimmed())
,date_(std::move(date))
{
  if (!amount_.is_within_n_decimals<2>()) BOOST_THROW_EXCEPTION(InvalidAmountFormat{} << InvalidAmountFormat::errinfo_amount{amount_});
  if (!date_.isValid()) BOOST_THROW_EXCEPTION(InvalidDate{});
}

// Formats an entry into an std::ostream.
std::ostream & io1::Entry::write(std::ostream & stream) const
{
  stream << boost::format("%1%%|10t|%2$15.2f%|30t|%3%\n")
              % date_formatter(date_)
              % amount_
              % description_.toStdString();

  return stream;
}

// Reads an entry from an std::istream.
io1::Entry io1::Entry::read(std::istream & stream)
{
  QDate date;
  stream >> blank >> date;

  Money amount;
  stream >> amount;

  std::string description;
  std::getline(stream >> blank,description);

  if (!stream) BOOST_THROW_EXCEPTION(ParseError{} << ParseError::errinfo_class_name{"Entry"});

  return Entry{amount, QString::fromStdString(description), std::move(date)};
}

// Returns true if rhs is the same as the object.
bool io1::Entry::equals(Entry const & rhs) const
{
  return (description_ == rhs.description_ && date_ == rhs.date_ && amount_ == rhs.amount_);
}