/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * - Neither the name of prim nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "search/Engine.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "strop/strop.h"

static const u8 HSE_DEBUG = 0;

CostFunction::CostFunction() {}
CostFunction::~CostFunction() {}

bool Comparator::operator()(const Hyperx& _lhs, const Hyperx& _rhs) const {
  return _rhs.cost > _lhs.cost;
}

Engine::Engine(u64 _min_dimensions, u64 _max_dimensions, u64 _min_radix,
               u64 _max_radix, u64 _min_concentration, u64 _max_concentration,
               u64 _min_terminals, u64 _max_terminals, f64 _min_bandwidth,
               f64 _max_bandwidth, u64 _max_width, u64 _max_weight,
               bool _fixed_width, bool _fixed_weight, u64 _max_results,
               const CostFunction* _cost_function)
    : min_dimensions_(_min_dimensions),
      max_dimensions_(_max_dimensions),
      min_radix_(_min_radix),
      max_radix_(_max_radix),
      min_concentration_(_min_concentration),
      max_concentration_(_max_concentration),
      min_terminals_(_min_terminals),
      max_terminals_(_max_terminals),
      min_bandwidth_(_min_bandwidth),
      max_bandwidth_(_max_bandwidth),
      max_width_(_max_width),
      max_weight_(_max_weight),
      fixed_width_(_fixed_width),
      fixed_weight_(_fixed_weight),
      max_results_(_max_results),
      cost_function_(_cost_function) {
  if (min_dimensions_ < 1) {
    throw std::runtime_error("mindimensions must be greater than 0");
  } else if (max_dimensions_ < min_dimensions_) {
    throw std::runtime_error(
        "maxdimensions must be greater than or equal to "
        "mindimensions");
  } else if (min_radix_ < 2) {
    throw std::runtime_error("minradix must be greater than 1");
  } else if (max_radix_ < min_radix_) {
    throw std::runtime_error(
        "maxradix must be greater than or equal to "
        "minradix");
  } else if (max_concentration_ < min_concentration_) {
    throw std::runtime_error(
        "maxconcentration must be greater than or equal "
        "to minconcentration");
  } else if (min_terminals_ < min_radix_) {
    throw std::runtime_error(
        "minterminals must be greater than or equal to "
        "minradix");
  } else if (max_terminals_ < min_terminals_) {
    throw std::runtime_error(
        "maxterminals must be greater than or equal to "
        "minterminals");
  } else if (min_bandwidth_ <= 0) {
    throw std::runtime_error("minbandwidth must be greater than 0.0");
  } else if (max_bandwidth_ < min_bandwidth_) {
    throw std::runtime_error(
        "maxbandwidth must be greater than or equal to "
        "minbandwidth");
  } else if (max_width_ <= 1) {
    throw std::runtime_error("maxwidth must be greater than 1");
  } else if (max_weight_ < 1) {
    throw std::runtime_error("maxweight must be greater than 0");
  }
}

Engine::~Engine() {}

void Engine::run() {
  hyperx_ = Hyperx();
  results_.clear();

  stage1();
}

const std::deque<Hyperx>& Engine::results() const {
  return results_;
}

void Engine::stage1() {
  /*
   * loop over the number of dimensions
   */
  for (hyperx_.dimensions = min_dimensions_;
       hyperx_.dimensions <= max_dimensions_; hyperx_.dimensions++) {
    // find the maximum width of any one dimension
    u64 max_width;
    if (!fixed_width_) {
      // HyperX
      max_width = max_radix_ - (hyperx_.dimensions - 1);
    } else {
      // FbFly
      max_width = ((max_radix_ - 1) / hyperx_.dimensions) + 1;
    }
    max_width = std::min(max_width, max_width_);

    if (max_width < 2) {
      break;
    }

    if (HSE_DEBUG >= 6) {
      printf("1: dimensions=%lu max_width=%lu\n", hyperx_.dimensions,
             max_width);
    }

    /*
     * generate possible dimension widths (S)
     */
    hyperx_.widths.clear();
    hyperx_.widths.resize(hyperx_.dimensions, 2);
    while (true) {
      // determine the number of routers
      hyperx_.routers = 1;
      u64 base_radix = 1;  // minimum current radix
      for (u64 d = 0; d < hyperx_.widths.size(); d++) {
        hyperx_.routers *= hyperx_.widths.at(d);
        base_radix += hyperx_.widths.at(d) - 1;
      }

      // find reasons to skip this case
      //  expr 1: at minimum, there would be 1 terminal per router
      //  expr 2: check minimum current router radix
      if ((hyperx_.routers <= max_terminals_) && (base_radix <= max_radix_)) {
        // if this configuration appears to work so far, use it
        stage2();
      } else if (HSE_DEBUG >= 7) {
        printf("1s: SKIPPING S=%s\n",
               strop::vecString<u64>(hyperx_.widths).c_str());
      }

      // detect when done
      if (hyperx_.widths.at(0) == max_width) {
        break;
      }

      // find the next widths configuration
      if (!fixed_width_) {
        // HyperX
        u64 ndim = U64_MAX;  // next dimension to increment
        for (u64 invdim = 0; invdim < hyperx_.dimensions; invdim++) {
          ndim = hyperx_.dimensions - invdim - 1;
          if (hyperx_.widths.at(ndim) == max_width) {
            continue;
          } else {
            break;
          }
        }

        // increment this dimension
        hyperx_.widths.at(ndim)++;

        // all inferior dimensions reset
        for (u64 d = ndim + 1; d < hyperx_.dimensions; d++) {
          hyperx_.widths.at(d) = hyperx_.widths.at(ndim);
        }
      } else {
        // FbFly
        for (u64 d = 0; d < hyperx_.dimensions; d++) {
          hyperx_.widths.at(d)++;
        }
      }
    }
  }
}

