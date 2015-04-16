#ifndef INCLUDE_UTILS_CLUSTER_H_
#define INCLUDE_UTILS_CLUSTER_H_
#include <glog/logging.h>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include "proto/cluster.pb.h"

using std::shared_ptr;
using std::string;
using std::vector;

namespace singa {

/**
 * Cluster is a singleton object, which provides cluster configuations,
 * e.g., the topology of the cluster.
 * All IDs start from 0.
 */
class Cluster {
 public:
  static shared_ptr<Cluster> Get();
  static shared_ptr<Cluster> Get(const ClusterProto& cluster, int procs_id);

  /**
   * @return total num of server procs
   */
  const int nservers()const{
    return cluster_.nservers();
  }
  /**
   * @return total num of worker procs
   */
  const int nworkers()const {
    return cluster_.nworkers();
  }
  int nworkers_per_group()const {return cluster_.nworkers_per_group();}
  int nservers_per_group()const {return cluster_.nservers_per_group();}
  int nthreads_per_worker()const{return cluster_.nthreads_per_worker();}
  int nthreads_per_server()const{return cluster_.nthreads_per_server();}

  /**
   * @return true if the calling procs is a server procs, otherwise false
   */
  bool AmIServer()const {
    return procs_id_>=nworkers()&&procs_id_<nworkers()+nservers();
  }
  /**
   * @return true if the calling procs is a worker procs, otherwise false
   */
  bool AmIWorker()const {
    return procs_id_>=0&&procs_id_<nworkers();
  }
  /**
   * @return global procs id, which starts from 0.
   */
  int procs_id()const {return procs_id_;}
  /**
   * @return procs id within a (worker or server) group (starts from 0).
   */
  int group_procs_id() const {
    if(AmIServer())
      return (procs_id_-nworkers())%nservers_per_group();
    else
      return (procs_id_)%nworkers_per_group();
  }
  /**
   * @return (worker or server) group id
   */
  int group_id() const {
     if(AmIServer())
      return (procs_id_-nworkers())/nservers_per_group();
    else
      return (procs_id_)/nworkers_per_group();
  }
  /**
   * @return num of worker groups.
   */
  int nworker_groups() const{return cluster_.nworkers()/nworkers_per_group();}
  /**
   * @return num of worker groups.
   */
  int nserver_groups() const{return cluster_.nservers()/nservers_per_group();}
  /**
   * Return the id of the worker thread within his group.
  int group_threadid(int local_threadid)const{
    return group_procsid()*nthreads_per_procs()+local_threadid;
  }
   */
  /**
   * @return host name
   */
  const string host_addr() const {
    return hosts_.at(procs_id_);
  }
  /**
   * @return host name of a procs with the specified id
   */
  const string host_addr(int procs_id) const {
    CHECK_LT(procs_id, nworkers()+nservers());
    CHECK_GE(procs_id, 0);
    return hosts_.at(procs_id);
  }
  /**
   * @return the host name of a server procs with specified server group id
   * and procs id within that group
   */
  const string server_addr(int group_id, int group_procs_id) const{
    CHECK_GE(group_procs_id,0);
    CHECK_LT(group_procs_id, nservers_per_group());
    CHECK_GE(group_id, 0);
    CHECK_LT(group_id, nserver_groups());
    return hosts_.at(nworkers()+group_id*nservers_per_group()+group_procs_id);
  }
  /*
  const string group_thread_addr(int group_threadid) const{
    CHECK_GE(group_threadid,0);
    CHECK_LT(group_threadid,nthreads_per_group());
    return addr_.at(global_procsid_+group_procsid(group_threadid));
  }

  const string pub_port() const {
    return std::to_string(cluster_.start_port());
  }

  const int router_port() const {
    return cluster_.start_port()+1;
  }
  */
  /**
   * pull port of Bridge layers.
  const string pull_port(int k) const {
    return std::to_string(cluster_.start_port()+2+k);
  }
   */
  //bool synchronous()const {return cluster_.synchronous();}
  const string workspace() {return cluster_.workspace();}
  const string vis_folder(){
    return cluster_.workspace()+"/visualization";
  }

  /**
   * bandwidth MB/s
  float bandwidth() const {
    return cluster_.bandwidth();
  }
   */

 private:
  Cluster(const ClusterProto &cluster, int procs_id) ;
  void SetupFolders(const ClusterProto &cluster);

 private:
  int procs_id_;
  std::vector<std::string> hosts_;
  // cluster config proto
  ClusterProto cluster_;
  // make this class a singlton
  static shared_ptr<Cluster> instance_;
};
}  // namespace singa

#endif  // INCLUDE_UTILS_CLUSTER_H_
