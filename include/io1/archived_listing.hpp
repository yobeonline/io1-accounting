/// \file archived_listing.hpp
#pragma once
#ifndef IO1_ARCHIVED_LISTING_HPP
#define IO1_ARCHIVED_LISTING_HPP

#include "io1/listing.hpp"
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace io1 {

  class ArchivedListing
  {
  public:
    using listing_type = Listing<non_committable_tag>;
    using path_type = boost::filesystem::path;

  private:
    using optional_listing_type = boost::optional<listing_type>;

  public:
    ArchivedListing(void) =default;
    explicit ArchivedListing(path_type filename, QDate final_date, Money final_balance, std::string sha1);
    explicit ArchivedListing(path_type filename, listing_type listing, QDate final_date, Money final_balance);

    listing_type const & listing(void) const;
    Money final_balance(void) const { return final_balance_; };

  public:
    std::ostream & write(std::ostream & stream) const;
    static ArchivedListing read(std::istream & stream);

  private:
    mutable optional_listing_type listing_;
    Money final_balance_;
    QDate final_date_;
    path_type filename_;
    std::string sha1_;
  };

  std::ostream & operator<<(std::ostream & stream, ArchivedListing const & archive);
  std::istream & operator>>(std::istream & stream, ArchivedListing & archive);
}

#endif