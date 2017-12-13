#include <string>
namespace slg { namespace ocl {
std::string KernelSource_plugin_objectidmask_funcs = 
"#line 2 \"plugin_objectidmask_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxCoreRender.                                   *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ObjectIDMaskFilterPlugin_Apply\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(256, 1, 1))) void ObjectIDMaskFilterPlugin_Apply(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global float *channel_IMAGEPIPELINE,\n"
"		__global uint *channel_FRAMEBUFFER_MASK,\n"
"		__global uint *channel_OBJECT_ID,\n"
"		const uint objectID) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	const uint maskValue = channel_FRAMEBUFFER_MASK[gid];\n"
"	const uint objectIDValue = channel_OBJECT_ID[gid];\n"
"	const float value = (maskValue && (objectIDValue == objectID)) ? 1.f : 0.f;\n"
"\n"
"	__global float *pixel = &channel_IMAGEPIPELINE[gid * 3];\n"
"	pixel[0] = value;\n"
"	pixel[1] = value;\n"
"	pixel[2] = value;\n"
"}\n"
; } }
