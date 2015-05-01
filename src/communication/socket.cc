#include "communication/socket.h"

namespace singa {
Poller::Poller(){
  poller_=zpoller_new(NULL);
}

void Poller::Add(Socket* socket){
  zsock_t* zsock=static_cast<zsock_t*>(socket->InternalID());
  zpoller_add(poller_, zsock);
  zsock2Socket_[zsock]=socket;
}

Socket* Poller::Poll(int timeout){
  zsock_t* sock=(zsock_t*)zpoller_wait(poller_, timeout);
  if(sock!=NULL)
    return zsock2Socket_[sock];
  else return nullptr;
}

int Dealer::Connect(string endpoint){
  dealer_=zsock_new_dealer(endpoint.c_str());
  CHECK_NOTNULL(dealer_);
  return 1;
}
int Dealer::Send(Msg *msg){
  zmsg_t* zmsg=(static_cast<Msg*>(msg))->DumpToZmsg();
  zmsg_send(&zmsg, dealer_);
  return 1;
}

Msg* Dealer::Receive(){
  zmsg_t* zmsg=zmsg_recv(dealer_);
  Msg* msg=new Msg();
  msg->ParseFromZmsg(zmsg);
  return msg;
}
Dealer::~Dealer(){
  zsock_destroy(&dealer_);
}
int Router::Bind(string endpoint){
  router_=zsock_new_router("inproc://router");
  CHECK_NOTNULL(router_);
  if(endpoint.length())
    zsock_bind(router_, endpoint.c_str());
  return 1;
}

int Router::Send(Msg *msg){
  zmsg_t* zmsg=static_cast<Msg*>(msg)->DumpToZmsg();
  int dstid=static_cast<Msg*>(msg)->dst();
  if(id2addr_.find(dstid)!=id2addr_.end()){
    zframe_t* addr=zframe_dup(id2addr_[dstid]);
    zmsg_prepend(zmsg, &addr);
    zmsg_send(&zmsg, router_);
  }else{
    if(bufmsg_.size()==0)
      nBufmsg_=0;
    bufmsg_[dstid].push_back(zmsg);
    nBufmsg_++;
    CHECK_LE(nBufmsg_, bufsize_);
  }
  return 1;
}

Msg* Router::Receive(){
  zmsg_t* zmsg=zmsg_recv(router_);
  zframe_t* dealer=zmsg_pop(zmsg);
  Msg* msg=new Msg();
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

Router::~Router(){
  zsock_destroy(&router_);
  for(auto it: id2addr_)
    zframe_destroy(&it.second);
  for(auto it: bufmsg_){
    for(auto *msg: it.second)
      zmsg_destroy(&msg);
  }
}
} /* singa */
