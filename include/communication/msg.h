#ifndef INCLUDE_COMMUNICATION_MSG_H_
#define INCLUDE_COMMUNICATION_MSG_H_
#include <string>
#include <czmq.h>
#include <glog/logging.h>

using std::string;
namespace singa {
class Msg{
  public:
  /**
    * Destructor to free memory
    */
  virtual ~Msg(){};
  /**
    * @param group_id worker/server group id
    * @param id worker/server id within the group
    * @param flag 0 for server, 1 for worker, 2 for stub
    */
  virtual void set_src(int group_id, int id, int flag)=0;
  virtual void set_dst(int group_id, int id, int flag)=0;
  virtual int src_group_id() const=0;
  virtual int src_id() const=0;
  virtual int dst_group_id() const=0;
  virtual int dst_id() const=0;
  virtual int src_flag() const=0;
  virtual int dst_flag() const=0;
  virtual void set_type(int type)=0;
  virtual int type() const=0;
  virtual void set_target(int target)=0;
  virtual int target() const=0;

  virtual void add_frame(const void*, int nBytes)=0;
  virtual int frame_size()=0;
  virtual void* frame_data()=0;
  /**
    * Move the cursor to the next frame
    * @return true if the next frame is not NULL; otherwise false
    */
  virtual bool next_frame()=0;
};


class ZMQMsg : public Msg{
 public:
  ZMQMsg() {
    msg_=zmsg_new();
  }
  virtual ~ZMQMsg(){
    if(msg_!=NULL)
      zmsg_destroy(&msg_);
  }
  virtual void set_src(int group_id, int id, int flag){
    src_=(group_id<<kOff1)|(id<<kOff2)|flag;
  }
  virtual void set_dst(int group_id, int id, int flag){
    dst_=(group_id<<kOff1)|(id<<kOff2)|flag;
  }
  virtual int src_group_id() const {
    int ret=src_>>kOff1;
    return ret;
  }
  int src() const {
    return src_;
  }
  int dst() const {
    return dst_;
  }
  virtual int src_id() const{
    int ret=(src_&kMask1)>>kOff2;
    return ret;
  }
  virtual int dst_group_id() const{
    int ret=dst_>>kOff1;
    return ret;
  }
  virtual int dst_id() const{
    int ret=(dst_&kMask1)>>kOff2;
    return ret;
  }
  virtual int src_flag() const{
    int ret=src_&kMask2;
    return ret;
  }
  virtual int dst_flag() const{
    int ret=dst_&kMask2;
    return ret;
  }
  virtual void set_type(int type){
    target_=(type<<kOff3)|(target_&kMask3);
  }
  virtual void set_target(int target){
    target_=(target_|kMask3)&(target);
  }
  virtual int type() const{
    int ret=target_>>kOff3;
    return ret;
  }
  virtual int target() const{
    int ret=target_&kMask3;
    return ret;
  }
  virtual void add_frame(const void* addr, int nBytes){
    zmsg_addmem(msg_, addr, nBytes);
  }
  virtual int frame_size(){
    return zframe_size(frame_);
  }

  virtual void* frame_data(){
    return zframe_data(frame_);
  }

  virtual bool next_frame(){
    frame_=zmsg_next(msg_);
    return frame_!=NULL;
  }

  void ParseFromZmsg(zmsg_t* msg){
    char* tmp=zmsg_popstr(msg);
    sscanf(tmp, "%d %d %d", &src_, &dst_, &target_);
    //LOG(ERROR)<<"recv "<<src_<<" "<<dst_<<" "<<target_;
    frame_=zmsg_next(msg);
    msg_=msg;
  }

  zmsg_t* DumpToZmsg(){
    zmsg_pushstrf(msg_, "%d %d %d",src_, dst_,target_);
    //LOG(ERROR)<<"send "<<src_<<" "<<dst_<<" "<<target_;
    zmsg_t* tmp=msg_;
    msg_=NULL;
    return tmp;
  }

 protected:
  static const unsigned int kOff1=16, kOff2=4, kOff3=24;
  static const unsigned int kMask1=(1<<kOff1)-1, kMask2=(1<<kOff2)-1,
               kMask3=(1<<kOff3)-1;
  unsigned int src_, dst_, target_;
  zmsg_t* msg_;
  zframe_t *frame_;
};

} /* singa */

#endif // INCLUDE_COMMUNICATION_MSG_H_
