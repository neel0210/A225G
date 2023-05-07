/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include "mtk_cpufreq_config.h"

#define NR_FREQ		16
#define ARRAY_COL_SIZE	4

static unsigned int fyTbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2351, 96, 1, 1 },	/* L */
	{ 2269, 89, 1, 1 },
	{ 2186, 82, 1, 1 },
	{ 2103, 75, 1, 1 },
	{ 2033, 69, 1, 1 },
	{ 1962, 63, 1, 1 },
	{ 1891, 57, 1, 1 },
	{ 1820, 51, 1, 1 },
	{ 1750, 45, 1, 1 },
	{ 1617, 40, 1, 1 },
	{ 1484, 35, 2, 1 },
	{ 1351, 30, 2, 1 },
	{ 1218, 25, 2, 1 },
	{ 1085, 20, 2, 1 },
	{  979, 16, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1701, 96, 1, 1 },	/* LL */
	{ 1612, 89, 1, 1 },
	{ 1522, 82, 1, 1 },
	{ 1433, 75, 2, 1 },
	{ 1356, 69, 2, 1 },
	{ 1279, 63, 2, 1 },
	{ 1203, 57, 2, 1 },
	{ 1126, 51, 2, 1 },
	{ 1050, 45, 2, 1 },
	{  948, 40, 2, 1 },
	{  846, 35, 2, 1 },
	{  745, 30, 4, 1 },
	{  643, 25, 4, 1 },
	{  542, 20, 4, 1 },
	{  460, 16, 4, 2 },
	{  400, 13, 4, 2 },

	{ 1051, 96, 2, 1 },	/* CCI */
	{ 1006, 89, 2, 1 },
	{  962, 82, 2, 1 },
	{  917, 75, 2, 1 },
	{  878, 69, 2, 1 },
	{  840, 63, 2, 1 },
	{  801, 57, 2, 1 },
	{  763, 51, 2, 1 },
	{  724, 45, 4, 1 },
	{  658, 40, 4, 1 },
	{  592, 35, 4, 1 },
	{  525, 30, 4, 1 },
	{  459, 25, 4, 2 },
	{  392, 20, 4, 2 },
	{  339, 16, 4, 2 },
	{  300, 13, 4, 2 },
};

static unsigned int sbTbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2501, 96, 1, 1 },	/* L */
	{ 2397, 89, 1, 1 },
	{ 2294, 82, 1, 1 },
	{ 2191, 75, 1, 1 },
	{ 2103, 69, 1, 1 },
	{ 2015, 63, 1, 1 },
	{ 1926, 57, 1, 1 },
	{ 1838, 51, 1, 1 },
	{ 1750, 45, 1, 1 },
	{ 1617, 40, 1, 1 },
	{ 1484, 35, 2, 1 },
	{ 1351, 30, 2, 1 },
	{ 1218, 25, 2, 1 },
	{ 1085, 20, 2, 1 },
	{  979, 16, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1901, 96, 1, 1 },	/* LL */
	{ 1784, 89, 1, 1 },
	{ 1667, 82, 1, 1 },
	{ 1550, 75, 1, 1 },
	{ 1450, 69, 2, 1 },
	{ 1350, 63, 2, 1 },
	{ 1250, 57, 2, 1 },
	{ 1150, 51, 2, 1 },
	{ 1050, 45, 2, 1 },
	{  948, 40, 2, 1 },
	{  846, 35, 2, 1 },
	{  745, 30, 4, 1 },
	{  643, 25, 4, 1 },
	{  542, 20, 4, 1 },
	{  460, 16, 4, 2 },
	{  400, 13, 4, 2 },

	{ 1101, 96, 2, 1 },	/* CCI */
	{ 1049, 89, 2, 1 },
	{  998, 82, 2, 1 },
	{  946, 75, 2, 1 },
	{  902, 69, 2, 1 },
	{  857, 63, 2, 1 },
	{  813, 57, 2, 1 },
	{  769, 51, 2, 1 },
	{  724, 45, 4, 1 },
	{  658, 40, 4, 1 },
	{  592, 35, 4, 1 },
	{  525, 30, 4, 1 },
	{  459, 25, 4, 2 },
	{  392, 20, 4, 2 },
	{  339, 16, 4, 2 },
	{  300, 13, 4, 2 },
};

