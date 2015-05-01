#include <gflags/gflags.h>
#include <glog/logging.h>
#include "trainer.h"
#include "utils/updater.h"
#include "utils/param.h"
#include "utils/singleton.h"
#include "utils/factory.h"
#include "worker/neuralnet.h"
#include "worker/pm_worker.h"
#include "server/pm_server.h"

/**
 * \file main.cc is the main entry of SINGA.
 */
DEFINE_int32(procsID, 0, "Global process ID");
DEFINE_string(cluster, "examples/imagenet12/cluster.conf",
    "Configuration file for the cluster");
DEFINE_string(model, "examples/imagenet12/model.conf",
    "Deep learning model configuration file");

/**
 * Register layers, and other customizable classes.
 *
 * If users want to use their own implemented classes, they should register
 * them here.
 */
void RegisterClasses(const singa::ModelProto& proto){
  singa::NeuralNet::RegisterLayers();
  Singleton<Factory<singa::Param>>::Instance()->Register(
      "Param", CreateInstance(singa::Param, singa::Param));
  Singleton<Factory<singa::Updater>>::Instance() ->Register(
      "Updater", CreateInstance(singa::SGDUpdater, singa::Updater));
  Singleton<Factory<singa::PMWorker>>::Instance() ->Register(
      "PMWorker", CreateInstance(singa::PMWorker, singa::PMWorker));
  Singleton<Factory<singa::PMServer>>::Instance() ->Register(
      "PMServer", CreateInstance(singa::PMServer, singa::PMServer));
}

#ifndef FLAGS_v
  DEFINE_int32(v, 3, "vlog controller");
#endif

int main(int argc, char **argv) {
  //FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  singa::ClusterProto cluster;
  singa::ReadProtoFromTextFile(FLAGS_cluster.c_str(), &cluster);
  singa::ModelProto model;
  singa::ReadProtoFromTextFile(FLAGS_model.c_str(), &model);
  LOG(INFO)<<"The cluster config is\n"<<cluster.DebugString()
    <<"\nThe model config is\n"<<model.DebugString();

  RegisterClasses(model);
  singa::Trainer trainer;
  trainer.Start(model, cluster, FLAGS_procsID);
  return 0;
}
