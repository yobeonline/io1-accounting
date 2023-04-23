#include "archived_listing.hpp"
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/errinfo_file_name.hpp>
#include <boost/exception/errinfo_errno.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/format.hpp>
#include "accounting_exception.hpp"
#include "date_formatter.hpp"
#include "sha1_sum.hpp"
#include "sha1_sum_filter.hpp"
#include "calculate_current_line.hpp"

io1::ArchivedListing::ArchivedListing(path_type filename, QDate final_date, Money final_balance, std::string sha1)
:filename_(std::move(filename))
,final_date_(std::move(final_date))
,final_balance_(final_balance)
,sha1_(std::move(sha1))
{
  if (!final_balance_.is_within_n_decimals<2>()) BOOST_THROW_EXCEPTION(InvalidAmountFormat() << InvalidAmountFormat::errinfo_amount{ final_balance_ });
  if (!final_date_.isValid()) BOOST_THROW_EXCEPTION(InvalidDate());
  if (!boost::filesystem::exists(filename_)) BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_errno(ENOENT) << boost::errinfo_file_name(filename_.string()));
  if (boost::filesystem::is_directory(filename_)) BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_errno(EISDIR) << boost::errinfo_file_name(filename_.string()));

  auto const actual_sha1 = sha1_sum(filename_);
  if (actual_sha1 != sha1_) BOOST_THROW_EXCEPTION(Sha1Mismatch() << Sha1Mismatch::errinfo_expected(sha1_) << Sha1Mismatch::errinfo_actual(actual_sha1));
}

io1::ArchivedListing::ArchivedListing(path_type filename, listing_type listing, QDate final_date, Money final_balance)
:filename_(std::move(filename))
,listing_(std::move(listing))
,final_date_(std::move(final_date))
,final_balance_(final_balance)
{
  if (!final_balance_.is_within_n_decimals<2>()) BOOST_THROW_EXCEPTION(InvalidAmountFormat() << InvalidAmountFormat::errinfo_amount{ final_balance_ });
  if (!final_date_.isValid()) BOOST_THROW_EXCEPTION(InvalidDate());

  (*listing_).stable_sort();

  if (boost::filesystem::exists(filename_)) BOOST_THROW_EXCEPTION(FileWriteError() << boost::errinfo_errno(EEXIST) << boost::errinfo_file_name(filename_.string()));

  {
    boost::iostreams::filtering_ostream out;
    auto const filter = push<sha1_sum_filter>(out);
    assert(filter);

    {
      boost::filesystem::ofstream file(filename_);
      if (!file) BOOST_THROW_EXCEPTION(FileWriteError() << boost::errinfo_errno(errno) << boost::errinfo_file_name(filename_.string()));
      out.push(file);
      out << *listing_ << std::flush;
    }

    sha1_ = filter->read_sha1();
  }
}

io1::ArchivedListing::listing_type const & io1::ArchivedListing::listing(void) const
{
  if (!listing_)
  {
    boost::filesystem::ifstream file{ filename_ };
    if (!file) BOOST_THROW_EXCEPTION(FileReadError() << boost::errinfo_errno(errno) << boost::errinfo_file_name(filename_.string()));
    
    boost::iostreams::filtering_istream in;

    auto const filter = push<sha1_sum_filter>(in);
    assert(filter);
    
    in.push(file);

    try
    {
      try
      {
        listing_ = listing_type::read(in);
        auto const actual_sha1 = filter->read_sha1();

        if (sha1_ != actual_sha1) BOOST_THROW_EXCEPTION(Sha1Mismatch() << Sha1Mismatch::errinfo_expected(sha1_) << Sha1Mismatch::errinfo_actual(actual_sha1));
      }
      catch (ParseError & e)
      {
        e << ParseError::errinfo_line_number(calculate_current_line(file));
        throw;
      }
    }
    catch(...)
    {
      BOOST_THROW_EXCEPTION(CorruptedFile{} << boost::errinfo_file_name(filename_.string()) << boost::errinfo_nested_exception(boost::current_exception()));
    }
  }

  return *listing_;
}

std::ostream & io1::ArchivedListing::write(std::ostream & stream) const
{
  stream << boost::format("%1% %2$_#15.2f%\t%3% %4%") % date_formatter(final_date_) % final_balance_ % sha1_ % filename_;
  return stream;
}

io1::ArchivedListing io1::ArchivedListing::read(std::istream & stream)
{
  Money balance;
  QDate date;
  path_type filename;
  std::string sha1;

  stream >> std::ws >> date;
  stream >> balance;
  stream >> sha1;
  stream >> filename;

  if (!stream) BOOST_THROW_EXCEPTION(ParseError() << ParseError::errinfo_class_name("Archived Listing"));

  return ArchivedListing{std::move(filename),std::move(date),std::move(balance),std::move(sha1)};
}

std::ostream & io1::operator<<(std::ostream & stream, ArchivedListing const & archive)
{
  return archive.write(stream);
}

std::istream & io1::operator>>(std::istream & stream, ArchivedListing & archive)
{
  archive = ArchivedListing::read(stream);
  return stream;
}