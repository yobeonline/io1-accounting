#include "io1/entry.hpp"
#include <format>
#include <iostream>
/*#include <iomanip>
#include <boost/format.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception_ptr.hpp>
#include "date_formatter.hpp"
#include "accounting_exception.hpp"
#include "blank.hpp"*/

// Constructs an entry with the given amount, dated today.
io1::Entry::Entry(Money amount) noexcept
:Entry(amount, {})
{}

namespace
{
  [[nodiscard]] auto now() noexcept
  {
    const std::chrono::time_point timepoint{ std::chrono::system_clock::now() };
    return std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(timepoint) };
  }
}

// Constructs an entry with the given amount and description, dated today.
io1::Entry::Entry(Money amount, std::string description) noexcept
:Entry(amount,std::move(description),now())
{}

// Constructs an entry with the given amount, description and date.
io1::Entry::Entry(Money amount, std::string description, std::chrono::year_month_day date) noexcept
  :amount_(amount)
  , description_(std::move(description))
  , date_(std::move(date))
{
  assert(date.ok() && "Precondition: all dates ever constructed are assumed ok.");
}

// Formats an entry into an std::ostream.
std::ostream & io1::Entry::write(std::ostream & stream) const
{
  return stream << std::format("{:<10}{:<10}{}\n", date_, amount_.data(), description_);
}

/*// Reads an entry from an std::istream.
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
}*/