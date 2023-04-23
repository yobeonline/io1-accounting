/// \file account.hpp
#pragma once
#ifndef IO1_ACCOUNT_HPP
#define IO1_ACCOUNT_HPP

#include "money.hpp"
#include "listing.hpp"
#include "archived_listing.hpp"

#include <vector>
#include <QString>
#include <QDate>

namespace io1 {
  
  /// Models an account.
  ///
  ///.
  class Account
  {
  public:
    using current_listing_type = Listing<committable_tag>;

  public:
    Account(void) =default;
    explicit Account(Money initial_balance, QString currency = QString());
    explicit Account(Money initial_balance, QDate date, QString currency = QString());

    Account & set_name(QString const & name) { current_listing_.set_name(name); return *this; };
    Account & set_description(QString const & description) { description_ = description; return *this; };

  public:
    current_listing_type & current_listing(void) { return current_listing_; };
    current_listing_type const & current_listing(void) const { return current_listing_; };

    std::vector<ArchivedListing> const & archived_listings(void) const { return archived_listings_; };

    Money balance(void) const;
    Money archived_balance(void) const;

    void archive(QString name);

  public:
    std::ostream & write(std::ostream & stream) const;
    static Account read(std::istream & stream);

  private:
    explicit Account(current_listing_type current_listing, std::vector<ArchivedListing> archives, QString description, QString currency = QString());

  private:
    QString description_;
    QString currency_;
    current_listing_type current_listing_;
    std::vector<ArchivedListing> archived_listings_;
  };

  std::ostream & operator<<(std::ostream & stream, Account const & account);
  std::istream & operator>>(std::istream & stream, Account & account);

  void save_as(boost::filesystem::path const & path, Account const & account);
  io1::Account open(boost::filesystem::path const & path);
}

#endif