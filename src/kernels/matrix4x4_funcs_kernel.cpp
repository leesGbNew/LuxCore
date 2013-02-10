#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_matrix4x4_funcs = 
"#line 2 \"matrix4x4_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2010 by authors (see AUTHORS.txt )                 *\n"
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
"float3 Matrix4x4_ApplyPoint(__global Matrix4x4 *m, const float3 point) {\n"
"	const float4 point4 = (float4)(point.x, point.y, point.z, 1.f);\n"
"\n"
"	const float4 row3 = VLOAD4F(&m->m[3][0]);\n"
"	const float iw = 1.f / dot(row3, point4);\n"
"\n"
"	const float4 row0 = VLOAD4F(&m->m[0][0]);\n"
"	const float4 row1 = VLOAD4F(&m->m[1][0]);\n"
"	const float4 row2 = VLOAD4F(&m->m[2][0]);\n"
"	return (float3)(\n"
"			iw * dot(row0, point4),\n"
"			iw * dot(row1, point4),\n"
"			iw * dot(row2, point4)\n"
"			);\n"
"}\n"
"\n"
"float3 Matrix4x4_ApplyVector(__global Matrix4x4 *m, const float3 vector) {\n"
"	const float3 row0 = VLOAD3F(&m->m[0][0]);\n"
"	const float3 row1 = VLOAD3F(&m->m[1][0]);\n"
"	const float3 row2 = VLOAD3F(&m->m[2][0]);\n"
"	return (float3)(\n"
"			dot(row0, vector),\n"
"			dot(row1, vector),\n"
"			dot(row2, vector)\n"
"			);\n"
"}\n"
; } }
