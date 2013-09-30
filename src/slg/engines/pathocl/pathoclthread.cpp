/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
 *                                                                         *
 *   This file is part of LuxRays.                                         *
 *                                                                         *
 *   LuxRays is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   LuxRays is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   LuxRays website: http://www.luxrender.net                             *
 ***************************************************************************/

#if !defined(LUXRAYS_DISABLE_OPENCL)

#include <boost/lexical_cast.hpp>

#include "luxrays/core/geometry/transform.h"
#include "luxrays/utils/ocl.h"
#include "luxrays/core/oclintersectiondevice.h"
#include "luxrays/kernels/kernels.h"

#include "slg/slg.h"
#include "slg/engines/pathocl/pathocl.h"
#include "slg/kernels/kernels.h"
#include "slg/renderconfig.h"

using namespace std;
using namespace luxrays;
using namespace slg;

#define SOBOL_MAXDEPTH 6

//------------------------------------------------------------------------------
// PathOCLRenderThread
//------------------------------------------------------------------------------

PathOCLRenderThread::PathOCLRenderThread(const u_int index,
		OpenCLIntersectionDevice *device, PathOCLRenderEngine *re) :
		PathOCLBaseRenderThread(index, device, re) {
	gpuTaskStats = NULL;

	initKernel = NULL;
	advancePathsKernel = NULL;

	raysBuff = NULL;
	hitsBuff = NULL;
	tasksBuff = NULL;
	samplesBuff = NULL;
	sampleDataBuff = NULL;
	taskStatsBuff = NULL;
}

PathOCLRenderThread::~PathOCLRenderThread() {
	if (editMode)
		EndEdit(EditActionList());
	if (started)
		Stop();

	delete initKernel;
	delete advancePathsKernel;

	delete[] gpuTaskStats;
}

void PathOCLRenderThread::GetThreadFilmSize(u_int *filmWidth, u_int *filmHeight) {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	const Film *engineFilm = engine->film;
	*filmWidth = engineFilm->GetWidth();
	*filmHeight = engineFilm->GetHeight();
}

string PathOCLRenderThread::AdditionalKernelOptions() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;

	stringstream ss;
	ss.precision(6);
	ss << scientific <<
			" -D PARAM_TASK_COUNT=" << engine->taskCount <<
			" -D PARAM_MAX_PATH_DEPTH=" << engine->maxPathDepth <<
			" -D PARAM_RR_DEPTH=" << engine->rrDepth <<
			" -D PARAM_RR_CAP=" << engine->rrImportanceCap << "f"
			;

	const slg::ocl::Filter *filter = engine->filter;
	switch (filter->type) {
		case slg::ocl::FILTER_NONE:
			ss << " -D PARAM_IMAGE_FILTER_TYPE=0";
			break;
		case slg::ocl::FILTER_BOX:
			ss << " -D PARAM_IMAGE_FILTER_TYPE=1" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_X=" << filter->box.widthX << "f" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_Y=" << filter->box.widthY << "f";
			break;
		case slg::ocl::FILTER_GAUSSIAN:
			ss << " -D PARAM_IMAGE_FILTER_TYPE=2" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_X=" << filter->gaussian.widthX << "f" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_Y=" << filter->gaussian.widthY << "f" <<
					" -D PARAM_IMAGE_FILTER_GAUSSIAN_ALPHA=" << filter->gaussian.alpha << "f";
			break;
		case slg::ocl::FILTER_MITCHELL:
			ss << " -D PARAM_IMAGE_FILTER_TYPE=3" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_X=" << filter->mitchell.widthX << "f" <<
					" -D PARAM_IMAGE_FILTER_WIDTH_Y=" << filter->mitchell.widthY << "f" <<
					" -D PARAM_IMAGE_FILTER_MITCHELL_B=" << filter->mitchell.B << "f" <<
					" -D PARAM_IMAGE_FILTER_MITCHELL_C=" << filter->mitchell.C << "f";
			break;
		default:
			assert (false);
	}

	if (engine->usePixelAtomics)
		ss << " -D PARAM_USE_PIXEL_ATOMICS";

	const slg::ocl::Sampler *sampler = engine->sampler;
	switch (sampler->type) {
		case slg::ocl::RANDOM:
			ss << " -D PARAM_SAMPLER_TYPE=0";
			break;
		case slg::ocl::METROPOLIS:
			ss << " -D PARAM_SAMPLER_TYPE=1" <<
					" -D PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE=" << sampler->metropolis.largeMutationProbability << "f" <<
					" -D PARAM_SAMPLER_METROPOLIS_IMAGE_MUTATION_RANGE=" << sampler->metropolis.imageMutationRange << "f" <<
					" -D PARAM_SAMPLER_METROPOLIS_MAX_CONSECUTIVE_REJECT=" << sampler->metropolis.maxRejects;
			break;
		case slg::ocl::SOBOL:
			ss << " -D PARAM_SAMPLER_TYPE=2" <<
					" -D PARAM_SAMPLER_SOBOL_STARTOFFSET=" << SOBOL_STARTOFFSET <<
					" -D PARAM_SAMPLER_SOBOL_MAXDEPTH=" << max(SOBOL_MAXDEPTH, engine->maxPathDepth);
			break;
		default:
			assert (false);
	}

	return ss.str();
}

