/* Copyright 2019-2021 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../tflite_importer.h"
#include <nncase/ir/ops/convert.h>
#include <nncase/ir/ops/gather_nd.h>

using namespace nncase;
using namespace nncase::importer;
using namespace nncase::ir;

DEFINE_TFLITE_LOWER(GATHER_ND)
{
    auto &input = get_tensor(op.inputs(), 0);
    auto &indices = get_tensor(op.inputs(), 1);
    auto &output = get_tensor(op.outputs(), 0);

    auto in_shape = get_shape(input.shape());
    auto indices_shape = get_shape(indices.shape());
    auto out_shape = get_shape(output.shape());

    const auto in_type = to_data_type(input.type());
    auto ga = graph_.emplace<gather_nd>(in_type, in_shape, indices_shape, out_shape, 0);
    ga->name(get_tensor(op.outputs(), 0).name()->string_view());
    input_convert_to_type(ga->indices(), indices, op.inputs()->Get(1), dt_int32);
    link_input_tensor(&ga->input(), op.inputs()->Get(0));
    link_output_tensor(op.outputs()->Get(0), &ga->output());
}