void Engine::stage2() {
  for (u64 dim = 1; dim < hyperx_.dimensions; dim++) {
    assert(hyperx_.widths.at(dim) >= hyperx_.widths.at(dim - 1));
  }

  if (HSE_DEBUG >= 5) {
    printf("2: S=%s P=%lu\n", strop::vecString<u64>(hyperx_.widths).c_str(),
           hyperx_.routers);
  }

  // compute the base_radix (no terminals)
  u64 base_radix = 0;
  for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
    base_radix += hyperx_.widths.at(dim) - 1;
  }

  // try possible values for terminals per router ratio
  for (hyperx_.concentration = min_concentration_;
       hyperx_.concentration <= max_concentration_; hyperx_.concentration++) {
    hyperx_.terminals = hyperx_.routers * hyperx_.concentration;
    u64 base_radix2 = base_radix + hyperx_.concentration;
    if ((hyperx_.terminals >= min_terminals_) &&
        (hyperx_.terminals <= max_terminals_) && (base_radix2 <= max_radix_)) {
      stage3();
    } else {
      if (HSE_DEBUG >= 7) {
        printf("2s: SKIPPING S=%s P=%lu T=%lu\n",
               strop::vecString<u64>(hyperx_.widths).c_str(), hyperx_.routers,
               hyperx_.concentration);
      }
    }
    if ((hyperx_.terminals > max_terminals_) || (base_radix2 > max_radix_)) {
      break;
    }
  }
}

