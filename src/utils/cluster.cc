#include <glog/logging.h>
#include <fcntl.h>
#include <fstream>
#include "utils/cluster.h"
#include "proto/cluster.pb.h"
#include <sys/stat.h>
#include <sys/types.h>
namespace singa {

std::shared_ptr<Cluster> Cluster::instance_;
Cluster::Cluster(const ClusterProto &cluster, int procs_id) {
  procs_id_=procs_id;
	cluster_ = cluster;
  auto nhosts=cluster_.nservers()+cluster_.nworkers();
  CHECK_LT(procs_id, nhosts);
  SetupFolders(cluster);
  //char hostname[256]; gethostname(hostname, sizeof(hostname));

  if(cluster_.nworkers()>0){
    std::ifstream ifs(cluster.hostfile(), std::ifstream::in);
    std::string line;
    while(std::getline(ifs, line)&&hosts_.size()<nhosts){
      hosts_.push_back(line);
    }
    CHECK_EQ(hosts_.size(), nhosts);
  }
}

void Cluster::SetupFolders(const ClusterProto &cluster){
  // create visulization folder
  mkdir(vis_folder().c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

shared_ptr<Cluster> Cluster::Get(const ClusterProto& cluster, int procs_id){
  instance_.reset(new Cluster(cluster, procs_id));
  return instance_;
}

shared_ptr<Cluster> Cluster::Get() {
  if(!instance_) {
    LOG(ERROR)<<"The first call to Get should "
              <<"provide the sys/model conf path";
  }
  return instance_;
}
}  // namespace singa
