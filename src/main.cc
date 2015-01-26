#include <gflags/gflags.h>
#include <glog/logging.h>
#include "utils/global_context.h"
#include "utils/common.h"
#include "proto/model.pb.h"
#include "proto/cluster.pb.h"
#include "server.h"
#include "worker.h"

/**
 * \file main.cc is the main entry of SINGA.
 */

DEFINE_string(cluster_conf, "examples/imagenet12/cluster.conf",
    "configuration file for node roles");
DEFINE_string(model_conf, "examples/imagenet12/model.conf",
    "DL model configuration file");

// for debug use
#ifndef FLAGS_v
  DEFINE_int32(v, 3, "vlog controller");
#endif

int main(int argc, char **argv) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  //MPI_Init(&argc, &argv);
  //FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Init GlobalContext
  singa::Cluster cluster;
  singa::ReadProtoFromTextFile(FLAGS_cluster_conf.c_str(), &cluster);
  auto gc=singa::GlobalContext::Get(cluster);
  singa::ModelProto model;
  singa::ReadProtoFromTextFile(FLAGS_model_conf.c_str(), &model);
  LOG(INFO)<<"The cluster config is\n"<<cluster.DebugString()
    <<"\nThe model config is\n"<<model.DebugString();

  singa::TableServer server;
  singa::Worker worker;
  if(gc->AmITableServer()) {
    server.Start(model.solver().sgd());
  }else {
    //singa::Debug();
    worker.Start(model);
  }
  gc->Finalize();
  MPI_Finalize();
  LOG(ERROR)<<"shutdown";
  return 0;
}
