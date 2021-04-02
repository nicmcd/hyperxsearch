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
#include <deque>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "grid/Grid.h"
#include "prim/prim.h"
#include "search/Calculator.h"
#include "search/CalculatorFactory.h"
#include "search/Engine.h"
#include "strop/strop.h"
#include "tclap/CmdLine.h"

s32 main(s32 _argc, char** _argv) {
  u64 min_dimensions;
  u64 max_dimensions;
  u64 min_radix;
  u64 max_radix;
  u64 min_concentration;
  u64 max_concentration;
  u64 min_terminals;
  u64 max_terminals;
  f64 min_bandwidth;
  f64 max_bandwidth;
  u64 max_width;
  u64 max_weight;
  bool fixed_width;
  bool fixed_weight;
  u64 max_results;
  bool print_settings;
  std::string cost_calc;

  std::string version = "1.1";
  std::string description =
      ("Search HyperX topologies for optimal solutions. Copyright (c) 2016. "
       "Nic McDonald. See LICENSE file for details.");

  try {
    // create the command line parser
    TCLAP::CmdLine cmd(description, ' ', version);

    // define command line args
    TCLAP::ValueArg<u64> min_dimensions_arg("", "mindimensions",
                                            "minimum number of dimensions",
                                            false, 1, "u64", cmd);
    TCLAP::ValueArg<u64> max_dimensions_arg("", "maxdimensions",
                                            "maximum number of dimensions",
                                            false, 4, "u64", cmd);
    TCLAP::ValueArg<u64> min_radix_arg("", "minradix", "minimum router radix",
                                       false, 2, "u64", cmd);
    TCLAP::ValueArg<u64> max_radix_arg("", "maxradix", "maximum router radix",
                                       false, 64, "u64", cmd);
    TCLAP::ValueArg<u64> min_concentration_arg("", "minconcentration",
                                               "minimum router concentration",
                                               false, 1, "u64", cmd);
    TCLAP::ValueArg<u64> max_concentration_arg("", "maxconcentration",
                                               "maximum router concentration",
                                               false, U32_MAX - 1, "u64", cmd);
    TCLAP::ValueArg<u64> min_terminals_arg("", "minterminals",
                                           "minimum number of terminals", false,
                                           32768, "u64", cmd);
    TCLAP::ValueArg<u64> max_terminals_arg("", "maxterminals",
                                           "maximum number of terminals", false,
                                           0, "u64", cmd);
    TCLAP::ValueArg<f64> min_bandwidth_arg(
        "", "minbandwidth", "minimum relative bisection bandwidth", false, 0.5,
        "f64", cmd);
    TCLAP::ValueArg<f64> max_bandwidth_arg(
        "", "maxbandwidth", "maximum relative bisection bandwidth", false,
        F64_POS_INF, "f64", cmd);
    TCLAP::ValueArg<u64> max_width_arg("", "maxwidth",
                                       "maximum width of any dimension", false,
                                       U32_MAX - 1, "u64", cmd);
    TCLAP::ValueArg<u64> max_weight_arg("", "maxweight",
                                        "maximum weight of any dimension",
                                        false, U32_MAX - 1, "u64", cmd);
    TCLAP::SwitchArg fixed_width_arg(
        "", "fixedwidth", "only search fixed width (fbfly) topologies", cmd,
        false);
    TCLAP::SwitchArg fixed_weight_arg(
        "", "fixedweight", "only search fixed weight (fbfly) topologies", cmd,
        false);
    TCLAP::ValueArg<u64> max_results_arg(
        "", "maxresults", "maximum number of results", false, 10, "u64", cmd);
    TCLAP::ValueArg<std::string> cost_calc_arg(
        "", "costcalc", "cost calculator to use", false, "router_channel_count",
        "string", cmd);
    TCLAP::SwitchArg print_settings_arg("p", "printsettings",
                                        "print the input settings", cmd, false);

    // parse the command line
    cmd.parse(_argc, _argv);

    // copy values out to variables
    min_dimensions = min_dimensions_arg.getValue();
    max_dimensions = max_dimensions_arg.getValue();
    min_radix = min_radix_arg.getValue();
    max_radix = max_radix_arg.getValue();
    min_concentration = min_concentration_arg.getValue();
    max_concentration = max_concentration_arg.getValue();
    min_terminals = min_terminals_arg.getValue();
    max_terminals = max_terminals_arg.getValue();
    if (max_terminals == 0) {
      max_terminals = min_terminals * 2;
    }
    min_bandwidth = min_bandwidth_arg.getValue();
    max_bandwidth = max_bandwidth_arg.getValue();
    max_width = max_width_arg.getValue();
    max_weight = max_weight_arg.getValue();
    fixed_width = fixed_width_arg.getValue();
    fixed_weight = fixed_weight_arg.getValue();
    max_results = max_results_arg.getValue();
    print_settings = print_settings_arg.getValue();
    cost_calc = cost_calc_arg.getValue();
  } catch (TCLAP::ArgException& e) {
    throw std::runtime_error(e.error().c_str());
  }

  // if in verbose mode, print input settings
  if (print_settings) {
    printf(
        "input settings:\n"
        "  min_dimensions = %lu\n"
        "  max_dimensions = %lu\n"
        "  min_radix = %lu\n"
        "  max_radix = %lu\n"
        "  min_concentration = %lu\n"
        "  max_concentration = %lu\n"
        "  min_terminals = %lu\n"
        "  max_terminals = %lu\n"
        "  min_bandwidth = %f\n"
        "  max_bandwidth = %f\n"
        "  max_width = %lu\n"
        "  max_weight = %lu\n"
        "  fixed_width = %s\n"
        "  fixed_weight = %s\n"
        "  max_results = %lu\n"
        "  cost_calc = %s\n"
        "\n",
        min_dimensions, max_dimensions, min_radix, max_radix, min_concentration,
        max_concentration, min_terminals, max_terminals, min_bandwidth,
        max_bandwidth, max_width, max_weight, (fixed_width ? "yes" : "no"),
        (fixed_weight ? "yes" : "no"), max_results, cost_calc.c_str());
  }

  // create the cost calculator
  Calculator* calc = CalculatorFactory::createCalculator(cost_calc);

  // create and run the engine
  Engine engine(min_dimensions, max_dimensions, min_radix, max_radix,
                min_concentration, max_concentration, min_terminals,
                max_terminals, min_bandwidth, max_bandwidth, max_width,
                max_weight, fixed_width, fixed_weight, max_results, calc);
  engine.run();

  // gather the results
  const std::deque<Hyperx>& results = engine.results();

  // create the output grid
  const std::vector<std::string>& ext_fields = calc->extFields();
  grid::Grid grid(1 + results.size(), 11 + ext_fields.size());

  // format the regular header
  grid.set(0, 0, "#");
  grid.set(0, 1, "Dimensions");
  grid.set(0, 2, "Widths");
  grid.set(0, 3, "Weights");
  grid.set(0, 4, "Concentration");
  grid.set(0, 5, "Terminals");
  grid.set(0, 6, "Routers");
  grid.set(0, 7, "Radix");
  grid.set(0, 8, "Channels");
  grid.set(0, 9, "Bisections");
  grid.set(0, 10, "Cost");

  // format the extension header
  for (u64 ext = 0; ext < ext_fields.size(); ext++) {
    grid.set(0, 11 + ext, ext_fields.at(ext));
  }

  // format the data section
  for (u64 idx = 0; idx < results.size(); idx++) {
    u64 row = idx + 1;

    // get the results
    const Hyperx& res = results.at(idx);

    // format the regular values in the row
    grid.set(row, 0, std::to_string(row));
    grid.set(row, 1, std::to_string(res.dimensions));
    grid.set(row, 2, strop::vecString<u64>(res.widths).c_str());
    grid.set(row, 3, strop::vecString<u64>(res.weights).c_str());
    grid.set(row, 4, std::to_string(res.concentration));
    grid.set(row, 5, std::to_string(res.terminals));
    grid.set(row, 6, std::to_string(res.routers));
    grid.set(row, 7, std::to_string(res.router_radix));
    grid.set(row, 8, std::to_string(res.channels));
    grid.set(row, 9, strop::vecString<f64>(res.bisections, ',', 2).c_str());
    grid.set(row, 10, std::to_string(res.cost));

    // get extension values from the calculator
    const std::unordered_map<std::string, std::string>& ext_values =
        calc->extValues(res);

    // format the extensions values in the row
    for (u64 ext = 0; ext < ext_fields.size(); ext++) {
      grid.set(row, 11 + ext, ext_values.at(ext_fields.at(ext)));
    }
  }

  // print the output grid
  printf("%s", grid.toString().c_str());

  // cleanup
  delete calc;

  return 0;
}