string PathOCLRenderThread::AdditionalKernelSources() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;

	string sobolLookupTable;
	if (engine->sampler->type == slg::ocl::SOBOL) {
		// Generate the Sobol vectors
		SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Sobol table size: " << sampleDimensions * SOBOL_BITS);
		u_int *directions = new u_int[sampleDimensions * SOBOL_BITS];

		SobolGenerateDirectionVectors(directions, sampleDimensions);

		stringstream sobolTableSS;
		sobolTableSS << "#line 2 \"Sobol Table in pathoclthread.cpp\"\n";
		sobolTableSS << "__constant uint SOBOL_DIRECTIONS[" << sampleDimensions * SOBOL_BITS << "] = {\n";
		for (u_int i = 0; i < sampleDimensions * SOBOL_BITS; ++i) {
			if (i != 0)
				sobolTableSS << ", ";
			sobolTableSS << directions[i] << "u";
		}
		sobolTableSS << "};\n";

		sobolLookupTable = sobolTableSS.str();

		delete directions;
	}

	stringstream ssKernel;
	ssKernel <<
		sobolLookupTable <<
		slg::ocl::KernelSource_pathocl_datatypes <<
		slg::ocl::KernelSource_pathocl_kernels;

	return ssKernel.str();
}

void PathOCLRenderThread::CompileAdditionalKernels(cl::Program *program) {
	//----------------------------------------------------------------------
	// Init kernel
	//----------------------------------------------------------------------

	CompileKernel(program, &initKernel, &initWorkGroupSize, "Init");

	//----------------------------------------------------------------------
	// AdvancePaths kernel
	//----------------------------------------------------------------------

	CompileKernel(program, &advancePathsKernel, &advancePathsWorkGroupSize, "AdvancePaths");
}

