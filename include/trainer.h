#ifndef INCLUDE_TRAINER_H_
#define INCLUDE_TRAINER_H_
#include "proto/cluster.pb.h"
#include "proto/model.pb.h"
namespace singa {
class Trainer{
 public:
  void Start(const ModelProto& modelproto, const ClusterProto& clusterproto,
    int procs_id);

  void Run();
};
} /* singa */
#endif // INCLUDE_TRAINER_H_
