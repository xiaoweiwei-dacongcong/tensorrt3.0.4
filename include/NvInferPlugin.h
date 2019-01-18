/*
 * Copyright 1993-2016 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

#ifndef NV_INFER_PLUGIN_H
#define NV_INFER_PLUGIN_H

#include "NvInfer.h"

/**
 * \file NvInferPlugin.h
 *
 * This is the API for the NVidia provided TensorRT plugins.
 */

namespace nvinfer1
{
    /**
     * \enum PluginType
     *
     * \brief the type values for the various plugins
     *
     * \see INvPlugin::getPluginType()
     */
    enum class PluginType : int
    {
        kFASTERRCNN = 0,         //!< FasterRCNN fused plugin (RPN + ROI pooling)
		kNORMALIZE = 1,          //!< Normalize plugin
		kPERMUTE = 2,            //!< Permute plugin
		kPRIORBOX = 3,           //!< PriorBox plugin
		kSSDDETECTIONOUTPUT = 4, //!< SSD DetectionOutput plugin
		kCONCAT = 5,             //!< Concat plugin 
        kPRELU = 6,          //!< YOLO PReLU Plugin
        kYOLOREORG = 7,              //!< YOLO Reorg Plugin
        kYOLOREGION = 8,             //!< YOLO Region Plugin
    };  
    template<> inline int EnumMax<PluginType>() { return 9; } //!<maximum number of elements in PluginType enum. \see PluginType
namespace plugin
{
        /**
     * \class INvPlugin
     * 
     * \brief Common interface for the NVidia created plugins.
     *
     * This class provides a common subset of functionality that is used
     * to provide distinguish the NVidia created plugins. Each plugin provides a
     * function to validate the parameter options and create the plugin
     * object. 
     */
    class INvPlugin : public IPlugin
    {
        public:
            /**
             * \brief get the parameter plugin ID.
             *
             * \return the ID of the plugin.
             */
            virtual PluginType getPluginType() const = 0;

            /**
             * \brief get the name of the plugin from the ID
             *
             * \return the name of the plugin specified by \p id.
             * \return nullptr if invalid ID is specified
             *
             * The valid \p id values are ranged [0, numPlugins()).
             */
            virtual const char *getName() const = 0;

			/**
			* \brief destroy the plugin
			*
			* The valid \p id values are ranged [0, numPlugins()).
			*/
			virtual void destroy() = 0;
        protected:
            ~INvPlugin() {}
    };  // INvPlugin

    /**
	 * \params featureStride <feature stride>
     * \params preNmsTop <number of proposals to keep before applying NMS>
     * \params nmsMaxOut <number of remaining proposals after applying NMS> 
     * \params iouThreshold <IoU threshold>
     * \params minBoxSize <minimum allowed bounding box size before scaling>
     * \params spatialScale <spatial scale between the input image and the last feature map>
     * \params pooling <spatial dimensions of pooled ROIs>
     * \params anchorRatios <aspect ratios for generating anchor windows>
     * \params anchorScales <scales for generating anchor windows>
     * \brief create a plugin layer that fuses the RPN and ROI pooling using user-defined parameters.
     *
     * \return returns nullptr on invalid inputs.
     * \return returns a FasterRCNN fused RPN+ROI pooling plugin
     *
     * \see INvPlugin
     */
    INvPlugin * createFasterRCNNPlugin(int featureStride, int preNmsTop,
									   int nmsMaxOut, float iouThreshold, float minBoxSize,
									   float spatialScale, DimsHW pooling,
									   Weights anchorRatios, Weights anchorScales);
    INvPlugin * createFasterRCNNPlugin(const void * data, size_t length);


    /* 
	 * \brief The Normalize plugin layer normalizes the input to have L2 norm of 1 with scale learnable.
     * \params scales scale weights that are applied to the output tensor
     * \params acrossSpatial whether to compute the norm over adjacent channels (acrossSpatial is true) or nearby spatial locations (within channel in which case acrossSpatial is false)
     * \params channelShared whether the scale weight(s) is shared across channels
     * \params eps epsilon for not diviiding by zero
	 */
    INvPlugin * createSSDNormalizePlugin(const Weights *scales, bool acrossSpatial, bool channelShared, float eps);
    INvPlugin * createSSDNormalizePlugin(const void * data, size_t length);

   /*
	* \brief The Permute plugin layer permutes the input tensor by changing the memory order of the data.
	* Quadruple defines a structure that contains an array of 4 integers. They can represent the permute orders or the strides in each dimension. 
	*/
    typedef struct
	{
		int data[4];
	} Quadruple;
   /*
    * \params permuteOrder <The new orders that are used to permute the data.>
	*/ 
    INvPlugin * createSSDPermutePlugin(Quadruple permuteOrder);
    INvPlugin * createSSDPermutePlugin(const void * data, size_t length);

