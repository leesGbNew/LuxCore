#include <string>
namespace slg { namespace ocl {
std::string KernelSource_material_types = 
"#line 2 \"material_types.cl\"\n"
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
"	MATTE, MIRROR, GLASS, ARCHGLASS, MIX, NULLMAT, MATTETRANSLUCENT,\n"
"	GLOSSY2, METAL2, ROUGHGLASS, VELVET, CLOTH, CARPAINT, ROUGHMATTE,\n"
"	ROUGHMATTETRANSLUCENT, GLOSSYTRANSLUCENT, GLOSSYCOATING,\n"
"			\n"
"	// Volumes\n"
"	HOMOGENEOUS_VOL, CLEAR_VOL, HETEROGENEOUS_VOL\n"
"} MaterialType;\n"
"\n"
"typedef struct {\n"
"    unsigned int kdTexIndex;\n"
"} MatteParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int kdTexIndex;\n"
"    unsigned int sigmaTexIndex;\n"
"} RoughMatteParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"} MirrorParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int exteriorIorTexIndex, interiorIorTexIndex;\n"
"} GlassParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int expTexIndex;\n"
"} MetalParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int exteriorIorTexIndex, interiorIorTexIndex;\n"
"} ArchGlassParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int matAIndex, matBIndex;\n"
"	unsigned int mixFactorTexIndex;\n"
"} MixParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"} MatteTranslucentParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int sigmaTexIndex;\n"
"} RoughMatteTranslucentParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int kdTexIndex;\n"
"	unsigned int ksTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"	unsigned int kaTexIndex;\n"
"	unsigned int depthTexIndex;\n"
"	unsigned int indexTexIndex;\n"
"	int multibounce;\n"
"} Glossy2Param;\n"
"\n"
"typedef struct {\n"
"    unsigned int fresnelTexIndex;\n"
"    unsigned int nTexIndex;\n"
"	unsigned int kTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"} Metal2Param;\n"
"\n"
"typedef struct {\n"
"    unsigned int krTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int exteriorIorTexIndex, interiorIorTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"} RoughGlassParam;\n"
"\n"
"typedef struct {\n"
"    unsigned int kdTexIndex;\n"
"	unsigned int p1TexIndex;\n"
"	unsigned int p2TexIndex;\n"
"	unsigned int p3TexIndex;\n"
"	unsigned int thicknessTexIndex;\n"
"} VelvetParam;\n"
"\n"
"typedef enum {\n"
"	DENIM, SILKSHANTUNG, SILKCHARMEUSE, COTTONTWILL, WOOLGARBARDINE, POLYESTER\n"
"} ClothPreset;\n"
"\n"
"typedef enum {\n"
"	WARP, WEFT\n"
"} YarnType;\n"
"\n"
"// Data structure describing the properties of a single yarn\n"
"typedef struct {\n"
"	// Fiber twist angle\n"
"	float psi;\n"
"	// Maximum inclination angle\n"
"	float umax;\n"
"	// Spine curvature\n"
"	float kappa;\n"
"	// Width of segment rectangle\n"
"	float width;\n"
"	// Length of segment rectangle\n"
"	float length;\n"
"	/*! u coordinate of the yarn segment center,\n"
"	 * assumes that the tile covers 0 <= u, v <= 1.\n"
"	 * (0, 0) is lower left corner of the weave pattern\n"
"	 */\n"
"	float centerU;\n"
"	// v coordinate of the yarn segment center\n"
"	float centerV;\n"
"\n"
"	// Weft/Warp flag\n"
"	YarnType yarn_type;\n"
"} Yarn;\n"
"\n"
"typedef struct  {\n"
"	// Size of the weave pattern\n"
"	unsigned int tileWidth, tileHeight;\n"
"	\n"
"	// Uniform scattering parameter\n"
"	float alpha;\n"
"	// Forward scattering parameter\n"
"	float beta;\n"
"	// Filament smoothing\n"
"	float ss;\n"
"	// Highlight width\n"
"	float hWidth;\n"
"	// Combined area taken up by the warp & weft\n"
"	float warpArea, weftArea;\n"
"\n"
"	// Noise-related parameters\n"
"	float fineness;\n"
"\n"
"	float dWarpUmaxOverDWarp;\n"
"	float dWarpUmaxOverDWeft;\n"
"	float dWeftUmaxOverDWarp;\n"
"	float dWeftUmaxOverDWeft;\n"
"	float period;\n"
"} WeaveConfig;\n"
"\n"
"typedef struct {\n"
"    ClothPreset Preset;\n"
"	unsigned int Weft_KdIndex;\n"
"	unsigned int Weft_KsIndex;\n"
"	unsigned int Warp_KdIndex;\n"
"	unsigned int Warp_KsIndex;\n"
"	float Repeat_U;\n"
"	float Repeat_V;\n"
"	float specularNormalization;\n"
"} ClothParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int KdTexIndex;\n"
"	unsigned int Ks1TexIndex;\n"
"	unsigned int Ks2TexIndex;\n"
"	unsigned int Ks3TexIndex;\n"
"	unsigned int M1TexIndex;\n"
"	unsigned int M2TexIndex;\n"
"	unsigned int M3TexIndex;\n"
"	unsigned int R1TexIndex;\n"
"	unsigned int R2TexIndex;\n"
"	unsigned int R3TexIndex;\n"
"	unsigned int KaTexIndex;\n"
"	unsigned int depthTexIndex;\n"
"} CarPaintParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int kdTexIndex;\n"
"	unsigned int ktTexIndex;\n"
"	unsigned int ksTexIndex;\n"
"	unsigned int ksbfTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nubfTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"	unsigned int nvbfTexIndex;\n"
"	unsigned int kaTexIndex;\n"
"	unsigned int kabfTexIndex;\n"
"	unsigned int depthTexIndex;\n"
"	unsigned int depthbfTexIndex;\n"
"	unsigned int indexTexIndex;\n"
"	unsigned int indexbfTexIndex;\n"
"	int multibounce;\n"
"	int multibouncebf;\n"
"} GlossyTranslucentParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int matBaseIndex;\n"
"	unsigned int ksTexIndex;\n"
"	unsigned int nuTexIndex;\n"
"	unsigned int nvTexIndex;\n"
"	unsigned int kaTexIndex;\n"
"	unsigned int depthTexIndex;\n"
"	unsigned int indexTexIndex;\n"
"	int multibounce;\n"
"} GlossyCoatingParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int sigmaATexIndex;\n"
"} ClearVolumeParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int sigmaATexIndex;\n"
"	unsigned int sigmaSTexIndex;\n"
"	unsigned int gTexIndex;\n"
"	int multiScattering;\n"
"} HomogenousVolumeParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int sigmaATexIndex;\n"
"	unsigned int sigmaSTexIndex;\n"
"	unsigned int gTexIndex;\n"
"	float stepSize;\n"
"	unsigned int maxStepsCount;\n"
"	int multiScattering;\n"
"} HeterogenousVolumeParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int iorTexIndex;\n"
"	// This is a different kind of emission texture from the one in\n"
"	// Material class (i.e. is not sampled by direct light).\n"
"	unsigned int volumeEmissionTexIndex;\n"
"	unsigned int volumeLightID;\n"
"	int priority;\n"
"\n"
"	union {\n"
"		ClearVolumeParam clear;\n"
"		HomogenousVolumeParam homogenous;\n"
"		HeterogenousVolumeParam heterogenous;\n"
"	};\n"
"} VolumeParam;\n"
"\n"
"typedef struct {\n"
"	MaterialType type;\n"
"	unsigned int matID, lightID;\n"
"	float bumpSampleDistance;\n"
"	Spectrum emittedFactor;\n"
"	float emittedCosThetaMax;\n"
"	int usePrimitiveArea;\n"
"	unsigned int transpTexIndex, emitTexIndex, bumpTexIndex;\n"
"	int samples;\n"
"	// Type of indirect paths where a light source is visible with a direct hit. It is\n"
"	// an OR of DIFFUSE, GLOSSY and SPECULAR.\n"
"	BSDFEvent visibility;\n"
"	unsigned int interiorVolumeIndex, exteriorVolumeIndex;\n"
"	int isShadowCatcher, isShadowCatcherOnlyInfiniteLights;\n"
"\n"
"	union {\n"
"		MatteParam matte;\n"
"		RoughMatteParam roughmatte;\n"
"		MirrorParam mirror;\n"
"		GlassParam glass;\n"
"		MetalParam metal;\n"
"		ArchGlassParam archglass;\n"
"		MixParam mix;\n"
"		// NULLMAT has no parameters\n"
"		MatteTranslucentParam matteTranslucent;\n"
"		RoughMatteTranslucentParam roughmatteTranslucent;\n"
"		Glossy2Param glossy2;\n"
"		Metal2Param metal2;\n"
"		RoughGlassParam roughglass;\n"
"		VelvetParam velvet;\n"
"        ClothParam cloth;\n"
"		CarPaintParam carpaint;\n"
"		GlossyTranslucentParam glossytranslucent;\n"
"		GlossyCoatingParam glossycoating;\n"
"		VolumeParam volume;\n"
"	};\n"
"} Material;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Some macro trick in order to have more readable code\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(SLG_OPENCL_KERNEL)\n"
"\n"
"#define MATERIALS_PARAM_DECL , __global const Material* restrict mats TEXTURES_PARAM_DECL\n"
"#define MATERIALS_PARAM , mats TEXTURES_PARAM\n"
"\n"
"#endif\n"
; } }
