#ifndef __BatchFillerPIDLabel_CXX__
#define __BatchFillerPIDLabel_CXX__

#include "BatchFillerPIDLabel.h"
#include "larcv/core/DataFormat/EventImage2D.h"
#include "larcv/core/DataFormat/EventParticle.h"
#include <random>

namespace larcv {

  static BatchFillerPIDLabelProcessFactory __global_BatchFillerPIDLabelProcessFactory__;

  BatchFillerPIDLabel::BatchFillerPIDLabel(const std::string name)
    : BatchFillerTemplate<float>(name)
  {}

  void BatchFillerPIDLabel::configure(const PSet& cfg)
  {
    _part_producer = cfg.get<std::string>("ParticleProducer");

    _pdg_list = cfg.get<std::vector<int> >("PdgClassList");
    if (_pdg_list.empty()) {
      LARCV_CRITICAL() << "PdgClassList needed to define classes!" << std::endl;
      throw larbys();
    }
    _num_class = _pdg_list.size();
  }

  void BatchFillerPIDLabel::initialize()
  {}

  void BatchFillerPIDLabel::_batch_begin_()
  {}

  void BatchFillerPIDLabel::_batch_end_()
  {
    if (logger().level() <= msg::kINFO) {
      LARCV_INFO() << "Total data size: " << batch_data().data_size() << std::endl;

      std::vector<size_t> ctr_v;
      for (auto const& v : batch_data().data()) {
        if (v >= ctr_v.size()) ctr_v.resize(v + 1, 0);
        ctr_v[v] += 1;
      }
      std::stringstream ss;
      ss << "Used: ";
      for (size_t i = 0; i < ctr_v.size(); ++i)
        ss << ctr_v[i] << " of class " << i << " ... ";
      LARCV_INFO() << ss.str() << std::endl;
    }
  }

  void BatchFillerPIDLabel::finalize()
  {}

  bool BatchFillerPIDLabel::process(IOManager& mgr)
  {
    auto const& event_part = mgr.get_data<larcv::EventParticle>(_part_producer);

    if (batch_data().dim().empty()) {
      std::vector<int> dim(2);
      dim[0] = batch_size();
      dim[1] = _num_class;
      set_dim(dim);
    }

    // labels
    auto const& part_v = event_part.as_vector();
    if (part_v.size() != 1) {
      LARCV_CRITICAL() << "Only support single particle label now: EventParticle size != 1" << std::endl;
      throw larbys();
    }
    // class
    size_t label = kINVALID_SIZE;
    for(auto const& part : part_v) {
      for(size_t class_idx=0; class_idx < _pdg_list.size(); ++class_idx) {
        if(part.pdg_code() != _pdg_list[class_idx]) continue;
        label = class_idx;
        break;
      }
      if(label!=kINVALID_SIZE) break;
    }
    _entry_data.resize(_num_class, 0);
    for (auto& v : _entry_data) v = 0;
    _entry_data.at(label) = 1.;
    set_entry_data(_entry_data);

    return true;
  }

}
#endif
