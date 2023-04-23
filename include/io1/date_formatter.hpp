/// \file date_format.hpp
#pragma once

#include <QDate>
#include <iosfwd>

namespace io1
{
  class date_formatter
  {
    public:
      explicit date_formatter(QDate const & date);
      friend std::ostream & operator<< (std::ostream &, date_formatter const d);

      static std::string sample_date(void);

    private:
      QDate const * date_;
  };

  std::istream & operator>> (std::istream &, QDate & d);
  std::ostream & operator<< (std::ostream & stream, date_formatter const d);
}
