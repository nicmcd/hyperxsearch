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
#ifndef SEARCH_ENGINE_H_
#define SEARCH_ENGINE_H_

#include <deque>
#include <vector>

#include "prim/prim.h"

struct Hyperx {
  u64 dimensions;               // L
  std::vector<u64> widths;      // S
  u64 routers;                  // P
  u64 concentration;            // T
  u64 terminals;                // N
  std::vector<u64> weights;     // K
  u64 router_radix;             // R
  std::vector<f64> bisections;  // B
  u64 channels;
  f64 cost;
};

class CostFunction {
 public:
  CostFunction();
  virtual ~CostFunction();
  virtual f64 cost(const Hyperx& _hyperx) const = 0;
};

class Comparator {
 public:
  bool operator()(const Hyperx& _lhs, const Hyperx& _rhs) const;
};

class Engine {
 public:
  Engine(u64 _min_dimensions, u64 _max_dimensions, u64 _min_radix,
         u64 _max_radix, u64 _min_Concentration, u64 _max_concentration,
         u64 _min_terminals, u64 _max_Terminals, f64 _min_bandwidth,
         f64 _max_bandwidth, u64 _max_width, u64 _max_weight, bool _fixed_width,
         bool _fixed_weight, u64 _max_results,
         const CostFunction* _cost_function);
  ~Engine();

  void run();
  const std::deque<Hyperx>& results() const;

 private:
  u64 min_dimensions_;
  u64 max_dimensions_;
  u64 min_radix_;
  u64 max_radix_;
  u64 min_concentration_;
  u64 max_concentration_;
  u64 min_terminals_;
  u64 max_terminals_;
  f64 min_bandwidth_;
  f64 max_bandwidth_;
  u64 max_width_;
  u64 max_weight_;
  bool fixed_width_;
  bool fixed_weight_;
  u64 max_results_;
  const CostFunction* cost_function_;
  Comparator comparator_;
  Hyperx hyperx_;
  std::deque<Hyperx> results_;

  void stage1();
  void stage2();
  void stage3();
  void stage4();
  void stage5();
};

#endif  // SEARCH_ENGINE_H_
