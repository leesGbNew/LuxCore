#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_clamp_funcs = 
"#line 2 \"texture_clamp_funcs.cl\"\n"
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
"// Clamp texture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_CLAMP)\n"
"\n"
"float ClampTexture_ConstEvaluateFloat(__global HitPoint *hitPoint,\n"
"		const float v, const float minVal, const float maxVal) {\n"
"	return clamp(v, minVal, maxVal);\n"
"}\n"
"\n"
"float3 ClampTexture_ConstEvaluateSpectrum(__global HitPoint *hitPoint,\n"
"		const float3 v, const float minVal, const float maxVal) {\n"
"	return clamp(v, minVal, maxVal);\n"
"}\n"
"\n"
"#endif\n"
; } }
