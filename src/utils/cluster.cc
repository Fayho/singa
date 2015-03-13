#include <glog/logging.h>
#include <fcntl.h>
#include <fstream>
#include "utils/cluster.h"
#include "proto/cluster.pb.h"
#include <sys/stat.h>
#include <sys/types.h>
namespace singa {

std::shared_ptr<Cluster> Cluster::instance_;
Cluster::Cluster(const ClusterProto &cluster, string hostfile, int procsid) {
  global_procsid_=procsid;
	cluster_ = cluster;
  SetupFolders(cluster);
  char hostname[256];
  gethostname(hostname, sizeof(hostname));
  hostname_=string(hostname);

  if(cluster_.nworkers()>1&&cluster_.nservers()>0){
    std::ifstream ifs(hostfile, std::ifstream::in);
    std::string line;
    while(std::getline(ifs, line)){
      addr_.push_back(line);
    }
  }
  //CHECK_EQ(addr_.size(), cluster_.nservers()+cluster_.nworkers());
}

void Cluster::SetupFolders(const ClusterProto &cluster){
  // create visulization folder
  mkdir(visualization_folder().c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void Cluster::SetupGroups(const ClusterProto &cluster){
}

shared_ptr<Cluster> Cluster::Get(const ClusterProto& cluster, string hostfile,
    int procsid){
  if(!instance_) {
    instance_.reset(new Cluster(cluster, hostfile, procsid));
  }
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
