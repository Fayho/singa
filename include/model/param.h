// Copyright © 2014 Wei Wang. All Rights Reserved.
// 2014-07-03 17:35

#ifndef INCLUDE_MODEL_PARAM_H_
#define INCLUDE_MODEL_PARAM_H_

#include <vector>
#include <string>
#include <map>
#include <functional>
#include "proto/lapis.pb.h"
#include "model/blob.h"

// Base paramter class.
// TODO(Jingyang) define split/partition function.
namespace lapis {
class Param {
 public:
  /**
   * Set properties of this parameter from ParamProto, allocate
   * corresponding memory and initialize the parameter
   */
  virtual void Init(const ParamProto &param_proto);
  //! Marshal properties of this parameter into google protobuf
  virtual void ToProto(ParamProto *param_proto);
  //! Return data pointer for this parameter
  const float *Content() const {
    return content_.content();
  }
  float *MutableContent() const {
    return content_.mutable_content();
  }

  const float *Gradient() const {
    return grad_.content();
  }

  float *MutableGradient() const {
    return grad_.mutable_content();
  }
  //! Return num of rows for matrix parameters
  const int Rows() {
    return content_.height();
  }
  //! Return num of columns for matrix parameters
  const int Cols() {
    return content_.width();
  }
  //! Return num of floats for vector parameters
  const int Length() {
    return content_.Length();
  }

 protected:
  Blob content_, grad_, history_grad_;
  float learning_rate_, weight_decay_;
  std::string initializer_;
  std::string name_;  //!< name of the parameter, e.g., 'weight', 'bias'
};


/**
 * macro for register parameter init functions
 * @param TYPE the identifier of this init function
 * @param FUNC  the init function
 */
#define REGISTER_PARAM_INIT_FUNC(ID, FUNC) \
  ParamInitFactory::Instance()->RegisterInitFunc(ID, FUNC)
/**
 * Parameter initialization function factory.
 * It registers the user defined parameter initialization functions at runtime.
 * It also return this function when the function identifier is provided
 */
class ParamInitFactory {
 public:
  static ParamInitFactory *Instance();
  /**
   * Register the init function.
   * This method is called by the register macro REGISTER_PARAM_INIT_FUNC
   * @param id identifier the function, e.g, "Gaussian", i.e., the initializer
   * field in ParamProto
   * @param func std::function object
   */
  void RegisterInitFunc(std::string id,
                        const std::function<void(Param *)> &func);
  std::function<void(Param *)> &Get(std::string id);
 private:
  ParamInitFactory() {}
  std::map<std::string, std::function<void(Param *)>> map_;
};
}  // namespace lapis

#endif  // INCLUDE_MODEL_PARAM_H_
