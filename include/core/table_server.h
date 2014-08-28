//  Copyright © 2014 Anh Dinh. All Rights Reserved.

//  handle operations of the memory server.
//  similar to Worker in Piccolo
//  its methods are registered as callbacks to the NetworkThread

#ifndef INCLUDE_CORE_MEMORY_SERVER_H_
#define INCLUDE_CORE_MEMORY_SERVER_H_

#include "core/common.h"
#include "core/rpc.h"
#include "core/table.h"
#include "core/global-table.h"
#include "core/local-table.h"
#include "proto/worker.pb.h"
#include "utils/network_thread.h"

namespace lapis {

class TableServer : private boost::noncopyable {
 public:
	TableServer();

  ~TableServer() {}

  void StartTableServer(const std::map<int, GlobalTable*> &tables);

  //  sends signals to the manager and ends gracefully
  void ShutdownTableServer(){ done_writing_ = true; }

  int id() {
    return server_id_;
  }

  //  update ownership of the partition. Only memory server
  //  storing the data will received this
  //  assignment happens only once at the beginning
  void HandleShardAssignment();

  //  read DiskData and dump to file
  void HandleDataPut();

  //  get notified from the coordinator that there's no more data
  void FinishDataPut();

  //  shutdown gracefully
  void HandleServerShutdown();

  void HandleUpdateRequest(const Message *message);
  void HandleGetRequest(const Message *message);

  //  id of the peer responsible for storing the partition
  int peer_for_partition(int table, int shard);

 private:

  int server_id_;
  mutable boost::recursive_mutex state_lock_;
  std::shared_ptr<NetworkThread> net_;
  std::map<int, GlobalTable*> tables_;
  boost::thread *disk_write_thread_;
  bool done_writing_, has_finalized_;

  void write_to_disk_loop();
  void data_put(const DiskData& data);

};

//  start memory server, only if rank < size()-1
// bool StartTableServer();

}  //  namespace lapis

#endif //  INCLUDE_CORE_MEMORY_SERVER_H_
