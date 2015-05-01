#include <thread>
#include <vector>
#include <map>
#include <glog/logging.h>
#include "trainer.h"
#include "worker/worker.h"
#include "server/server.h"
using std::vector;
using std::map;

namespace singa {

void Trainer::Start(const ModelProto& modelproto,
    const ClusterProto& clusterproto,
    int procs_id){
  auto cluster=singa::Cluster::Get(clusterproto, procs_id);
  vector<shared_ptr<Server>> servers;
  int nSocket=1; // the first socket is the router
  if(cluster->has_server()){
    int pid=cluster->procs_id()-cluster->nworker_procs();
    int gid=pid*cluster->nservers_per_procs()/cluster->nservers_per_group();
    int start=pid*cluster->nservers_per_procs()%cluster->nservers_per_group();
    int end=(pid+1)*cluster->nservers_per_procs()%cluster->nservers_per_group();
    auto shard=make_shared<ParamShard>();
    for(int sid=start;sid<end;sid++){
      auto server=make_shared<Server>(gid, sid);
      auto dealer=make_shared<Dealer>(nSocket++);
      dealer->Connect("inproc://router");
      server->Setup(modelproto.updater(), shard, dealer);
      servers.push_back(server);
    }
  }

  vector<shared_ptr<Worker>> workers;
  if(cluster->has_worker()){
    auto net=NeuralNet::SetupNeuralNet(modelproto.neuralnet(), kTrain);
    shared_ptr<SharedParamShard> shard=nullptr;
    // TODO(wangwei) setup shard
    int pid=cluster->procs_id();
    int gstart, gend, wstart, wend;

    if(cluster->nworkers_per_group()>cluster->nworkers_per_procs()){
      gstart=pid*cluster->nworkers_per_procs()/cluster->nworkers_per_group();
      gend=gstart+1;
      wstart=pid*cluster->nworkers_per_procs()%cluster->nworkers_per_group();
      wend=(pid+1)*cluster->nworkers_per_procs()%cluster->nworkers_per_group();
    }else{
      CHECK_EQ(cluster->nworkers_per_procs()%cluster->nworkers_per_group(),0);
      int groups_per_procs=
        cluster->nworkers_per_procs()/cluster->nworkers_per_group();
      gstart=pid*groups_per_procs;
      gend=(pid+1)*groups_per_procs;
      wstart=0;
      wend=cluster->nworkers_per_group();
    }
    for(int gid=gstart;gid<gend;gid++){
      shared_ptr<NeuralNet> train_net, test_net, validation_net;
      if(gid==gstart)
        train_net=net;
      else{
        train_net=NeuralNet::SetupNeuralNet(modelproto.neuralnet(), kTrain);
        if(modelproto.updater().hogwild())
          train_net->ShareParams(net, kValueOnly);
      }
      if(gid==0){
        if(modelproto.test_steps()){
          auto test_net=NeuralNet::SetupNeuralNet(modelproto.neuralnet(), kTest);
          if(test_net!=nullptr)
            test_net->ShareParams(train_net, kWhole);
        }
        if(modelproto.validation_steps()){
          validation_net=NeuralNet::SetupNeuralNet(modelproto.neuralnet(), kValidation);
          if(validation_net!=nullptr)
            validation_net->ShareParams(train_net, kWhole);
        }
      }
      for(int wid=wstart;wid<wend;wid++){
        shared_ptr<Worker> worker=nullptr;
        if(modelproto.alg()==ModelProto_GradCalcAlg_kBackPropagation)
          worker=make_shared<BPWorker>(gid, wid);
        else{
        // TODO add CDWorker
        }
        auto layer_dealer=make_shared<Dealer>(nSocket++);
        auto param_dealer=make_shared<Dealer>(nSocket++);
        layer_dealer->Connect("inproc://router");
        param_dealer->Connect("inproc://router");
        worker->Setup(modelproto, train_net, shard, layer_dealer, param_dealer);
        worker->set_test_net(test_net);
        worker->set_validation_net(validation_net);
        workers.push_back(worker);
      }
    }
  }

#ifdef USE_MPI
  for(int i=0;i<nSocket;i++){
    MPIQueues.push_back(make_shared<SafeQueue>());
  }
#endif
  vector<std::thread> threads;
  for(auto server: servers)
    threads.push_back(std::thread(&Server::Run,server));
  for(auto worker: workers)
    threads.push_back(std::thread(&Worker::Run,worker));
  Run();
  for(auto& thread: threads)
    thread.join();
}
int ProcsIDOf(int group_id, int id, int flag){
  int procsid;
  auto cluster=Cluster::Get();
  if(flag==kServer){
      int nprocs_per_server_group=
        cluster->nservers_per_group()/cluster->nservers_per_procs();
      procsid=group_id*nprocs_per_server_group+id/cluster->nservers_per_procs();
      if(cluster->server_worker_separate())
        procsid+=cluster->nworker_procs();
  }else if(flag==kWorkerLayer || flag==kWorkerParam){
    procsid=group_id*cluster->nworkers_per_group()/cluster->nworkers_per_procs();
  }else{
    LOG(ERROR)<<"Unkown flag ("<<flag<<")";
  }
  return procsid;
}

void Trainer::Run(){
  auto router=shared_ptr<Router>();
  auto cluster=Cluster::Get();
  router->Bind(cluster->endpoint());

  map<int, shared_ptr<Dealer>> inter_dealers;
  while(true){
    Msg* msg=router->Receive();
    int dst_flag=msg->dst_flag();
    int type=msg->type();
    int group_id, id, procs_id;
    switch (dst_flag){
      case kStub:
        if(type==kConnect){
          delete msg;
        }else{
          LOG(ERROR)<<"Unkown message type ("<<type<<") to stub";
        }
        break;
      default:
        group_id=msg->dst_group_id();
        id=msg->dst_id();
        procs_id=ProcsIDOf(group_id, id, dst_flag);
        if(procs_id!=cluster->procs_id()){
          if (inter_dealers.find(procs_id)==inter_dealers.end())
            inter_dealers[procs_id]=make_shared<Dealer>(procs_id);
          inter_dealers[procs_id]->Send(msg);
        }
        else
          router->Send(msg);
        break;
    }
  }
}
} /* singa */
