#include <string>
namespace slg { namespace ocl {
std::string KernelSource_biaspathocl_kernels = 
"#line 2 \"biaspatchocl_kernels.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *\n"
" *                                                                         *\n"
" *   This file is part of LuxRays.                                         *\n"
" *                                                                         *\n"
" *   LuxRays is free software; you can redistribute it and/or modify       *\n"
" *   it under the terms of the GNU General Public License as published by  *\n"
" *   the Free Software Foundation; either version 3 of the License, or     *\n"
" *   (at your option) any later version.                                   *\n"
" *                                                                         *\n"
" *   LuxRays is distributed in the hope that it will be useful,            *\n"
" *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
" *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
" *   GNU General Public License for more details.                          *\n"
" *                                                                         *\n"
" *   You should have received a copy of the GNU General Public License     *\n"
" *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *\n"
" *                                                                         *\n"
" *   LuxRays website: http://www.luxrender.net                             *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Init Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"void GenerateCameraPath(\n"
"		__global GPUTask *task,\n"
"		__global Sample *sample,\n"
"		__global float *sampleData,\n"
"		__global Camera *camera,\n"
"		const uint filmWidth,\n"
"		const uint filmHeight,\n"
"		__global Ray *ray,\n"
"		Seed *seed) {\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"\n"
"	const float scrSampleX = sampleData[IDX_SCREEN_X];\n"
"	const float scrSampleY = sampleData[IDX_SCREEN_Y];\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Rnd_FloatValue(seed);\n"
"	const float dofSampleY = Rnd_FloatValue(seed);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Rnd_FloatValue(seed);\n"
"#endif\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 1)\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"	const float scrSampleX = Sampler_GetSamplePath(IDX_SCREEN_X);\n"
"	const float scrSampleY = Sampler_GetSamplePath(IDX_SCREEN_Y);\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Sampler_GetSamplePath(IDX_DOF_X);\n"
"	const float dofSampleY = Sampler_GetSamplePath(IDX_DOF_Y);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Sampler_GetSamplePath(IDX_EYE_PASSTHROUGH);\n"
"#endif\n"
"#endif\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 2)\n"
"	const float scrSampleX = sampleData[IDX_SCREEN_X];\n"
"	const float scrSampleY = sampleData[IDX_SCREEN_Y];\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	const float dofSampleX = Sampler_GetSamplePath(IDX_DOF_X);\n"
"	const float dofSampleY = Sampler_GetSamplePath(IDX_DOF_Y);\n"
"#endif\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	const float eyePassthrough = Sampler_GetSamplePath(IDX_EYE_PASSTHROUGH);\n"
"#endif\n"
"#endif\n"
"\n"
"	Camera_GenerateRay(camera, filmWidth, filmHeight, ray, scrSampleX, scrSampleY\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"			, dofSampleX, dofSampleY\n"
"#endif\n"
"			);\n"
"\n"
"	// Initialize the path state\n"
"	task->pathStateBase.state = RT_NEXT_VERTEX;\n"
"	task->pathStateBase.depth = 1;\n"
"	VSTORE3F(WHITE, &task->pathStateBase.throughput.r);\n"
"	task->directLightState.pathBSDFEvent = NONE;\n"
"	task->directLightState.lastBSDFEvent = SPECULAR; // SPECULAR is required to avoid MIS\n"
"	task->directLightState.lastPdfW = 1.f;\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	// This is a bit tricky. I store the passThroughEvent in the BSDF\n"
"	// before of the initialization because it can be use during the\n"
"	// tracing of next path vertex ray.\n"
"\n"
"	task->pathStateBase.bsdf.hitPoint.passThroughEvent = eyePassthrough;\n"
"#endif\n"
"}\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void Init(\n"
"		uint seedBase,\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global Sample *samples,\n"
"		__global float *samplesData,\n"
"		__global Ray *rays,\n"
"		__global Camera *camera,\n"
"		const uint filmWidth,\n"
"		const uint filmHeight\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	// Initialize the task\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// Initialize random number generator\n"
"	Seed seed;\n"
"	Rnd_Init(seedBase + gid, &seed);\n"
"\n"
"	// Initialize the sample and path\n"
"	__global Sample *sample = &samples[gid];\n"
"	__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n"
"	Sampler_Init(&seed, sample, sampleData, filmWidth, filmHeight);\n"
"	GenerateCameraPath(task, sample, sampleData, camera, filmWidth, filmHeight, &rays[gid], &seed);\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed.s1;\n"
"	task->seed.s2 = seed.s2;\n"
"	task->seed.s3 = seed.s3;\n"
"\n"
"	__global GPUTaskStats *taskStat = &taskStats[gid];\n"
"	taskStat->sampleCount = 0;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// BiasAdvancePaths Kernel\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths(\n"
"		__global GPUTask *tasks,\n"
"		__global GPUTaskStats *taskStats,\n"
"		__global Sample *samples,\n"
"		__global float *samplesData,\n"
"		__global Ray *rays,\n"
"		__global RayHit *rayHits,\n"
"		// Film parameters\n"
"		const uint filmWidth, const uint filmHeight\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"		, __global float *filmRadianceGroup0\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"		, __global float *filmRadianceGroup1\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"		, __global float *filmRadianceGroup2\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"		, __global float *filmRadianceGroup3\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"		, __global float *filmRadianceGroup4\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"		, __global float *filmRadianceGroup5\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"		, __global float *filmRadianceGroup6\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"		, __global float *filmRadianceGroup7\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n"
"		, __global float *filmAlpha\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n"
"		, __global float *filmDepth\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n"
"		, __global float *filmPosition\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n"
"		, __global float *filmGeometryNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n"
"		, __global float *filmShadingNormal\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n"
"		, __global uint *filmMaterialID\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"		, __global float *filmDirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"		, __global float *filmDirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"		, __global float *filmEmission\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"		, __global float *filmIndirectDiffuse\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"		, __global float *filmIndirectGlossy\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"		, __global float *filmIndirectSpecular\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK)\n"
"		, __global float *filmMaterialIDMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"		, __global float *filmDirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"		, __global float *filmIndirectShadowMask\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n"
"		, __global float *filmUV\n"
"#endif\n"
"		,\n"
"		// Scene parameters\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"		__global Material *mats,\n"
"		__global Texture *texs,\n"
"		__global uint *meshMats,\n"
"		__global Mesh *meshDescs,\n"
"		__global Point *vertices,\n"
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n"
"		__global Vector *vertNormals,\n"
"#endif\n"
"#if defined(PARAM_HAS_UVS_BUFFER)\n"
"		__global UV *vertUVs,\n"
"#endif\n"
"#if defined(PARAM_HAS_COLS_BUFFER)\n"
"		__global Spectrum *vertCols,\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n"
"		__global float *vertAlphas,\n"
"#endif\n"
"		__global Triangle *triangles,\n"
"		__global Camera *camera,\n"
"		__global float *lightsDistribution\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		, __global InfiniteLight *infiniteLight\n"
"		, __global float *infiniteLightDistribution\n"
"#endif\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"		, __global SunLight *sunLight\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"		, __global SkyLight *skyLight\n"
"#endif\n"
"#if (PARAM_DL_LIGHT_COUNT > 0)\n"
"		, __global TriangleLight *triLightDefs\n"
"		, __global uint *meshTriLightDefsOffset\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"		, __global ImageMap *imageMapDescs, __global float *imageMapBuff0\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"		, __global float *imageMapBuff1\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"		, __global float *imageMapBuff2\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"		, __global float *imageMapBuff3\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"		, __global float *imageMapBuff4\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"		, __global float *imageMapBuff5\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"		, __global float *imageMapBuff6\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"		, __global float *imageMapBuff7\n"
"#endif\n"
"		ACCELERATOR_INTERSECT_PARAM_DECL\n"
"		) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= PARAM_TASK_COUNT)\n"
"		return;\n"
"\n"
"	__global GPUTask *task = &tasks[gid];\n"
"\n"
"	// Read the path state\n"
"	PathState pathState = task->pathStateBase.state;\n"
"	const uint depth = task->pathStateBase.depth;\n"
"	__global BSDF *bsdf = &task->pathStateBase.bsdf;\n"
"\n"
"	__global Sample *sample = &samples[gid];\n"
"	__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n"
"	__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n"
"#if (PARAM_SAMPLER_TYPE != 0)\n"
"	// Used by Sampler_GetSamplePathVertex() macro\n"
"	__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n"
"			sample, sampleDataPathBase, depth);\n"
"#endif\n"
"\n"
"	// Read the seed\n"
"	Seed seedValue;\n"
"	seedValue.s1 = task->seed.s1;\n"
"	seedValue.s2 = task->seed.s2;\n"
"	seedValue.s3 = task->seed.s3;\n"
"	// This trick is required by Sampler_GetSample() macro\n"
"	Seed *seed = &seedValue;\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Initialize image maps page pointer table\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"	__global float *imageMapBuff[PARAM_IMAGEMAPS_COUNT];\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_0)\n"
"	imageMapBuff[0] = imageMapBuff0;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_1)\n"
"	imageMapBuff[1] = imageMapBuff1;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_2)\n"
"	imageMapBuff[2] = imageMapBuff2;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_3)\n"
"	imageMapBuff[3] = imageMapBuff3;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_4)\n"
"	imageMapBuff[4] = imageMapBuff4;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_5)\n"
"	imageMapBuff[5] = imageMapBuff5;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_6)\n"
"	imageMapBuff[6] = imageMapBuff6;\n"
"#endif\n"
"#if defined(PARAM_IMAGEMAPS_PAGE_7)\n"
"	imageMapBuff[7] = imageMapBuff7;\n"
"#endif\n"
"#endif\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"	// Render a sample\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	__global Ray *ray = &rays[gid];\n"
"	__global RayHit *rayHit = &rayHits[gid];\n"
"\n"
"	do {\n"
"		//----------------------------------------------------------------------\n"
"		// Ray trace step\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		{\n"
"			Ray r = *ray;\n"
"			RayHit rh = *rayHit;\n"
"			Accelerator_Intersect(&r, &rh\n"
"				ACCELERATOR_INTERSECT_PARAM);\n"
"			*rayHit = rh;\n"
"			*ray = r;\n"
"		}\n"
"\n"
"		//----------------------------------------------------------------------\n"
"		// Advance the finite state machine step\n"
"		//----------------------------------------------------------------------\n"
"\n"
"		const bool rayMiss = (rayHit->meshIndex == NULL_INDEX);\n"
"\n"
"		//--------------------------------------------------------------------------\n"
"		// Evaluation of the Path finite state machine.\n"
"		//\n"
"		// From: RT_NEXT_VERTEX\n"
"		// To: SPLAT_SAMPLE or GENERATE_DL_RAY\n"
"		//--------------------------------------------------------------------------\n"
"\n"
"		if (pathState == RT_NEXT_VERTEX) {\n"
"			if (!rayMiss) {\n"
"				//------------------------------------------------------------------\n"
"				// Something was hit\n"
"				//------------------------------------------------------------------\n"
"\n"
"				VADD3F(&sample->result.radiancePerPixelNormalized[0].r, BLACK);\n"
"				pathState = SPLAT_SAMPLE;\n"
"			} else {\n"
"				//------------------------------------------------------------------\n"
"				// Nothing was hit, add environmental lights radiance\n"
"				//------------------------------------------------------------------\n"
"\n"
"				VADD3F(&sample->result.radiancePerPixelNormalized[0].r, WHITE);\n"
"				pathState = SPLAT_SAMPLE;\n"
"			}\n"
"		}\n"
"\n"
"		//--------------------------------------------------------------------------\n"
"		// Evaluation of the Path finite state machine.\n"
"		//\n"
"		// From: SPLAT_SAMPLE\n"
"		// To: RT_NEXT_VERTEX\n"
"		//--------------------------------------------------------------------------\n"
"\n"
"		if (pathState == SPLAT_SAMPLE) {	\n"
"			// Initialize Film radiance group pointer table\n"
"			__global float *filmRadianceGroup[PARAM_FILM_RADIANCE_GROUP_COUNT];\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"			filmRadianceGroup[0] = filmRadianceGroup0;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"			filmRadianceGroup[1] = filmRadianceGroup1;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"			filmRadianceGroup[2] = filmRadianceGroup2;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"			filmRadianceGroup[3] = filmRadianceGroup3;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"			filmRadianceGroup[3] = filmRadianceGroup4;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"			filmRadianceGroup[3] = filmRadianceGroup5;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"			filmRadianceGroup[3] = filmRadianceGroup6;\n"
"	#endif\n"
"	#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"			filmRadianceGroup[3] = filmRadianceGroup7;\n"
"	#endif\n"
"\n"
"			Sampler_NextSample(seed, sample, sampleData\n"
"					FILM_PARAM);\n"
"			taskStats[gid].sampleCount += 1;\n"
"\n"
"			pathState = DONE;\n"
"		} else {\n"
"			// Save the state\n"
"			task->pathStateBase.state = pathState;\n"
"		}\n"
"	} while (pathState != DONE);\n"
"		\n"
"\n"
"	//--------------------------------------------------------------------------\n"
"\n"
"	// Save the seed\n"
"	task->seed.s1 = seed->s1;\n"
"	task->seed.s2 = seed->s2;\n"
"	task->seed.s3 = seed->s3;\n"
"}\n"
; } }