void PathOCLRenderThread::InitGPUTaskBuffer() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	const u_int taskCount = engine->taskCount;
	const u_int triAreaLightCount = engine->compiledScene->triLightDefs.size();
	const bool hasPassThrough = engine->compiledScene->RequiresPassThrough();

	// Add Seed memory size
	size_t gpuTaksSize = sizeof(slg::ocl::Seed);

	// Add PathStateBase memory size
	gpuTaksSize += sizeof(int) + sizeof(u_int) + sizeof(Spectrum);

	// Add PathStateBase.BSDF.HitPoint memory size
	size_t hitPointSize = sizeof(Vector) + sizeof(Point) + sizeof(UV) + 2 * sizeof(Normal);
	if (engine->compiledScene->IsTextureCompiled(HITPOINTCOLOR) ||
			engine->compiledScene->IsTextureCompiled(HITPOINTGREY))
		hitPointSize += sizeof(Spectrum);
	if (engine->compiledScene->IsTextureCompiled(HITPOINTALPHA))
		hitPointSize += sizeof(float);
	if (hasPassThrough)
		hitPointSize += sizeof(float);

	// Add PathStateBase.BSDF memory size
	size_t bsdfSize = hitPointSize;
	// Add PathStateBase.BSDF.materialIndex memory size
	bsdfSize += sizeof(u_int);
	// Add PathStateBase.BSDF.triangleLightSourceIndex memory size
	if (triAreaLightCount > 0)
		bsdfSize += sizeof(u_int);
	// Add PathStateBase.BSDF.Frame memory size
	bsdfSize += sizeof(slg::ocl::Frame);
	gpuTaksSize += bsdfSize;

	// Add PathStateDirectLight memory size
	gpuTaksSize += sizeof(Spectrum) + sizeof(u_int) + 2 * sizeof(BSDFEvent) + sizeof(float);
	// Add PathStateDirectLight.tmpHitPoint memory size
	if (triAreaLightCount > 0)
		gpuTaksSize += hitPointSize;

	// Add PathStateDirectLightPassThrough memory size
	if (hasPassThrough)
		gpuTaksSize += sizeof(float) + bsdfSize;

	SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Size of a GPUTask: " << gpuTaksSize << "bytes");
	AllocOCLBufferRW(&tasksBuff, gpuTaksSize * taskCount, "GPUTask");
}

void PathOCLRenderThread::InitSamplesBuffer() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	const u_int taskCount = engine->taskCount;

	//--------------------------------------------------------------------------
	// SampleResult size
	//--------------------------------------------------------------------------

	// SampleResult.filmX and SampleResult.filmY
	size_t sampleResultSize = 2 * sizeof(float);
	// SampleResult.radiancePerPixelNormalized[PARAM_FILM_RADIANCE_GROUP_COUNT]
	sampleResultSize += sizeof(slg::ocl::Spectrum) * threadFilm->GetRadianceGroupCount();
	if (threadFilm->HasChannel(Film::ALPHA))
		sampleResultSize += sizeof(float);
	if (threadFilm->HasChannel(Film::DEPTH))
		sampleResultSize += sizeof(float);
	if (threadFilm->HasChannel(Film::POSITION))
		sampleResultSize += sizeof(Point);
	if (threadFilm->HasChannel(Film::GEOMETRY_NORMAL))
		sampleResultSize += sizeof(Normal);
	if (threadFilm->HasChannel(Film::SHADING_NORMAL))
		sampleResultSize += sizeof(Normal);
	if (threadFilm->HasChannel(Film::MATERIAL_ID))
		sampleResultSize += sizeof(u_int);
	if (threadFilm->HasChannel(Film::DIRECT_DIFFUSE))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::DIRECT_GLOSSY))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::EMISSION))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::INDIRECT_DIFFUSE))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::INDIRECT_GLOSSY))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::INDIRECT_SPECULAR))
		sampleResultSize += sizeof(Spectrum);
	if (threadFilm->HasChannel(Film::MATERIAL_ID_MASK))
		sampleResultSize += sizeof(float);
	if (threadFilm->HasChannel(Film::DIRECT_SHADOW_MASK))
		sampleResultSize += sizeof(float);
	if (threadFilm->HasChannel(Film::INDIRECT_SHADOW_MASK))
		sampleResultSize += sizeof(float);
	if (threadFilm->HasChannel(Film::UV))
		sampleResultSize += sizeof(UV);
	
	//--------------------------------------------------------------------------
	// Sample size
	//--------------------------------------------------------------------------
	size_t sampleSize = sampleResultSize;

	// Add Sample memory size
	if (engine->sampler->type == slg::ocl::RANDOM) {
		// Nothing to add
	} else if (engine->sampler->type == slg::ocl::METROPOLIS) {
		sampleSize += 2 * sizeof(float) + 5 * sizeof(u_int) + sampleResultSize;		
	} else if (engine->sampler->type == slg::ocl::SOBOL) {
		sampleSize += 2 * sizeof(float) + 2 * sizeof(u_int);
	} else
		throw std::runtime_error("Unknown sampler.type: " + boost::lexical_cast<std::string>(engine->sampler->type));

	SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Size of a Sample: " << sampleSize << "bytes");
	AllocOCLBufferRW(&samplesBuff, sampleSize * taskCount, "Sample");
}

