﻿using System;
using System.Collections.Generic;
using System.Text;
using NnCase.Evaluation;
using NnCase.IR;
using NnCase.Transforms;

namespace NnCase.Targets
{
    public abstract class Target
    {
        public void OptimizePass2(Graph graph)
        {
            var transforms = GetDefaultTransforms();

            AddOptimize2Transforms(transforms);
            Transform.TransformGraph(graph, transforms);
        }

        public void AddQuantizationCheckpoints(Graph graph)
        {
            var transforms = GetDefaultTransforms();

            AddQuantizationCheckpointsTransforms(transforms);
            Transform.TransformGraph(graph, transforms);
        }

        public void QuantizeGraph(Graph graph, Quantizer quantizer)
        {
            var transforms = GetDefaultTransforms();

            AddQuantizeTransforms(transforms, quantizer);
            Transform.TransformGraph(graph, transforms);
        }

        private List<Transform> GetDefaultTransforms()
        {
            return new List<Transform>
            {
                new FoldTransposeTransform(),
                new FoldNopTransposeTransform(),
                new FoldNopReshapeTransform(),
                new ConstantFoldingTransform(),
                new TransposeBinaryMotionTransform(),
                new TransposeConcatMotionTransform(),
                new TransposeReduceMotionTransform(),
                new FoldQuantizeTransform()
            };
        }

        public virtual void RegisterEvaluators(EvaluatorRegistry registry)
        {
        }

        protected virtual void AddOptimize2Transforms(List<Transform> transforms)
        {
        }

        protected virtual void AddQuantizationCheckpointsTransforms(List<Transform> transforms)
        {
        }

        protected virtual void AddQuantizeTransforms(List<Transform> transforms, Quantizer quantizer)
        {
        }
    }
}
