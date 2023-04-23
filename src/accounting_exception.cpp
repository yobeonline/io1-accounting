#include "accounting_exception.hpp"
#include <boost/exception/get_error_info.hpp>
#include <boost/format.hpp>
#include "date_formatter.hpp"

void io1::InvalidAmountFormat::format_message(void) const
{
  if (auto const amount = boost::get_error_info<errinfo_amount>(*this))
    message_ = str(boost::format("%1$#g: Invalid amount format.") % *amount);
  else
    message_ = "Invalid amount format.";

  message_ += "\nThere cannot be more than two decimal digits in an entry's amount.";

  return;
}

void io1::InvalidDate::format_message(void) const
{
  message_ = "Invalid date.";
  return;
}

void io1::InvalidDateFormat::format_message(void) const
{
  if (auto const date = boost::get_error_info<errinfo_date_string>(*this))
    message_ = str(boost::format("%1% is not a valid date.") % *date);
  else
    message_ = "Invalid date format.";

  message_ += str(boost::format("\nThe expected date format is: %1%.") % date_formatter::sample_date());

  return;
}

void io1::ParseError::format_message(void) const
{
  if (auto const line_number = boost::get_error_info<errinfo_line_number>(*this))
    message_ = str(boost::format("Line %1%: ") % *line_number);

  message_ += "Parse error";

  if (auto const class_name = boost::get_error_info<errinfo_class_name>(*this))
    message_ += " while reading a " + *class_name;

  message_ += ".";

  return;
}