void PathOCLRenderThread::InitSampleDataBuffer() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	Scene *scene = engine->renderConfig->scene;
	const u_int taskCount = engine->taskCount;
	const bool hasPassThrough = engine->compiledScene->RequiresPassThrough();

	const size_t eyePathVertexDimension =
		// IDX_SCREEN_X, IDX_SCREEN_Y
		2 +
		// IDX_EYE_PASSTROUGHT
		(hasPassThrough ? 1 : 0) +
		// IDX_DOF_X, IDX_DOF_Y
		((scene->camera->lensRadius > 0.f) ? 2 : 0);
	const size_t PerPathVertexDimension =
		// IDX_PASSTHROUGH,
		(hasPassThrough ? 1 : 0) +
		// IDX_BSDF_X, IDX_BSDF_Y
		2 +
		// IDX_DIRECTLIGHT_X, IDX_DIRECTLIGHT_Y, IDX_DIRECTLIGHT_Z, IDX_DIRECTLIGHT_W, IDX_DIRECTLIGHT_A
		4 + (hasPassThrough ? 1 : 0) +
		// IDX_RR
		1;
	sampleDimensions = eyePathVertexDimension + PerPathVertexDimension * engine->maxPathDepth;

	size_t uDataSize;
	if ((engine->sampler->type == slg::ocl::RANDOM) ||
			(engine->sampler->type == slg::ocl::SOBOL)) {
		// Only IDX_SCREEN_X, IDX_SCREEN_Y
		uDataSize = sizeof(float) * 2;
		
		if (engine->sampler->type == slg::ocl::SOBOL) {
			// Limit the number of dimension where I use Sobol sequence (after, I switch
			// to Random sampler.
			sampleDimensions = eyePathVertexDimension + PerPathVertexDimension * max(SOBOL_MAXDEPTH, engine->maxPathDepth);
		}
	} else if (engine->sampler->type == slg::ocl::METROPOLIS) {
		// Metropolis needs 2 sets of samples, the current and the proposed mutation
		uDataSize = 2 * sizeof(float) * sampleDimensions;
	} else
		throw std::runtime_error("Unknown sampler.type: " + boost::lexical_cast<std::string>(engine->sampler->type));

	SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Sample dimensions: " << sampleDimensions);
	SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Size of a SampleData: " << uDataSize << "bytes");

	// TOFIX
	AllocOCLBufferRW(&sampleDataBuff, uDataSize * taskCount + 1, "SampleData"); // +1 to avoid METROPOLIS + Intel\AMD OpenCL crash
}

void PathOCLRenderThread::AdditionalInit() {
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	const u_int taskCount = engine->taskCount;

	// In case renderEngine->taskCount has changed
	delete[] gpuTaskStats;
	gpuTaskStats = new slg::ocl::pathocl::GPUTaskStats[taskCount];

	//--------------------------------------------------------------------------
	// Allocate Ray/RayHit buffers
	//--------------------------------------------------------------------------

	AllocOCLBufferRW(&raysBuff, sizeof(Ray) * taskCount, "Ray");
	AllocOCLBufferRW(&hitsBuff, sizeof(RayHit) * taskCount, "RayHit");

	//--------------------------------------------------------------------------
	// Allocate GPU task statistic buffers
	//--------------------------------------------------------------------------

	AllocOCLBufferRW(&taskStatsBuff, sizeof(slg::ocl::pathocl::GPUTaskStats) * taskCount, "GPUTask Stats");

	//--------------------------------------------------------------------------
	// Allocate GPU task buffers
	//--------------------------------------------------------------------------

	InitGPUTaskBuffer();

	//--------------------------------------------------------------------------
	// Allocate sample buffers
	//--------------------------------------------------------------------------

	InitSamplesBuffer();

	//--------------------------------------------------------------------------
	// Allocate sample data buffers
	//--------------------------------------------------------------------------

	InitSampleDataBuffer();
}

