#include <string>
namespace slg { namespace ocl {
std::string KernelSource_hitpoint_types = 
"#line 2 \"hitpoint_types.cl\"\n"
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
"// This is defined only under OpenCL because of variable size structures\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"typedef struct {\n"
"	// The incoming direction. It is the eyeDir when fromLight = false and\n"
"	// lightDir when fromLight = true\n"
"	Vector fixedDir;\n"
"	Point p;\n"
"	UV uv;\n"
"	Normal geometryN;\n"
"	Normal shadeN;\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"	// Note: dpdu and dpdv are orthogonal to shading normal (i.e not geometry normal)\n"
"	Vector dpdu, dpdv;\n"
"	Normal dndu, dndv;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTCOLOR) || defined(PARAM_ENABLE_TEX_HITPOINTGREY) || defined(PARAM_TRIANGLE_LIGHT_HAS_VERTEX_COLOR)\n"
"	Spectrum color;\n"
"#endif\n"
"#if defined(PARAM_ENABLE_TEX_HITPOINTALPHA)\n"
"	float alpha;\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	// passThroughEvent can be stored here in a path state even before of\n"
"	// BSDF initialization (while tracing the next path vertex ray)\n"
"	float passThroughEvent;\n"
"#endif\n"
"\n"
"	// Transformation from world to local object reference frame\n"
"	Matrix4x4 worldToLocal;\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"	// Interior and exterior volume (this includes volume priority system\n"
"	// computation and scene default world volume)\n"
"	unsigned int interiorVolumeIndex, exteriorVolumeIndex;\n"
"	// Material code (i.e. glass, etc.) doesn't have access to materials list\n"
"	// so I use HitPoint to carry texture index information\n"
"	unsigned int interiorIorTexIndex, exteriorIorTexIndex;\n"
"	int intoObject;\n"
"#endif\n"
"} HitPoint;\n"
"\n"
"#endif\n"
; } }
