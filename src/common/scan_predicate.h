// Copyright (c) 2013, Cloudera, inc.
// All rights reserved.
#ifndef KUDU_COMMON_SCAN_PREDICATE_H
#define KUDU_COMMON_SCAN_PREDICATE_H

#include <boost/optional.hpp>
#include <string>

#include "common/rowblock.h"
#include "util/bitmap.h"
#include "util/faststring.h"

namespace kudu {

using std::string;


class ValueRange {
 public:
  // Construct a new column range predicate.
  //
  // The min_value and upper_bound pointers should point to storage
  // which represents a constant cell value to be used as a range.
  // The range is inclusive on both ends.
  // The cells are not copied by this object, so should remain unchanged
  // for the lifetime of this object.
  //
  // If either optional is unspecified (ie boost::none), then the range is
  // open on that end.
  //
  // A range must be bounded on at least one end.
  ValueRange(const TypeInfo &type,
             boost::optional<const void *> lower_bound,
             boost::optional<const void *> upper_bound);

  bool has_lower_bound() const {
    return lower_bound_.is_initialized();
  }

  bool has_upper_bound() const {
    return upper_bound_.is_initialized();
  }

  const void *lower_bound() const {
    return lower_bound_.get();
  }

  const void *upper_bound() const {
    return upper_bound_.get();
  }

  bool ContainsCell(const void *cell) const;

 private:
  const TypeInfo *type_info_;
  boost::optional<const void *> lower_bound_;
  boost::optional<const void *> upper_bound_;
};


class EncodedKey {
 public:
  EncodedKey(const Schema &schema,
             const void *raw_key);

  const void *raw_key() const { return raw_key_; }

  const Slice encoded_key() const { return Slice(encoded_key_); }

  const faststring &as_faststring() const { return encoded_key_; }

 private:
  const void *raw_key_;
  faststring encoded_key_;
};


// Specifies upper and lower bound using encoded keys

class EncodedKeyRange {
 public:

  EncodedKeyRange(EncodedKey *lower_bound,
                  EncodedKey *upper_bound);

  const EncodedKey &lower_bound() const {
    return *lower_bound_;
  }

  const EncodedKey &upper_bound() const {
    return *upper_bound_;
  }

  bool has_lower_bound() const {
    return lower_bound_.get() != NULL;
  }

  bool has_upper_bound() const {
    return upper_bound_.get() != NULL;
  }

  bool ContainsKey(const Slice &key) const;

 private:
  gscoped_ptr<EncodedKey> lower_bound_;
  gscoped_ptr<EncodedKey> upper_bound_;
};


// Predicate which evaluates to true when the value for a given column
// is within a specified range.
//
// TODO: extract an interface for this once it's clearer what the interface should
// look like. Column range is not the only predicate in the world.
class ColumnRangePredicate {
 public:

  // Construct a new column range predicate.
  // The lower_bound and upper_bound pointers should point to storage
  // which represents a constant cell value to be used as a range.
  // The range is inclusive on both ends.
  // If either optional is unspecified (ie boost::none), then the range is
  // open on that end.
  ColumnRangePredicate(const ColumnSchema &col,
                       boost::optional<const void *> lower_bound,
                       boost::optional<const void *> upper_bound);


  // Evaluate the predicate on every row in the rowblock.
  //
  // This is evaluated as an 'AND' with the current contents of *sel:
  // - wherever the predicate evaluates false, set the appropriate bit in the selection
  //   vector to 0.
  // - If the predicate evalutes true, does not make any change to the
  //   selection vector.
  //
  // On any rows where the current value of *sel is false, the predicate evaluation
  // may be skipped.
  //
  // NOTE: the evaluation result is stored into '*sel' which may or may not be the
  // same vector as block->selection_vector().
  void Evaluate(RowBlock *block, SelectionVector *sel) const;

  const ColumnSchema &column() const {
    return col_;
  }

  string ToString() const;

  // Return the value range for which this predicate passes.
  const ValueRange &range() const { return range_; }

 private:
  ColumnSchema col_;
  ValueRange range_;
};

}
#endif
