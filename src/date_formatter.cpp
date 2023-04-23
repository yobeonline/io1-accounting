#include "date_formatter.hpp"

#include <array>
#include "accounting_exception.hpp"
#include <boost/throw_exception.hpp>


namespace
{
  auto const format = Qt::RFC2822Date;
  auto const format_size = 11;
}

io1::date_formatter::date_formatter(QDate const & date)
:date_(&date)
{}

std::string io1::date_formatter::sample_date(void)
{
  return QDate(2018, 02, 28).toString(format).toStdString();
}

std::ostream & io1::operator<< (std::ostream & stream, date_formatter const date)
{
  return stream << date.date_->toString(format).toStdString();
}

std::istream & io1::operator>> (std::istream & stream, QDate & date)
{
  std::array<char, format_size + 1> date_c_str;
  date_c_str.back() = '\0';
  stream.read(date_c_str.data(), format_size);

  date = QDate::fromString(QString(date_c_str.data()), format);
  if (!date.isValid()) BOOST_THROW_EXCEPTION(io1::InvalidDateFormat{} << io1::InvalidDateFormat::errinfo_date_string{ date_c_str.data() });

  return stream;
}
