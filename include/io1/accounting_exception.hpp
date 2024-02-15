#pragma once
#include "io1/exception.hpp"
#include <io1/money.hpp>

namespace io1
{
  /// Exception thrown when trying to construct an entry with an amount that counts more than two decimal digits.
  struct InvalidAmountFormat : virtual Exception
  {
    using errinfo_amount = boost::error_info<struct tag_amount, Money>;
    void format_message(void) const override;
  };

  struct InvalidDate : virtual Exception
  {
    void format_message(void) const override;
  };

  struct InvalidDateFormat : virtual Exception
  {
    using errinfo_date_string = boost::error_info<struct tag_date_string, std::string>;
    void format_message(void) const override;
  };

  struct ParseError : virtual Exception
  {
    using errinfo_line_number = boost::error_info<struct tag_line_number, std::size_t>;
    using errinfo_class_name = boost::error_info<struct tag_class_name, std::string>;
    void format_message(void) const override;
  };


}