   /*
    * \brief The PriorBox plugin layer generates the prior boxes of designated sizes and aspect ratios across all dimensions @f$ (H \times W) @f$.
    * PriorBoxParameters defines a set of parameters for creating the PriorBox plugin layer. 
    * It contains: 
    * \params minSize minimum box size in pixels. Can not be nullptr
    * \params maxSize maximum box size in pixels. Can be nullptr
    * \params aspectRatios aspect ratios of the boxes. Can be nullptr
    * \params numMinSize number of elements in minSize. Must be larger than 0
    * \params numMaxSize number of elements in maxSize. Can be 0 or same as numMinSize
    * \params numAspectRatios number of elements in aspectRatios. Can be 0
    * \params flip If true, will flip each aspect ratio. For example, if there is aspect ratio "r", the aspect ratio "1.0/r" will be generated as well.
    * \params clip If true, will clip the prior so that it is within [0,1]
    * \params variance variance for adjusting the prior boxes.
    * \params imgH image height. If 0, then the H dimension of the data tensor will be used
    * \params imgW image width. If 0, then the W dimension of the data tensor will be used
	* \params stepH step in H. If 0, then (float)imgH/h will be used where h is the H dimension of the 1st input tensor
    * \params stepW step in W. If 0, then (float)imgW/w will be used where w is the W dimension of the 1st input tensor
    * \params offset offset to the top left corner of each cell
	*/
    struct PriorBoxParameters
	{
		float *minSize, *maxSize, *aspectRatios;
		int numMinSize, numMaxSize, numAspectRatios;
		bool flip;
		bool clip;
		float variance[4];
		int imgH, imgW;
		float stepH, stepW;
		float offset;
	};

   /*
    * \params param set of parameters for creating the PriorBox plugin layer
    */

    INvPlugin * createSSDPriorBoxPlugin(PriorBoxParameters param);
    INvPlugin * createSSDPriorBoxPlugin(const void * data, size_t length);


   /*
	* \brief The DetectionOutput plugin layer generates the detection output based on location and confidence predictions by doing non maximum suppression. 
    * DetectionOutputParameters defines a set of parameters for creating the DetectionOutput plugin layer. 
    * It contains:
    * \params shareLocation If true, bouding box are shared among different classes
    * \params varianceEncodedInTarget If true, variance is encoded in target. Otherwise we need to adjust the predicted offset accordingly
    * \params backgroundLabelId background label ID. If there is no background class, set it as -1
    * \params numClasses number of classes to be predicted
    * \params topK number of boxes per image with top confidence scores that are fed into the NMS algorithm
    * \params keepTopK number of total bounding boxes to be kept per image after NMS step
    * \params confidenceThreshold only consider detections whose confidences are larger than a threshold
    * \params nmsThreshold threshold to be used in NMS
    * \params codeType type of coding method for bbox
	*/
    typedef enum
 	{
		CORNER      = 1,
		CENTER_SIZE = 2,
		CORNER_SIZE = 3
	} CodeType_t;

    struct DetectionOutputParameters
	{
		bool shareLocation, varianceEncodedInTarget;
		int backgroundLabelId, numClasses, topK, keepTopK;
		float confidenceThreshold, nmsThreshold;
		CodeType_t codeType;
	};

   /*
    * \params param set of parameters for creating the DetectionOutput plugin layer
	*/

    INvPlugin * createSSDDetectionOutputPlugin(DetectionOutputParameters param);
    INvPlugin * createSSDDetectionOutputPlugin(const void * data, size_t length);

   /*
	* \brief The Concat plugin layer basically performs the concatention for 4D tensors. Unlike the Concatenation layer in early version of TensorRT, it allows the user to specify the axis along which to concatenate. The axis can be 1 (across channel), 2 (across H), or 3 (across W). More particularly, this Concat plugin layer also implements the "ignoring the batch dimension" switch. If turned on, all the input tensors will be treated as if their batch sizes were 1. 
    * \params concatAxis axis along which to concatenate. Can't be the "N" dimension
    * \params ignoreBatch If true, all the input tensors will be treated as if their batch sizes were 1
	*/

    INvPlugin * createConcatPlugin(int concatAxis, bool ignoreBatch);
	INvPlugin * createConcatPlugin(const void * data, size_t length);


   /*
	* \brief The PReLu plugin layer performs leaky ReLU for 4D tensors. Give an input value x, the PReLU layer computes the output as x if x > 0 and negative_slope * x if x <= 0.
    * \params negSlope negative_slope value
	*/

    INvPlugin * createPReLUPlugin(float negSlope);
    INvPlugin * createPReLUPlugin(const void * data, size_t length);

   /*
	* \brief The Reorg plugin layer maps the 512x26x26 feature map onto a 2048x13x13 feature map, so that it can be concatenated with the feature maps at 13x13 resolution.
    * \params stride strides in H and W
	*/

    INvPlugin * createYOLOReorgPlugin(int stride);
    INvPlugin * createYOLOReorgPlugin(const void * data, size_t length);

   /*
	* \brief The Region plugin layer performs region proposal calculation: generate 5 bounding boxes per cell (for yolo9000, generate 3 bounding boxes per cell). For each box, calculating its probablities of objects detections from 80 pre-defined classifications (yolo9000 has 9416 pre-defined classifications, and these 9416 items are organized as work-tree structure). 
    * RegionParameters defines a set of parameters for creating the Region plugin layer. 
    * \params num number of predicted bounding box for each grid cell
    * \params coords number of coordinates for a bounding box 
    * \params classes number of classfications to be predicted
    * \params softmaxTree when performing yolo9000, softmaxTree is helping to do softmax on confidence scores, for element to get the precise classfication through word-tree structured classfication definition.
	*/

    typedef struct{
        int *leaf;
        int n;
        int *parent;
        int *child;
        int *group;
        char **name;

        int groups;
        int *groupSize;
        int *groupOffset;
    } softmaxTree; // softmax tree

    struct RegionParameters
    {
        int num;
        int coords;
        int classes;
        softmaxTree * smTree;
    };

    INvPlugin * createYOLORegionPlugin(RegionParameters params);
    INvPlugin * createYOLORegionPlugin(const void * data, size_t length);

} // end plugin namespace
} // end nvinfer1 namespace
#endif // NV_INFER_PLUGIN_H
