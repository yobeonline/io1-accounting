/// \file listing.hpp
#pragma once
#ifndef IO1_LISTING_HPP
#define IO1_LISTING_HPP

#include <vector>
#include <boost/range/istream_range.hpp>
#include <boost/container/flat_set.hpp>
#include <QString>
#include "statement.hpp"

namespace io1
{
  struct committable_tag;
  struct non_committable_tag;
  
  /// A class that models a bank account listing.
  ///
  /// This class only provides const access to statements once they are added.
  template<typename COMMITTABLE> class Listing
  {
  public:
    using statement_type = typename COMMITTABLE::statement_type;

  private:
    using vector_type = std::vector<statement_type>;

  public:
    using const_iterator = typename vector_type::const_iterator;
    using const_range = boost::iterator_range<const_iterator>;
    using statement_selection = boost::container::flat_set<const_iterator>;

  public:
    Listing(void) =default;
    explicit Listing(QString name);
    template<class RANGE> explicit Listing(QString name, RANGE const & range)
    :name_(std::move(name)),statements_(range.begin(),range.end())
    {};

  public:
    /// Adds a statement to the listing. Arguments are forwarded to the Statement constructor.
    template <class... ARGS> const_iterator add_statement(ARGS && ... args)
    {
      return statements_.emplace(statements_.end(),std::forward<ARGS>(args)...);
    };

    void sort(void);
    void stable_sort(void);
    const_iterator erase_statement(const_iterator position);
    const_iterator group_range(QString description, QDate date, const_range statements);
    const_range gather_selection(statement_selection const & selected_statements);
    const_range split_statement(const_iterator statement);
    void swap_statements(const_iterator position1, const_iterator position2);
    void move_statement_down(const_iterator statement, const_iterator position);
    void move_statement_up(const_iterator statement, const_iterator position);
    void move_statement(const_iterator statement, const_iterator position);

    /// Changes the designated statement for the one constructed from the remaining arguments.
    template <class... ARGS> void alter_statement(const_iterator position, ARGS && ... args)
    {
      // const_casting is faster than doing statements_.emplace(statements_.erase(position),std::forward<ARGS>(args)...).
      auto & statement_ref = const_cast<statement_type &>(*position);
      statement_ref = statement_type(std::forward<ARGS>(args)...);

      return;
    };

  public:
    QString const & name(void) const { return name_; }; /// Returns the name of the listing.
    void set_name (QString const & name) { name_ = name; }; /// Changes the name of the listing.

  public:
    const_range statements(void) const { return statements_; };
    const_iterator begin(void) const { return statements_.begin(); };
    const_iterator end(void) const { return statements_.end(); };

  public:
    std::ostream & write(std::ostream & stream) const;
    static Listing read(std::istream & stream);
    bool equals(Listing const & rhs) const;
    bool empty(void) const { return statements_.empty(); };

  private:
    typename vector_type::iterator remove_const (const_iterator statement);

  private:
    QString name_;
    QString currency_;
    vector_type statements_;
  };

  template<typename COMMITTABLE> std::ostream & operator<<(std::ostream & stream, Listing<COMMITTABLE> const & listing);
  template<typename COMMITTABLE> std::istream & operator>>(std::istream & stream, Listing<COMMITTABLE> & listing);
  template<typename COMMITTABLE> bool operator==(Listing<COMMITTABLE> const & lhs, Listing<COMMITTABLE> const & rhs) { return lhs.equals(rhs); };

  // Definition of the tags
  struct committable_tag
  {
    using statement_type = CommittableStatement;
  };

  struct non_committable_tag
  {
    using statement_type = Statement;
  };
}

#endif