void PathOCLRenderThread::SetAdditionalKernelArgs() {
	// Set OpenCL kernel arguments

	// OpenCL kernel setArg() is the only no thread safe function in OpenCL 1.1 so
	// I need to use a mutex here
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	boost::unique_lock<boost::mutex> lock(engine->setKernelArgsMutex);

	//--------------------------------------------------------------------------
	// advancePathsKernel
	//--------------------------------------------------------------------------

	CompiledScene *cscene = engine->compiledScene;

	u_int argIndex = 0;
	advancePathsKernel->setArg(argIndex++, *tasksBuff);
	advancePathsKernel->setArg(argIndex++, *taskStatsBuff);
	advancePathsKernel->setArg(argIndex++, *samplesBuff);
	advancePathsKernel->setArg(argIndex++, *sampleDataBuff);
	advancePathsKernel->setArg(argIndex++, *raysBuff);
	advancePathsKernel->setArg(argIndex++, *hitsBuff);

	// Film parameters
	advancePathsKernel->setArg(argIndex++, threadFilm->GetWidth());
	advancePathsKernel->setArg(argIndex++, threadFilm->GetHeight());
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff.size(); ++i)
		advancePathsKernel->setArg(argIndex++, *(channel_RADIANCE_PER_PIXEL_NORMALIZEDs_Buff[i]));
	if (threadFilm->HasChannel(Film::ALPHA))
		advancePathsKernel->setArg(argIndex++, *channel_ALPHA_Buff);
	if (threadFilm->HasChannel(Film::DEPTH))
		advancePathsKernel->setArg(argIndex++, *channel_DEPTH_Buff);
	if (threadFilm->HasChannel(Film::POSITION))
		advancePathsKernel->setArg(argIndex++, *channel_POSITION_Buff);
	if (threadFilm->HasChannel(Film::GEOMETRY_NORMAL))
		advancePathsKernel->setArg(argIndex++, *channel_GEOMETRY_NORMAL_Buff);
	if (threadFilm->HasChannel(Film::SHADING_NORMAL))
		advancePathsKernel->setArg(argIndex++, *channel_SHADING_NORMAL_Buff);
	if (threadFilm->HasChannel(Film::MATERIAL_ID))
		advancePathsKernel->setArg(argIndex++, *channel_MATERIAL_ID_Buff);
	if (threadFilm->HasChannel(Film::DIRECT_DIFFUSE))
		advancePathsKernel->setArg(argIndex++, *channel_DIRECT_DIFFUSE_Buff);

	if (threadFilm->HasChannel(Film::DIRECT_GLOSSY))
		advancePathsKernel->setArg(argIndex++, *channel_DIRECT_GLOSSY_Buff);
	if (threadFilm->HasChannel(Film::EMISSION))
		advancePathsKernel->setArg(argIndex++, *channel_EMISSION_Buff);
	if (threadFilm->HasChannel(Film::INDIRECT_DIFFUSE))
		advancePathsKernel->setArg(argIndex++, *channel_INDIRECT_DIFFUSE_Buff);
	if (threadFilm->HasChannel(Film::INDIRECT_GLOSSY))
		advancePathsKernel->setArg(argIndex++, *channel_INDIRECT_GLOSSY_Buff);
	if (threadFilm->HasChannel(Film::INDIRECT_SPECULAR))
		advancePathsKernel->setArg(argIndex++, *channel_INDIRECT_SPECULAR_Buff);
	if (threadFilm->HasChannel(Film::MATERIAL_ID_MASK))
		advancePathsKernel->setArg(argIndex++, *channel_MATERIAL_ID_MASK_Buff);
	if (threadFilm->HasChannel(Film::DIRECT_SHADOW_MASK))
		advancePathsKernel->setArg(argIndex++, *channel_DIRECT_SHADOW_MASK_Buff);
	if (threadFilm->HasChannel(Film::INDIRECT_SHADOW_MASK))
		advancePathsKernel->setArg(argIndex++, *channel_INDIRECT_SHADOW_MASK_Buff);
	if (threadFilm->HasChannel(Film::UV))
		advancePathsKernel->setArg(argIndex++, *channel_UV_Buff);

	// Scene parameters
	advancePathsKernel->setArg(argIndex++, cscene->worldBSphere.center.x);
	advancePathsKernel->setArg(argIndex++, cscene->worldBSphere.center.y);
	advancePathsKernel->setArg(argIndex++, cscene->worldBSphere.center.z);
	advancePathsKernel->setArg(argIndex++, cscene->worldBSphere.rad);
	advancePathsKernel->setArg(argIndex++, *materialsBuff);
	advancePathsKernel->setArg(argIndex++, *texturesBuff);
	advancePathsKernel->setArg(argIndex++, *meshMatsBuff);
	advancePathsKernel->setArg(argIndex++, *meshDescsBuff);
	advancePathsKernel->setArg(argIndex++, *vertsBuff);
	if (normalsBuff)
		advancePathsKernel->setArg(argIndex++, *normalsBuff);
	if (uvsBuff)
		advancePathsKernel->setArg(argIndex++, *uvsBuff);
	if (colsBuff)
		advancePathsKernel->setArg(argIndex++, *colsBuff);
	if (alphasBuff)
		advancePathsKernel->setArg(argIndex++, *alphasBuff);
	advancePathsKernel->setArg(argIndex++, *trianglesBuff);
	advancePathsKernel->setArg(argIndex++, *cameraBuff);
	advancePathsKernel->setArg(argIndex++, *lightsDistributionBuff);
	if (infiniteLightBuff) {
		advancePathsKernel->setArg(argIndex++, *infiniteLightBuff);
		advancePathsKernel->setArg(argIndex++, *infiniteLightDistributionBuff);
	}
	if (sunLightBuff)
		advancePathsKernel->setArg(argIndex++, *sunLightBuff);
	if (skyLightBuff)
		advancePathsKernel->setArg(argIndex++, *skyLightBuff);
	if (triLightDefsBuff) {
		advancePathsKernel->setArg(argIndex++, *triLightDefsBuff);
		advancePathsKernel->setArg(argIndex++, *meshTriLightDefsOffsetBuff);
	}
	if (imageMapDescsBuff) {
		advancePathsKernel->setArg(argIndex++, *imageMapDescsBuff);

		for (u_int i = 0; i < imageMapsBuff.size(); ++i)
			advancePathsKernel->setArg(argIndex++, *(imageMapsBuff[i]));
	}

	//--------------------------------------------------------------------------
	// initKernel
	//--------------------------------------------------------------------------

	argIndex = 0;
	initKernel->setArg(argIndex++, engine->seedBase + threadIndex * engine->taskCount);
	initKernel->setArg(argIndex++, *tasksBuff);
	initKernel->setArg(argIndex++, *taskStatsBuff);
	initKernel->setArg(argIndex++, *samplesBuff);
	initKernel->setArg(argIndex++, *sampleDataBuff);
	initKernel->setArg(argIndex++, *raysBuff);
	initKernel->setArg(argIndex++, *cameraBuff);
	initKernel->setArg(argIndex++, threadFilm->GetWidth());
	initKernel->setArg(argIndex++, threadFilm->GetHeight());
}

