#include "communication/socket.h"

namespace singa {
ZMQPoller::ZMQPoller(){
  poller_=zpoller_new(NULL);
}

void ZMQPoller::Add(Socket* socket){
  zsock_t* zsock=static_cast<zsock_t*>(socket->InternalID());
  zpoller_add(poller_, zsock);
  zsock2Socket_[zsock]=socket;
}

Socket* ZMQPoller::Poll(int timeout){
  zsock_t* sock=(zsock_t*)zpoller_wait(poller_, timeout);
  if(sock!=NULL)
    return zsock2Socket_[sock];
  else return nullptr;
}

int ZMQDealer::Connect(string endpoint){
  dealer_=zsock_new_dealer(endpoint.c_str());
  CHECK_NOTNULL(dealer_);
  /*
  zmsg_t* msg=zmsg_new();
  zmsg_pushstr(msg, "PING");
  zmsg_send(&msg, dealer_);
  msg=zmsg_recv(dealer_);
  char *tmp=zmsg_popstr(msg);
  CHECK_STREQ(tmp, "PONG");
  zmsg_destroy(&msg);
  */
  return 1;
}
int ZMQDealer::Send(Msg *msg, int dst){
  zmsg_t* zmsg=(static_cast<ZMQMsg*>(msg))->DumpToZmsg();
  zmsg_send(&zmsg, dealer_);
  return 1;
}

Msg* ZMQDealer::Receive(int src){
  zmsg_t* zmsg=zmsg_recv(dealer_);
  ZMQMsg* msg=new ZMQMsg();
  msg->ParseFromZmsg(zmsg);
  return msg;
}
ZMQDealer::~ZMQDealer(){
  zsock_destroy(&dealer_);
}
int ZMQRouter::Bind(string endpoint, int expected_connections){
  router_=zsock_new_router("inproc://router");
  CHECK_NOTNULL(router_);
  if(endpoint.length())
    zsock_bind(router_, endpoint.c_str());
  return 1;
}

int ZMQRouter::Send(Msg *msg, int dst){
  zmsg_t* zmsg=static_cast<ZMQMsg*>(msg)->DumpToZmsg();
  int dstid=static_cast<ZMQMsg*>(msg)->dst();
  if(id2addr_.find(dstid)!=id2addr_.end()){
    zframe_t* addr=zframe_dup(id2addr_[dstid]);
    zmsg_prepend(zmsg, &addr);
    zmsg_send(&zmsg, router_);
  }else{
    bufmsg_[dstid].push_back(zmsg);
  }
  return 1;
}

Msg* ZMQRouter::Receive(int src){
  zmsg_t* zmsg=zmsg_recv(router_);
  zframe_t* dealer=zmsg_pop(zmsg);
  ZMQMsg* msg=new ZMQMsg();
  msg->ParseFromZmsg(zmsg);
  if (id2addr_.find(msg->src())==id2addr_.end()){
    id2addr_[msg->src()]=dealer;
    if(bufmsg_.find(msg->src())!=bufmsg_.end()){
      for(auto& it: bufmsg_.at(msg->src())){
        zframe_t* addr=zframe_dup(dealer);
        zmsg_prepend(it, &addr);
        zmsg_send(&it, router_);
      }
      bufmsg_.erase(msg->src());
    }
  }
  else
    zframe_destroy(&dealer);
  return msg;
}

ZMQRouter::~ZMQRouter(){
  zsock_destroy(&router_);
  for(auto it: id2addr_)
    zframe_destroy(&it.second);
  for(auto it: bufmsg_){
    for(auto *msg: it.second)
      zmsg_destroy(&msg);
  }
}
} /* singa */
