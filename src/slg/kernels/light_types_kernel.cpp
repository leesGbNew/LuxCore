#include <string>
namespace slg { namespace ocl {
std::string KernelSource_light_types = 
"#line 2 \"light_types.cl\"\n"
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
"typedef enum {\n"
"	TYPE_IL, TYPE_IL_SKY, TYPE_SUN, TYPE_TRIANGLE, TYPE_POINT, TYPE_MAPPOINT,\n"
"	TYPE_SPOT, TYPE_PROJECTION, TYPE_IL_CONSTANT, TYPE_SHARPDISTANT, TYPE_DISTANT,\n"
"	TYPE_IL_SKY2, TYPE_LASER,\n"
"	LIGHT_SOURCE_TYPE_COUNT\n"
"} LightSourceType;\n"
"\n"
"typedef struct {\n"
"	TextureMapping2D mapping;\n"
"	unsigned int imageMapIndex;\n"
"	unsigned int distributionOffset;\n"
"} InfiniteLightParam;\n"
"\n"
"typedef struct {\n"
"	float absoluteTheta;\n"
"	float absolutePhi;\n"
"	float zenith_Y, zenith_x, zenith_y;\n"
"	float perez_Y[6], perez_x[6], perez_y[6];\n"
"} SkyLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absoluteSunDir, absoluteUpDir;\n"
"	Spectrum aTerm, bTerm, cTerm, dTerm, eTerm, fTerm,\n"
"		gTerm, hTerm, iTerm, radianceTerm;\n"
"	int hasGround, isGroundBlack;\n"
"	Spectrum scaledGroundColor;\n"
"} SkyLight2Param;\n"
"\n"
"typedef struct {\n"
"	Vector absoluteDir;\n"
"	float turbidity, relSize;\n"
"	// XY Vectors for cone sampling\n"
"	Vector x, y;\n"
"	float cosThetaMax, sin2ThetaMax;\n"
"	Spectrum color;\n"
"} SunLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absolutePos;\n"
"	Spectrum emittedFactor;\n"
"} PointLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absolutePos, localPos;\n"
"	Spectrum emittedFactor;\n"
"	float avarage;\n"
"	unsigned int imageMapIndex;\n"
"} MapPointLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absolutePos;\n"
"	Spectrum emittedFactor;\n"
"	float cosTotalWidth, cosFalloffStart;\n"
"} SpotLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absolutePos, lightNormal;\n"
"	Spectrum emittedFactor;\n"
"	Matrix4x4 lightProjection;\n"
"	float screenX0, screenX1, screenY0, screenY1;\n"
"	unsigned int imageMapIndex;\n"
"} ProjectionLightParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum color;\n"
"} ConstantInfiniteLightParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum color;\n"
"	Vector absoluteLightDir;\n"
"} SharpDistantLightParam;\n"
"\n"
"typedef struct {\n"
"	Spectrum color;\n"
"	Vector absoluteLightDir, x, y;\n"
"	float cosThetaMax;\n"
"} DistantLightParam;\n"
"\n"
"typedef struct {\n"
"	Vector absoluteLightPos, absoluteLightDir;\n"
"	Spectrum emittedFactor;\n"
"	float radius;\n"
"} LaserLightParam;\n"
"\n"
"typedef struct {\n"
"	Transform light2World;\n"
"	Spectrum gain;\n"
"\n"
"	union {\n"
"		SunLightParam sun;\n"
"		SkyLightParam sky;\n"
"		SkyLight2Param sky2;\n"
"		InfiniteLightParam infinite;\n"
"		PointLightParam point;\n"
"		MapPointLightParam mapPoint;\n"
"		SpotLightParam spot;\n"
"		ProjectionLightParam projection;\n"
"		ConstantInfiniteLightParam constantInfinite;\n"
"		SharpDistantLightParam sharpDistant;\n"
"		DistantLightParam distant;\n"
"		LaserLightParam laser;\n"
"	};\n"
"} NotIntersectableLightSource;\n"
"\n"
"typedef struct {\n"
"	Vector v0, v1, v2;\n"
"	Normal geometryN;\n"
"	Normal n0, n1, n2;\n"
"	UV uv0, uv1, uv2;\n"
"	Spectrum rgb0, rgb1, rgb2;\n"
"	float alpha0, alpha1, alpha2;\n"
"	float invTriangleArea, invMeshArea;\n"
"\n"
"	unsigned int materialIndex;\n"
"	unsigned int lightSceneIndex;\n"
"\n"
"	// Used for image map and/or IES map\n"
"	float avarage;\n"
"	unsigned int imageMapIndex;\n"
"} TriangleLightParam;\n"
"\n"
"typedef struct {\n"
"	LightSourceType type;\n"
"	unsigned int lightSceneIndex;\n"
"	unsigned int lightID;\n"
"	int samples;\n"
"	// Type of indirect paths where a light source is visible with a direct hit. It is\n"
"	// an OR of DIFFUSE, GLOSSY and SPECULAR.\n"
"	BSDFEvent visibility;\n"
"	\n"
"	union {\n"
"		NotIntersectableLightSource notIntersectable;\n"
"		TriangleLightParam triangle;\n"
"	};\n"
"} LightSource;\n"
"\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Some macro trick in order to have more readable code\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#define LIGHTS_PARAM_DECL , __global const LightSource* restrict lights, __global const uint* restrict envLightIndices, const uint envLightCount, __global const uint* restrict meshTriLightDefsOffset, __global const float* restrict infiniteLightDistribution, __global const float* restrict lightsDistribution, __global const float* restrict infiniteLightSourcesDistribution MATERIALS_PARAM_DECL\n"
"#define LIGHTS_PARAM , lights, envLightIndices, envLightCount, meshTriLightDefsOffset, infiniteLightDistribution, lightsDistribution, infiniteLightSourcesDistribution MATERIALS_PARAM\n"
"\n"
"#endif\n"
; } }