void PathOCLRenderThread::Stop() {
	PathOCLBaseRenderThread::Stop();

	FreeOCLBuffer(&raysBuff);
	FreeOCLBuffer(&hitsBuff);
	FreeOCLBuffer(&tasksBuff);
	FreeOCLBuffer(&samplesBuff);
	FreeOCLBuffer(&sampleDataBuff);
	FreeOCLBuffer(&taskStatsBuff);
}

void PathOCLRenderThread::RenderThreadImpl() {
	//SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Rendering thread started");

	cl::CommandQueue &oclQueue = intersectionDevice->GetOpenCLQueue();
	PathOCLRenderEngine *engine = (PathOCLRenderEngine *)renderEngine;
	const u_int taskCount = engine->taskCount;

	try {
		//----------------------------------------------------------------------
		// Execute initialization kernels
		//----------------------------------------------------------------------

		// Clear the frame buffer
		const u_int filmPixelCount = threadFilm->GetWidth() * threadFilm->GetHeight();
		oclQueue.enqueueNDRangeKernel(*filmClearKernel, cl::NullRange,
			cl::NDRange(RoundUp<u_int>(filmPixelCount, filmClearWorkGroupSize)),
			cl::NDRange(filmClearWorkGroupSize));

		// Initialize the tasks buffer
		oclQueue.enqueueNDRangeKernel(*initKernel, cl::NullRange,
				cl::NDRange(RoundUp<u_int>(engine->taskCount, initWorkGroupSize)),
				cl::NDRange(initWorkGroupSize));

		//----------------------------------------------------------------------
		// Rendering loop
		//----------------------------------------------------------------------

		// signed int in order to avoid problems with underflows (when computing
		// iterations - 1)
		int iterations = 1;

		double startTime = WallClockTime();
		while (!boost::this_thread::interruption_requested()) {
			/*if (threadIndex == 0)
				cerr << "[DEBUG] =================================";*/

			// Async. transfer of the Film buffers
			TransferFilm(oclQueue);

			// Async. transfer of GPU task statistics
			oclQueue.enqueueReadBuffer(
				*(taskStatsBuff),
				CL_FALSE,
				0,
				sizeof(slg::ocl::pathocl::GPUTaskStats) * taskCount,
				gpuTaskStats);

			// Decide the target refresh time based on screen refresh interval
			const u_int screenRefreshInterval = engine->renderConfig->GetScreenRefreshInterval();
			double targetTime;
			if (screenRefreshInterval <= 100)
				targetTime = 0.025; // 25 ms
			else if (screenRefreshInterval <= 500)
				targetTime = 0.050; // 50 ms
			else
				targetTime = 0.075; // 75 ms

			for (;;) {
				cl::Event event;

				const double t1 = WallClockTime();
				for (int i = 0; i < iterations; ++i) {
					// Trace rays
					intersectionDevice->EnqueueTraceRayBuffer(*raysBuff,
							*(hitsBuff), taskCount, NULL, (i == 0) ? &event : NULL);

					// Advance to next path state
					oclQueue.enqueueNDRangeKernel(*advancePathsKernel, cl::NullRange,
							cl::NDRange(RoundUp<u_int>(taskCount, advancePathsWorkGroupSize)),
							cl::NDRange(advancePathsWorkGroupSize));
				}
				oclQueue.flush();

				event.wait();
				const double t2 = WallClockTime();

				/*if (threadIndex == 0)
					cerr << "[DEBUG] Delta time: " << (t2 - t1) * 1000.0 <<
							"ms (screenRefreshInterval: " << screenRefreshInterval <<
							" iterations: " << iterations << ")\n";*/

				// Check if I have to adjust the number of kernel enqueued
				if (t2 - t1 > targetTime)
					iterations = Max(iterations - 1, 1);
				else
					iterations = Min(iterations + 1, 128);

				// Check if it is time to refresh the screen
				if (((t2 - startTime) * 1000.0 > (double)screenRefreshInterval) ||
						boost::this_thread::interruption_requested())
					break;
			}

			startTime = WallClockTime();
		}

		//SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Rendering thread halted");
	} catch (boost::thread_interrupted) {
		SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Rendering thread halted");
	} catch (cl::Error err) {
		SLG_LOG("[PathOCLRenderThread::" << threadIndex << "] Rendering thread ERROR: " << err.what() <<
				"(" << oclErrorString(err.err()) << ")");
	}

	TransferFilm(oclQueue);
	oclQueue.finish();
}

#endif
