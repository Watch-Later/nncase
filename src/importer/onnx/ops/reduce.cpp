/* Copyright 2020 Alexey Chernov <4ernov@gmail.com>
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

#include "../onnx_importer.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <nncase/ir/graph.h>
#include <nncase/ir/ops/reduce.h>
#include <nncase/ir/ops/unary.h>

using namespace nncase;
using namespace nncase::importer;
using namespace nncase::ir;
using namespace onnx;

void onnx_importer::convert_op_ReduceMax(const NodeProto &node)
{
    convert_reduce(node, reduce_max, std::numeric_limits<float>::lowest());
}

void onnx_importer::convert_op_ReduceMean(const NodeProto &node)
{
    convert_reduce(node, reduce_mean, 0.f);
}

void onnx_importer::convert_op_ReduceMin(const NodeProto &node)
{
    convert_reduce(node, reduce_min, std::numeric_limits<float>::max());
}

void onnx_importer::convert_op_ReduceSum(const NodeProto &node)
{
    convert_reduce(node, reduce_sum, 0.f);
}

void onnx_importer::convert_reduce(const NodeProto &node, const reduce_op_t reduce_op, const float init_value)
{
    const auto &op_name { generate_name(node) };

    const auto &input = node.input()[0];
    const auto &output = node.output()[0];

    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);

    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    bool keepdims = true;
    const auto &keepdims_attr = get_attribute<int>(node, "keepdims");
    if (keepdims_attr)
        keepdims = static_cast<bool>(keepdims_attr.value());

    auto op = graph_.emplace<reduce>(reduce_op, input_type, input_shape, std::move(axes), init_value, keepdims);
    op->name(op_name + '(' + reduce_op_to_string(reduce_op) + ')');

    input_tensors_.emplace(&op->input(), input);
    output_tensors_.emplace(output, &op->output());
}

void onnx_importer::convert_op_ReduceL1(const NodeProto &node)
{
    const auto &op_name { generate_name(node) };

    const auto &input = node.input()[0];
    const auto &output = node.output()[0];
    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);

    // axes
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);
    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    // keepdims
    auto keepdims_attr = get_attribute<int>(node, "keepdims");
    bool keepdims = keepdims_attr ? keepdims_attr.value() == 1 : true;

    auto abs = graph_.emplace<unary>(unary_abs, input_shape);
    abs->name(op_name + ".abs(ReduceL1)");

    auto sum = graph_.emplace<reduce>(reduce_sum, input_type, abs->output().shape(), axes, 0.f, keepdims);
    sum->name(op_name + ".reduce_sum(ReduceL1)");

    sum->input().connect(abs->output());

    input_tensors_.emplace(&abs->input(), input);
    output_tensors_.emplace(output, &sum->output());
}

void onnx_importer::convert_op_ReduceL2(const NodeProto &node)
{
    const auto &op_name { generate_name(node) };

    const auto &input = node.input()[0];
    const auto &output = node.output()[0];
    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);

    // axes
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);
    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    // keepdims
    auto keepdims_attr = get_attribute<int>(node, "keepdims");
    bool keepdims = keepdims_attr ? keepdims_attr.value() == 1 : true;

    auto square = graph_.emplace<unary>(unary_square, input_shape);
    square->name(op_name + ".square(ReduceL2)");

    auto sum = graph_.emplace<reduce>(reduce_sum, input_type, square->output().shape(), axes, 0.f, keepdims);
    sum->name(op_name + ".reduce_sum(ReduceL2)");

    auto sqrt = graph_.emplace<unary>(unary_sqrt, sum->output().shape());
    sqrt->name(op_name + ".sqrt(ReduceL2)");

    sum->input().connect(square->output());
    sqrt->input().connect(sum->output());

    input_tensors_.emplace(&square->input(), input);
    output_tensors_.emplace(output, &sqrt->output());
}

// ReduceLogSum(x) = Log(ReduceSum(x))
void onnx_importer::convert_op_ReduceLogSum(const NodeProto &node)
{
    const auto &op_name { generate_name(node) };
    const auto &input = node.input()[0];
    const auto &output = node.output()[0];
    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);

    // axes
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);
    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    // keepdims
    auto keepdims_attr = get_attribute<int>(node, "keepdims");
    bool keepdims = keepdims_attr ? keepdims_attr.value() == 1 : true;

    // sum
    auto sum = graph_.emplace<reduce>(reduce_sum, input_type, input_shape, axes, 0.f, keepdims);
    sum->name(op_name + ".reduce_sum(ReduceLogSum)");

    // log
    auto log = graph_.emplace<unary>(unary_log, sum->output().shape());
    log->name(op_name + ".log(ReduceLogSum)");
    log->input().connect(sum->output());

    input_tensors_.emplace(&sum->input(), input);
    output_tensors_.emplace(output, &log->output());
}

// ReduceLogSumExp(x) = Log(Sum(Exp(x)))
void onnx_importer::convert_op_ReduceLogSumExp(const NodeProto &node)
{
    const auto &op_name { generate_name(node) };
    const auto &input = node.input()[0];
    const auto &output = node.output()[0];
    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);

    // axes
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);
    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    // keepdims
    auto keepdims_attr = get_attribute<int>(node, "keepdims");
    bool keepdims = keepdims_attr ? keepdims_attr.value() == 1 : true;

    // exp
    auto exp = graph_.emplace<unary>(unary_exp, input_shape);
    exp->name(op_name + ".exp(ReduceLogSumExp)");

    // sum
    auto sum = graph_.emplace<reduce>(reduce_sum, input_type, exp->output().shape(), axes, 0.f, keepdims);
    sum->name(op_name + ".reduce_sum(ReduceLogSumExp)");
    sum->input().connect(exp->output());

    // log
    auto log = graph_.emplace<unary>(unary_log, sum->output().shape());
    log->name(op_name + ".log(ReduceLogSumExp)");
    log->input().connect(sum->output());

    input_tensors_.emplace(&exp->input(), input);
    output_tensors_.emplace(output, &log->output());
}

// ReduceSumSquare(x) = Sum(Square(x))
void onnx_importer::convert_op_ReduceSumSquare(const NodeProto &node)
{
    const auto &op_name { generate_name(node) };
    const auto &input = node.input()[0];
    const auto &output = node.output()[0];
    const auto input_type = get_datatype(input).value();
    const auto &input_shape = get_shape(input);

    // axes
    axis_t axes(input_shape.size());
    std::iota(begin(axes), end(axes), 0);
    const auto &axes_attr = get_attribute<axis_t>(node, "axes");
    if (axes_attr)
    {
        axes = axes_attr.value();
        std::transform(std::begin(axes), std::end(axes), std::begin(axes),
            [&input_shape](const auto e) { return real_axis(e, input_shape.size()); });
    }

    // keepdims
    auto keepdims_attr = get_attribute<int>(node, "keepdims");
    bool keepdims = keepdims_attr ? keepdims_attr.value() == 1 : true;

    // square
    auto square = graph_.emplace<unary>(unary_square, input_shape);
    square->name(op_name + ".square(ReduceSumSquare)");

    // sum
    auto sum = graph_.emplace<reduce>(reduce_sum, input_type, square->output().shape(), axes, 0.f, keepdims);
    sum->name(op_name + ".reduce_sum(ReduceSumSquare)");
    sum->input().connect(square->output());

    input_tensors_.emplace(&square->input(), input);
    output_tensors_.emplace(output, &sum->output());
}