static unsigned int c65tTbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2501, 96, 1, 1 },	/* L */
	{ 2383, 88, 1, 1 },
	{ 2280, 81, 1, 1 },
	{ 2191, 75, 1, 1 },
	{ 2103, 69, 1, 1 },
	{ 2015, 63, 1, 1 },
	{ 1926, 57, 1, 1 },
	{ 1838, 51, 1, 1 },
	{ 1750, 45, 1, 1 },
	{ 1617, 40, 1, 1 },
	{ 1484, 35, 2, 1 },
	{ 1351, 30, 2, 1 },
	{ 1218, 25, 2, 1 },
	{ 1085, 20, 2, 1 },
	{  979, 16, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1800, 96, 1, 1 },	/* LL */
	{ 1682, 88, 1, 1 },
	{ 1579, 81, 1, 1 },
	{ 1491, 75, 2, 1 },
	{ 1402, 69, 2, 1 },
	{ 1314, 63, 2, 1 },
	{ 1226, 57, 2, 1 },
	{ 1138, 51, 2, 1 },
	{ 1050, 45, 2, 1 },
	{  948, 40, 2, 1 },
	{  846, 35, 2, 1 },
	{  745, 30, 4, 1 },
	{  643, 25, 4, 1 },
	{  542, 20, 4, 1 },
	{  501, 18, 4, 1 },
	{  400, 13, 4, 2 },

	{ 1101, 96, 2, 1 },	/* CCI */
	{ 1042, 88, 2, 1 },
	{  990, 81, 2, 1 },
	{  946, 75, 2, 1 },
	{  902, 69, 2, 1 },
	{  857, 63, 2, 1 },
	{  813, 57, 2, 1 },
	{  769, 51, 2, 1 },
	{  724, 45, 4, 1 },
	{  658, 40, 4, 1 },
	{  592, 35, 4, 1 },
	{  525, 30, 4, 1 },
	{  459, 25, 4, 2 },
	{  392, 20, 4, 2 },
	{  339, 16, 4, 2 },
	{  300, 13, 4, 2 },
};

static unsigned int c65Tbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2301, 96, 1, 1 },	/* L */
	{ 2215, 88, 1, 1 },
	{ 2139, 81, 1, 1 },
	{ 2074, 75, 1, 1 },
	{ 2009, 69, 1, 1 },
	{ 1944, 63, 1, 1 },
	{ 1879, 57, 1, 1 },
	{ 1814, 51, 1, 1 },
	{ 1750, 45, 1, 1 },
	{ 1617, 40, 1, 1 },
	{ 1484, 35, 2, 1 },
	{ 1351, 30, 2, 1 },
	{ 1218, 25, 2, 1 },
	{ 1085, 20, 2, 1 },
	{  979, 16, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1800, 96, 1, 1 },	/* LL */
	{ 1682, 88, 1, 1 },
	{ 1579, 81, 1, 1 },
	{ 1491, 75, 2, 1 },
	{ 1402, 69, 2, 1 },
	{ 1314, 63, 2, 1 },
	{ 1226, 57, 2, 1 },
	{ 1138, 51, 2, 1 },
	{ 1050, 45, 2, 1 },
	{  948, 40, 2, 1 },
	{  846, 35, 2, 1 },
	{  745, 30, 4, 1 },
	{  643, 25, 4, 1 },
	{  542, 20, 4, 1 },
	{  501, 18, 4, 1 },
	{  400, 13, 4, 2 },

	{ 1051, 96, 2, 1 },	/* CCI */
	{ 1000, 88, 2, 1 },
	{  955, 81, 2, 1 },
	{  917, 75, 2, 1 },
	{  878, 69, 2, 1 },
	{  840, 63, 2, 1 },
	{  801, 57, 2, 1 },
	{  763, 51, 2, 1 },
	{  724, 45, 4, 1 },
	{  658, 40, 4, 1 },
	{  592, 35, 4, 1 },
	{  525, 30, 4, 1 },
	{  459, 25, 4, 2 },
	{  392, 20, 4, 2 },
	{  339, 16, 4, 2 },
	{  300, 13, 4, 2 },
};

