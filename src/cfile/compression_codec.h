// Copyright (c) 2013, Cloudera, inc.
#ifndef KUDU_CFILE_COMPRESSION_CODEC_H
#define KUDU_CFILE_COMPRESSION_CODEC_H

#include <string>
#include <tr1/memory>
#include <vector>

#include "cfile/cfile.pb.h"
#include "gutil/macros.h"
#include "util/slice.h"
#include "util/status.h"

using std::tr1::shared_ptr;

namespace kudu {
namespace cfile {

class CompressionCodec {
 public:
  CompressionCodec();
  virtual ~CompressionCodec();

  // REQUIRES: "compressed" must point to an area of memory that is at
  // least "MaxCompressedLength(input_length)" bytes in length.
  //
  // Takes the data stored in "input[0..input_length]" and stores
  // it in the array pointed to by "compressed".
  //
  // returns the length of the compressed output.
  virtual Status Compress(const Slice& input,
                          uint8_t *compressed, size_t *compressed_length) = 0;

  virtual Status Compress(const std::vector<Slice>& input_slices,
                          uint8_t *compressed, size_t *compressed_length) = 0;

  // Given data in "compressed[0..compressed_length-1]" generated by
  // calling the Compress routine, this routine stores the uncompressed data
  // to uncompressed[0..uncompressed_length-1]
  // returns false if the message is corrupted and could not be uncompressed
  virtual Status Uncompress(const Slice& compressed,
                            uint8_t *uncompressed, size_t uncompressed_length) = 0;

  // Returns the maximal size of the compressed representation of
  // input data that is "source_bytes" bytes in length.
  virtual size_t MaxCompressedLength(size_t source_bytes) = 0;
 private:
  DISALLOW_COPY_AND_ASSIGN(CompressionCodec);
};

// Returns the compression codec for the specified type
Status GetCompressionCodec(CompressionType compression,
                           shared_ptr<CompressionCodec> *codec);

// Returns the compression codec type given the name
CompressionType GetCompressionCodecType(const std::string& name);

} // namespace cfile
} // namespace kudu
#endif
