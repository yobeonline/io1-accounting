#include "account.hpp"
#include <iomanip>
#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/numeric.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/errinfo_file_name.hpp>
#include <boost/exception/errinfo_errno.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception_ptr.hpp>
#include "date_formatter.hpp"
#include "sha1_sum_filter.hpp"
#include "calculate_current_line.hpp"
#include "accounting_exception.hpp"

namespace
{
  auto const currency_marker = "\xc2\xa4";

  template <class RANGE> io1::Money accumulate (RANGE const & range, io1::Money initial_value)
  {
    return boost::accumulate(range,initial_value,[](io1::Money acc, typename RANGE::const_iterator::reference statement){ return acc += statement.amount(); });
  };

  std::string const listing_extension = ".lst";

  std::string write_statements(io1::Account::current_listing_type const & statements, boost::filesystem::path const & filename)
  {
    boost::iostreams::filtering_ostream out;
    auto const filter = io1::push<io1::sha1_sum_filter>(out);
    assert(filter);

    {
      boost::filesystem::ofstream file(filename);
      out.push(file);
      out << statements;
    }

    return filter->read_sha1();
  };

  bool starts_with_sha1(std::string const & line)
  {
    return (40 == line.find_first_not_of("0123456789abcdef"));
  };

}

io1::Account::Account(Money initial_balance, QString currency)
:Account(initial_balance,QDate::currentDate(),QString())
{}

io1::Account::Account(Money initial_balance, QDate date, QString currency)
:currency_(std::move(currency))
{
  auto const st_it = current_listing_.add_statement(initial_balance,"Initial balance.",std::move(date));
  st_it->set_committed();
}

io1::Account::Account(current_listing_type current_listing, std::vector<ArchivedListing> archives, QString description, QString currency)
:current_listing_(std::move(current_listing))
,archived_listings_(std::move(archives))
,description_(std::move(description))
,currency_(std::move(currency))
{}

io1::Money io1::Account::balance(void) const
{
  return accumulate(current_listing_,archived_balance());
}

io1::Money io1::Account::archived_balance(void) const
{
  return archived_listings_.empty() ? 0_USD : archived_listings_.back().final_balance();
}

void io1::Account::archive(QString name)
{
  using statement_type = typename current_listing_type::statement_type;
  
  auto const committed_range = current_listing_.statements() | boost::adaptors::filtered([](statement_type const & st) {return st.is_committed(); });
  ArchivedListing::listing_type archived_listing(std::move(name),committed_range);
  archived_listing.stable_sort();

  auto const un_committed_range = current_listing_.statements() | boost::adaptors::filtered([](statement_type const & st) {return !st.is_committed(); });
  current_listing_type new_current_listing(current_listing_.name(),un_committed_range);

  auto const new_archived_balance = accumulate(archived_listing, archived_balance());

  QDate const archived_date = archived_listing.statements().back().date();
  auto const archive_filename = name.toStdString() + listing_extension;

  archived_listings_.emplace_back(archive_filename, std::move(archived_listing), archived_date, new_archived_balance);
  std::swap(current_listing_, new_current_listing);

  return;
}

std::ostream & io1::Account::write(std::ostream & stream) const
{
  stream << description_.toStdString() << "\n\n";

  if (!currency_.isEmpty()) stream << currency_marker << currency_.toStdString() << "\n\n";
  
  auto const current_listing_filename = current_listing_.name().toStdString() + listing_extension;
  auto const current_listing_sha1 = write_statements(current_listing_,current_listing_filename);

  stream << boost::format("%1%\t%2%\n") % current_listing_sha1 % current_listing_filename;

  if (!archived_listings_.empty())
  {
    for (auto const & ar: archived_listings_) stream << ar;
  }

  return stream;
}

std::ostream & io1::operator<<(std::ostream & stream, Account const & account)
{
  return account.write(stream);
}

io1::Account io1::Account::read(std::istream & stream)
{
  std::string line;
  std::string description;
  std::string currency;

  std::getline(stream >> std::ws, line);
  while (stream && !starts_with_sha1(line))
  {
    if (0 == line.find(currency_marker))
    {
      if (currency.empty()) currency = line.substr(1);
      else BOOST_THROW_EXCEPTION(ParseError() << ParseError::errinfo_class_name("Account")); // a currency was already read cannot overwrite it.
    }
    else description += line + '\n';

    std::getline(stream >> std::ws,line);
  }
  if (!stream) BOOST_THROW_EXCEPTION(ParseError() << ParseError::errinfo_class_name("Account")); // file read error, a sha1 was expected at some point.

  auto const sha1 = line.substr(0,40);
  boost::filesystem::path filename = line.substr(40);

  boost::iostreams::filtering_istream in;

  auto const sha1_sum = push<sha1_sum_filter>(in);
  assert(sha1_sum);

  current_listing_type current_listing;
  {
    boost::filesystem::ifstream file(filename);
    if (!file) BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_errno(errno) << boost::errinfo_file_name(filename.string()));

    in.push(file);
    
    try
    {
      in >> current_listing;
    }
    catch (ParseError & e)
    {
      e << ParseError::errinfo_line_number(calculate_current_line(file));
      BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_file_name(filename.string()) << boost::errinfo_nested_exception(boost::current_exception()));
    }
  }

  auto const actual_sha1 = sha1_sum->read_sha1();
  if (sha1 != actual_sha1) BOOST_THROW_EXCEPTION(Sha1Mismatch() << Sha1Mismatch::errinfo_actual(actual_sha1) << Sha1Mismatch::errinfo_expected(sha1));

  std::vector<ArchivedListing> archives;
  while ((stream >> std::ws).good())
  {
    ArchivedListing ar;
    stream >> std::ws >> ar;
    archives.push_back(std::move(ar));
  }

  return Account(std::move(current_listing),std::move(archives),QString::fromStdString(description),QString::fromStdString(currency).trimmed());
}

std::istream & io1::operator>>(std::istream & stream, Account & account)
{
  account = Account::read(stream);
  return stream;
}

void io1::save_as(boost::filesystem::path const & path, Account const & account)
{
  boost::filesystem::ofstream file(path);
  
  if (!file) BOOST_THROW_EXCEPTION(FileWriteError() << boost::errinfo_errno(errno) << boost::errinfo_file_name(path.string()));
  
  try
  {
    file << account;
  }
  catch (boost::exception const &)
  {
    BOOST_THROW_EXCEPTION(FileWriteError() << boost::errinfo_file_name(path.string()) << boost::errinfo_nested_exception(boost::current_exception()));
  }

  return;
}

io1::Account io1::open(boost::filesystem::path const & path)
{
  boost::filesystem::ifstream file(path);
  if (!file) BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_errno(errno) << boost::errinfo_file_name(path.string()));

  try
  {
    try
    {
      return Account::read(file);
    }
    catch (ParseError & e)
    {
      e << ParseError::errinfo_line_number(calculate_current_line(file));
      throw;
    }
  }
  catch (boost::exception const &)
  {
    BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_file_name(path.string()) << boost::errinfo_nested_exception(boost::current_exception()));
  }
}