static unsigned int c62Tbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2001, 96, 1, 1 },	/* L */
	{ 1961, 88, 1, 1 },
	{ 1927, 81, 1, 1 },
	{ 1897, 75, 1, 1 },
	{ 1868, 69, 1, 1 },
	{ 1838, 63, 1, 1 },
	{ 1809, 57, 1, 1 },
	{ 1779, 51, 1, 1 },
	{ 1750, 45, 1, 1 },
	{ 1617, 40, 1, 1 },
	{ 1484, 35, 2, 1 },
	{ 1351, 30, 2, 1 },
	{ 1218, 25, 2, 1 },
	{ 1085, 20, 2, 1 },
	{  979, 16, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1500, 96, 2, 1 },	/* LL */
	{ 1429, 88, 2, 1 },
	{ 1367, 81, 2, 1 },
	{ 1314, 75, 2, 1 },
	{ 1261, 69, 2, 1 },
	{ 1208, 63, 2, 1 },
	{ 1155, 57, 2, 1 },
	{ 1102, 51, 2, 1 },
	{ 1050, 45, 2, 1 },
	{  948, 40, 2, 1 },
	{  846, 35, 2, 1 },
	{  745, 30, 4, 1 },
	{  643, 25, 4, 1 },
	{  542, 20, 4, 1 },
	{  501, 18, 4, 1 },
	{  400, 13, 4, 2 },

	{ 1048, 96, 2, 1 },	/* CCI */
	{  997, 88, 2, 1 },
	{  953, 81, 2, 1 },
	{  915, 75, 2, 1 },
	{  877, 69, 2, 1 },
	{  839, 63, 2, 1 },
	{  801, 57, 2, 1 },
	{  763, 51, 2, 1 },
	{  724, 45, 4, 1 },
	{  658, 40, 4, 1 },
	{  592, 35, 4, 1 },
	{  525, 30, 4, 1 },
	{  459, 25, 4, 2 },
	{  392, 20, 4, 2 },
	{  339, 16, 4, 2 },
	{  300, 13, 4, 2 },
};

static unsigned int c62lyTbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	{ 2001, 96, 1, 1 },	/* L */
	{ 1961, 90, 1, 1 },
	{ 1927, 84, 1, 1 },
	{ 1897, 79, 1, 1 },
	{ 1868, 74, 1, 1 },
	{ 1838, 69, 1, 1 },
	{ 1809, 64, 1, 1 },
	{ 1779, 59, 1, 1 },
	{ 1750, 53, 1, 1 },
	{ 1617, 47, 1, 1 },
	{ 1484, 45, 2, 1 },
	{ 1351, 35, 2, 1 },
	{ 1218, 28, 2, 1 },
	{ 1085, 22, 2, 1 },
	{  979, 17, 2, 1 },
	{  900, 13, 2, 1 },

	{ 1500, 96, 2, 1 },	/* LL */
	{ 1429, 90, 2, 1 },
	{ 1367, 84, 2, 1 },
	{ 1314, 79, 2, 1 },
	{ 1261, 74, 2, 1 },
	{ 1208, 69, 2, 1 },
	{ 1155, 64, 2, 1 },
	{ 1102, 59, 2, 1 },
	{ 1050, 53, 2, 1 },
	{  948, 47, 2, 1 },
	{  846, 45, 2, 1 },
	{  745, 35, 4, 1 },
	{  643, 28, 4, 1 },
	{  542, 22, 4, 1 },
	{  501, 20, 4, 1 },
	{  400, 13, 4, 2 },

	{ 1048, 96, 2, 1 },	/* CCI */
	{  997, 90, 2, 1 },
	{  953, 84, 2, 1 },
	{  915, 79, 2, 1 },
	{  877, 74, 2, 1 },
	{  839, 69, 2, 1 },
	{  801, 64, 2, 1 },
	{  763, 59, 2, 1 },
	{  724, 53, 4, 1 },
	{  658, 47, 4, 1 },
	{  592, 45, 4, 1 },
	{  525, 35, 4, 1 },
	{  459, 28, 4, 2 },
	{  392, 22, 4, 2 },
	{  339, 17, 4, 2 },
	{  300, 13, 4, 2 },
};

unsigned int *xrecordTbl[NUM_CPU_LEVEL] = {	/* v1.2 */
	[CPU_LEVEL_0] = &fyTbl[0][0],
	[CPU_LEVEL_1] = &sbTbl[0][0],
	[CPU_LEVEL_2] = &c65tTbl[0][0],
	[CPU_LEVEL_3] = &c65Tbl[0][0],
	[CPU_LEVEL_4] = &c62Tbl[0][0],
	[CPU_LEVEL_5] = &c62lyTbl[0][0],
};
