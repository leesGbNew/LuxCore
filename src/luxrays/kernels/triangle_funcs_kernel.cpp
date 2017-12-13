#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_triangle_funcs = 
"#line 2 \"triangle_funcs.cl\"\n"
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
"void Triangle_UniformSample(const float u0, const float u1, float *b1, float *b2) {\n"
"	const float su1 = sqrt(u0);\n"
"	*b1 = 1.f - su1;\n"
"	*b2 = u1 * su1;\n"
"}\n"
"\n"
"float3 Triangle_Sample(const float3 p0, const float3 p1, const float3 p2,\n"
"	const float u0, const float u1,\n"
"	float *b0, float *b1, float *b2) {\n"
"	Triangle_UniformSample(u0, u1, b1, b2);\n"
"	*b0 = 1.f - (*b1) - (*b2);\n"
"\n"
"	return (*b0) * p0 + (*b1) * p1 + (*b2) * p2;\n"
"}\n"
"\n"
"float3 Triangle_GetGeometryNormal(const float3 p0, const float3 p1, const float3 p2) {\n"
"	return normalize(cross(p1 - p0, p2 - p0));\n"
"}\n"
"\n"
"float3 Triangle_InterpolateNormal(const float3 n0, const float3 n1, const float3 n2,\n"
"		const float b0, const float b1, const float b2) {\n"
"	return normalize(b0 * n0 + b1 * n1 + b2 * n2);\n"
"}\n"
"\n"
"float2 Triangle_InterpolateUV(const float2 uv0, const float2 uv1, const float2 uv2,\n"
"		const float b0, const float b1, const float b2) {\n"
"	return b0 * uv0 + b1 * uv1 + b2 * uv2;\n"
"}\n"
"\n"
"float3 Triangle_InterpolateColor(const float3 rgb0, const float3 rgb1, const float3 rgb2,\n"
"		const float b0, const float b1, const float b2) {\n"
"	return b0 * rgb0 + b1 * rgb1 + b2 * rgb2;\n"
"}\n"
"\n"
"float Triangle_InterpolateAlpha(const float a0, const float a1, const float a2,\n"
"		const float b0, const float b1, const float b2) {\n"
"	return b0 * a0 + b1 * a1 + b2 * a2;\n"
"}\n"
"\n"
"void Triangle_Intersect(\n"
"		const float3 rayOrig,\n"
"		const float3 rayDir,\n"
"		const float mint,\n"
"		float *maxt,\n"
"		uint *hitMeshIndex,\n"
"		uint *hitTriangleIndex,\n"
"		float *hitB1,\n"
"		float *hitB2,\n"
"		const uint currentMeshIndex,\n"
"		const uint currentTriangleIndex,\n"
"		const float3 v0,\n"
"		const float3 v1,\n"
"		const float3 v2) {\n"
"	// Calculate intersection\n"
"	const float3 e1 = v1 - v0;\n"
"	const float3 e2 = v2 - v0;\n"
"	const float3 s1 = cross(rayDir, e2);\n"
"\n"
"	const float divisor = dot(s1, e1);\n"
"	if (divisor == 0.f)\n"
"		return;\n"
"\n"
"	const float invDivisor = 1.f / divisor;\n"
"\n"
"	// Compute first barycentric coordinate\n"
"	const float3 d = rayOrig - v0;\n"
"	const float b1 = dot(d, s1) * invDivisor;\n"
"	if (b1 < 0.f)\n"
"		return;\n"
"\n"
"	// Compute second barycentric coordinate\n"
"	const float3 s2 = cross(d, e1);\n"
"	const float b2 = dot(rayDir, s2) * invDivisor;\n"
"	if (b2 < 0.f)\n"
"		return;\n"
"\n"
"	const float b0 = 1.f - b1 - b2;\n"
"	if (b0 < 0.f)\n"
"		return;\n"
"\n"
"	// Compute _t_ to intersection point\n"
"	const float t = dot(e2, s2) * invDivisor;\n"
"	if (t < mint || t > *maxt)\n"
"		return;\n"
"\n"
"	*maxt = t;\n"
"	*hitB1 = b1;\n"
"	*hitB2 = b2;\n"
"	*hitMeshIndex = currentMeshIndex;\n"
"	*hitTriangleIndex = currentTriangleIndex;\n"
"}\n"
; } }
