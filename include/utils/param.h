#ifndef INCLUDE_MODEL_PARAM_H_
#define INCLUDE_MODEL_PARAM_H_
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <czmq.h>
#include "proto/model.pb.h"
#include "utils/blob.h"
// Base paramter class.
namespace singa {
enum kMsgType{
kGet=0,
kPut=1,
kSync=2,
kUpdate=3,
kSyncRequest=4,
kSyncResponse=5,
kStop=6,
kData=7
};
class Param {
 public:
   Param();
   virtual ~Param();

  //Anh's stuff
   //return only <content>
   virtual zmsg_t* ParseToMsg(); /** Parse Param's content to zmsg_t message */

   //msg of the format <kData><paramId><content>
   virtual void ParseToParam(zmsg_t **msg);

   //End of Anh's stuff


  /**
   * handle put msg by server.
   * Return NULL if successful
   */
  virtual zmsg_t* HandlePutMsg(zmsg_t** msg);

  /**
   * handle update msg
   * return NULL if unsuccesful.
   * return the parameter message
   */
  virtual zmsg_t* HandleUpdateMsg(zmsg_t **msg);

  /**
   * handle get msg by server
   */
  virtual zmsg_t* HandleGetMsg(zmsg_t** msg);
  /**
   * handle sync msg by server
   */
  virtual zmsg_t* HandleSyncMsg(zmsg_t** msg);
  /**
   * gen sync msg by worker
   */
  zmsg_t *GenSyncMsgFromWorker(float sample_ratio){return NULL;}
  /**
   * parse sync msg by worker
   */
  void ParseSyncMsgFromPS(zmsg_t** msg){}

  /**
   * setup param shape
   */
  virtual void Setup(const ParamProto& proto, const std::vector<int>& shape, int fan_in);
  /*
   * fill the data according to initmethod, i.e., random/gaussian/fixed value
   */
  virtual void Init();
  /**
   * if the Param shares data with others, then point to the owner.
   * otherwise points to itself.
   */
  const Param* owner() const{
    return owner_;
  }
  const std::string& name() {
    return proto_.name();
  }

  int id() const{
    return proto_.id();
  }
  void set_id(int id){
    proto_.set_id(id);
  }
  void ShareData(shared_ptr<Param> other){
    owner_=other.get();
    CHECK(std::equal(data_.shape().begin(), data_.shape().end(),
          other->data_.shape().begin()));
    data_.ShareData(other->data_);
  }
  float learning_rate_multiplier() {
    return proto_.learning_rate_multiplier();
  }
  float weight_decay_multiplier() {
    return proto_.weight_decay_multiplier();
  }
  /*
  const int split_threshold(){
    return proto_.split_threshold();
  }
  */
   /**
    * @return num of floats.
    */
  int size() const {
    return data_.count();
  }
  /**
   * Return const mem address for the content of this parameter
   */
  const Blob<float> &data() {
    return data_;
  }
  Blob<float> *mutable_data() {
    return &data_;
  }
  /**
   * Return gradient of this parameter
   */
  const Blob<float> &grad() {
    return grad_;
  }
  Blob<float> *mutable_grad() {
    return &grad_;
  }

  const Blob<float> &history() {
    return history_;
  }
  Blob<float> *mutable_history() {
    return &history_;
  }

  float* mutable_cpu_data(){
    return data_.mutable_cpu_data();
  }
  float* mutable_cpu_grad(){
    return grad_.mutable_cpu_data();
  }
  float* mutable_cpu_history(){
    return history_.mutable_cpu_data();
  }
  float* mutable_cpu_update(){
    return update_.mutable_cpu_data();
  }
 static int64_t ps_handle_sync, worker_gen_sync, worker_handle_sync;
 protected:
  /**
   * name of the parameter used to share wights between neuralnets
   */
  std::string name_;
  //! content, gradient, history gradient and snapshot of this parameter
  Blob<float> data_, grad_, history_, update_, snapshot_;
  Param* owner_;

  ParamProto proto_;
  int fan_in_;

  zmutex_t *param_lock_;
};

/**
 * Sync with server by randomly sampling some parameters for every sync.
 */
class RandomSyncParam: public Param{
 public:
  virtual zmsg_t* HandleSyncMsg(zmsg_t** msg);
  virtual zmsg_t *GenSyncMsgFromWorker(float sample_ratio);
  virtual void ParseSyncMsgFromPS(zmsg_t** msg);
  virtual void Setup(const ParamProto& proto, const vector<int>& shape, int fan_in);
  virtual void Init();

  float* mutable_cpu_snapshot(){
    return snapshot_.mutable_cpu_data();
  }
  const float* cpu_snapshot(){
    return snapshot_.cpu_data();
  }

 protected:
  const vector<int> RandomSample(int seed, int m, int n);


  Blob<float> snapshot_;
};
/**
 * Sync with server by elastic SGD.
 */
class ElasticParam: public Param{
 public:
  virtual zmsg_t* HandleSyncMsg(zmsg_t** msg);
  virtual zmsg_t *GenSyncMsgFromWorker(float moving_rate);
  virtual void ParseSyncMsgFromPS(zmsg_t** msg);
};


}  // namespace singa

#endif  // INCLUDE_MODEL_PARAM_H_
