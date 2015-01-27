#ifndef INCLUDE_NET_NET_H_
#define INCLUDE_NET_NET_H_

#include <glog/logging.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <stack>
#include "model/param.h"
#include "proto/model.pb.h"
#include "model/layer.h"

namespace singa {
/**
 * The neural network consists of Layers and Edges.
 */
class Net {
 public:
  /**
   * construct the net structure, init layers and sort layer orders
   */
  explicit Net(const NetProto &net_proto);
  /**
   * desctruct the net.
   * free layer objects.
   */
  ~Net();
  /**
   * construct a string for describing the layers and parameters, including
   * shape info.
   */
  std::string ToString();

  /**
   * construct a DOT string for drawing network structure
  std::string ToDOT();
   */

  /**
   * setup the layers and parameters.
   * shapes of layer data and parameters are infered from input records/shapes.
   * DAry partition is set according to PartitionMode.
   * parameters are initialized.
   * Memory of DAry is allocated when first time used.
   *
   * @input_shapes shapes for the input layers
   * @mode partition mode, namely, kHybrid, kModel, kData, kNone
   */
  void Setup(const vector<vector<int>>& input_shapes, PartitionMode mode=kNone);
  /**
   * @batchsize mini-batch size
   * @record input record to the net, used to set the shapes of input layers
   * @mode partition mode, namely, kHybrid, kModel, kData, kNone
   */
  void Setup(int batchsize, const Record &record,PartitionMode mode=kNone);

  /**
   * serialize the net.
   */
  void ToProto(NetProto *net_proto, bool copyData=false);
  PerformanceLayer* performance_layer(int k) {
    CHECK_LT(k, performance_layers_.size());
    return performance_layers_[k];
  }
  const std::vector<InputLayer *> &input_layer() {
    return input_layers_;
  }
  InputLayer * input_layer(int k) {
    CHECK_LT(k, input_layers_.size());
    return input_layers_[k];
  }
    const std::vector<Layer *>& layers() {
    return layers_;
  }
  const std::vector<Param *> &params() {
    return params_;
  }
  Layer* name2layer(string name){
    if (name2layer_.find(name)!=name2layer_.end())
      return name2layer_[name];
    else return NULL;
  }
  const vector<Layer*> name2srclayers(string name){
     if (name2srclayers_.find(name)!=name2srclayers_.end())
      return name2srclayers_[name];
    else return vector<Layer*>{};
  }
  const vector<Layer*> name2dstlayers(string name){
    if (name2dstlayers_.find(name)!=name2dstlayers_.end())
      return name2dstlayers_[name];
    else return vector<Layer*>{};
  }

 protected:
  // called internally to setup the net.
  void Setup(PartitionMode mode);
  // SortLayersForBP
  void topology_sort(vector<Layer *> *layers,
                     const map<string, vector<Layer*>>& name2dstlayers);
  void topology_sort_inner(Layer *layer,
                         const std::map<Layer *,
                         std::vector<Layer *>> &adjacent_list,
                         std::map<Layer *, bool> *visited,
                         std::stack<Layer *> *stack) ;

  // TODO SortLayersForCD
 private:
  std::vector<Layer *> layers_;
  std::vector<PerformanceLayer *> performance_layers_;
  std::vector<InputLayer *> input_layers_;
  std::vector<Param *> params_;

  std::map<string, Layer*> name2layer_;
  std::map<string, vector<Layer*>> name2srclayers_;;
  std::map<string, vector<Layer*>> name2dstlayers_;;
  // <src layer name, dst layer name>, 'Dummy' for dangling layer
  std::unordered_set<string> edge_set_;
};
}  // namespace singa
#endif  // INCLUDE_NET_NET_H_