// Copyright © 2014 Wei Wang. All Rights Reserved.
// 2014-07-16 22:00

#ifndef INCLUDE_DISK_DATA_SOURCE_H_
#define INCLUDE_DISK_DATA_SOURCE_H_
#include <string>

#include "model/blob.h"
#include "proto/lapis.pb.h"


namespace lapis {
class DataSource {
 public:
  explicit DataSource(const DataSourceProto &ds_proto);
  void ToProto(DataSourceProto *ds_proto);
  /**
   * Put one batch is data into blob
   * @param blob where the next batch of data will be put
   */
  void GetData(Blob *blob);
  const int Batchsize() {
    return batchsize_;
  }
  const int Size() {
    return size_;
  }
  const int Channels() {
    return channels_;
  }
  const int Height() {
    return height_;
  }
  const int Width() {
    return width_;
  }
  const std::string &Name() {
    return name_;
  }

  ~DataSource();
 private:
  //! current batch id
  // int batchid_;
  //! num of instances per batch
  // int batchsize_;
  //! total number of instances/images
  int size_;
  //! the path of the data source file
  std::string path_;
  //! identifier of the data source
  std::string name_;
  //! properties for rgs image feature
  int channels_, height_, width_;
  //! data source type
  // DataSourceProto_DataType type_;
  //! record reader
  RecordReader *reader_;
};

}  // namespace lapis

#endif  // INCLUDE_DISK_DATA_SOURCE_H_