void Engine::stage3() {
  if (HSE_DEBUG >= 4) {
    printf("3: S=%s T=%lu N=%lu P=%lu\n",
           strop::vecString<u64>(hyperx_.widths).c_str(), hyperx_.concentration,
           hyperx_.terminals, hyperx_.routers);
  }

  // find the base radix
  u64 base_radix = hyperx_.concentration;
  for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
    base_radix += hyperx_.widths.at(dim) - 1;
  }
  u64 delta_radix = max_radix_ - base_radix;

  // find the amount of weighting that is within maximum bounds
  std::vector<u64> max_weights(hyperx_.dimensions, 1);
  for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
    u64 m = 1 + (delta_radix / (hyperx_.widths.at(dim) - 1));
    if (m > max_weights.at(dim)) {
      max_weights.at(dim) = std::min(max_weight_, m);
    }
  }
  if (HSE_DEBUG >= 4) {
    printf("3: base_radix=%lu delta_radix=%lu, max_weights=%s\n", base_radix,
           delta_radix, strop::vecString<u64>(max_weights).c_str());
  }

  // try finding acceptable weights
  hyperx_.weights.clear();
  hyperx_.weights.resize(hyperx_.dimensions, 1);
  u64 ldim = 0;  // last incremented dimension
  while (true) {
    // compute router radix
    hyperx_.router_radix = hyperx_.concentration;
    for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
      hyperx_.router_radix +=
          ((hyperx_.widths.at(dim) - 1) * hyperx_.weights.at(dim));
    }

    bool too_small_radix = (hyperx_.router_radix < min_radix_);
    bool too_big_radix = (hyperx_.router_radix > max_radix_);

    // test router radix
    if ((too_small_radix || too_big_radix) && (HSE_DEBUG >= 6)) {
      printf("3s: SKIPPING S=%s T=%lu N=%lu P=%lu K=%s R=%lu\n",
             strop::vecString<u64>(hyperx_.widths).c_str(),
             hyperx_.concentration, hyperx_.terminals, hyperx_.routers,
             strop::vecString<u64>(hyperx_.weights).c_str(),
             hyperx_.router_radix);
    }

    // if not already skipped, compute bisection bandwidth
    bool too_small_bandwidth = false;
    bool too_big_bandwidth = false;
    if (!too_small_radix && !too_big_radix) {
      hyperx_.bisections.clear();
      hyperx_.bisections.resize(hyperx_.dimensions, 0.0);
      f64 smallest_bandwidth = F64_POS_INF;
      f64 largest_bandwidth = F64_NEG_INF;
      for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
        hyperx_.bisections.at(dim) =
            (hyperx_.widths.at(dim) * hyperx_.weights.at(dim)) /
            (2.0 * hyperx_.concentration);
        if (hyperx_.bisections.at(dim) < smallest_bandwidth) {
          smallest_bandwidth = hyperx_.bisections.at(dim);
        }
        if (hyperx_.bisections.at(dim) > largest_bandwidth) {
          largest_bandwidth = hyperx_.bisections.at(dim);
        }
      }
      if (smallest_bandwidth < min_bandwidth_) {
        too_small_bandwidth = true;
        if (HSE_DEBUG >= 7) {
          printf("3s: SKIPPING S=%s T=%lu N=%lu P=%lu K=%s R=%lu B=%s\n",
                 strop::vecString<u64>(hyperx_.widths).c_str(),
                 hyperx_.concentration, hyperx_.terminals, hyperx_.routers,
                 strop::vecString<u64>(hyperx_.weights).c_str(),
                 hyperx_.router_radix,
                 strop::vecString<f64>(hyperx_.bisections).c_str());
        }
      } else if (largest_bandwidth > max_bandwidth_) {
        too_big_bandwidth = true;
        if (HSE_DEBUG >= 7) {
          printf("3s: SKIPPING S=%s T=%lu N=%lu P=%lu K=%s R=%lu B=%s\n",
                 strop::vecString<u64>(hyperx_.widths).c_str(),
                 hyperx_.concentration, hyperx_.terminals, hyperx_.routers,
                 strop::vecString<u64>(hyperx_.weights).c_str(),
                 hyperx_.router_radix,
                 strop::vecString<f64>(hyperx_.bisections).c_str());
        }
      }
    }

    // if passed all tests, send to next stage
    if (!too_small_radix && !too_big_bandwidth && !too_big_radix &&
        !too_small_bandwidth) {
      stage4();
    }

    // detect when done, if the last dimension was incremented then
    //  subsequentally skipped due to too large of router radix
    if ((too_big_radix) && (ldim == (hyperx_.dimensions - 1))) {
      break;
    }
    // find the next weights configuration
    if (!fixed_weight_) {
      // HyperX
      u64 ndim = U64_MAX;  // next dimension to increment
      for (ndim = 0; ndim < hyperx_.dimensions; ndim++) {
        if (hyperx_.weights.at(ndim) == max_weights.at(ndim)) {
          continue;
        } else {
          break;
        }
      }
      if (ndim == hyperx_.dimensions) {
        break;
      }
      hyperx_.weights.at(ndim)++;
      ldim = ndim;
      for (u64 d = 0; ndim != 0 && d < ndim; d++) {
        hyperx_.weights.at(d) = hyperx_.weights.at(ndim);
      }
    } else {
      // FbFly
      for (u64 d = 0; d < hyperx_.dimensions; d++) {
        hyperx_.weights.at(d)++;
      }
      ldim = hyperx_.dimensions - 1;
    }
  }
}

void Engine::stage4() {
  for (u64 dim = 1; dim < hyperx_.dimensions; dim++) {
    assert(hyperx_.weights.at(dim) <= hyperx_.weights.at(dim - 1));
  }

  if (HSE_DEBUG >= 3) {
    printf("4: S=%s T=%lu N=%lu K=%s B=%s\n",
           strop::vecString<u64>(hyperx_.widths).c_str(), hyperx_.concentration,
           hyperx_.terminals, strop::vecString<u64>(hyperx_.weights).c_str(),
           strop::vecString<f64>(hyperx_.bisections).c_str());
  }

  // compute the number of channels
  hyperx_.channels = hyperx_.terminals;
  for (u64 dim = 0; dim < hyperx_.dimensions; dim++) {
    u64 triNum = hyperx_.widths.at(dim);
    triNum = (triNum * (triNum - 1)) / 2;
    u64 dim_channels = hyperx_.weights.at(dim) * triNum;
    for (u64 dim2 = 0; dim2 < hyperx_.dimensions; dim2++) {
      if (dim2 != dim) {
        dim_channels *= hyperx_.widths.at(dim2);
      }
    }
    hyperx_.channels += dim_channels;
  }

  stage5();
}

void Engine::stage5() {
  if (HSE_DEBUG >= 2) {
    printf("5: S=%s T=%lu N=%lu P=%lu K=%s R=%lu B=%s\n",
           strop::vecString<u64>(hyperx_.widths).c_str(), hyperx_.concentration,
           hyperx_.terminals, hyperx_.routers,
           strop::vecString<u64>(hyperx_.weights).c_str(), hyperx_.router_radix,
           strop::vecString<f64>(hyperx_.bisections).c_str());
  }

  hyperx_.cost = cost_function_->cost(hyperx_);

  results_.push_back(hyperx_);
  std::sort(results_.begin(), results_.end(), comparator_);

  if (results_.size() > max_results_) {
    results_.pop_back();
  }
}
