/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_fb.h"
#include "mipi_ams367_oled.h"

#ifndef DEBUG_MIPI
#define DEBUG_MIPI 1
#define DEBUG_THIS()	pr_err("(mipi) %s : %d called\n", __func__, __LINE__)
#define DEBUG_IN()	pr_err("(mipi) + %s : %d called\n", __func__, __LINE__)
#define DEBUG_OUT()	pr_err("(mipi) - %s : %d called\n", __func__, __LINE__)
#define DEBUG_STR(X, ...)	pr_err("(mipi) %s : "X"\n", __func__, ## __VA_ARGS__)

static char debug_str[256];
static int debug_str_pos;
#define DEBUG_STR_CLEAR() do { debug_str[0] = 0; debug_str_pos = 0; } while (0)
#define DEBUG_STR_ADD(a)	strcpy(debug_str + strlen(debug_str), a)
#define DEBUG_STR_ADD_PRINTF(a, ...)	snprintf(debug_str + strlen(debug_str), sizeof(debug_str) - strlen(debug_str) - 1, a, ## __VA_ARGS__)
#define DEBUG_STR_SHOW()	pr_err("(mipi) %s : %s\n", __func__, debug_str)

#ifndef TRUE
#define TRUE (1 == 1)
#define FALSE (!TRUE)
#endif
#endif /* #ifndef DEBUG_MIPI */

static struct msm_panel_info pinfo;
#define MAX_GAMMA_VALUE 25

static char all_pixel_off_cmd[] = { 0x22, /* no param */  };
static char normal_mode_on_cmd[] = { 0x13, /* no parm */  };
static char sleep_in_cmd[] = { 0x10, /* no param */  };
static char sleep_out_cmd[] = { 0x11, /* no param */  };
static char display_on_cmd[] = { 0x29, /* no param */  };
static char display_off_cmd[] = { 0x28, /* no param */  };

/******************** gamma ******************/

/* gamma value: 2.2 */
/* warning : these values are not made from SMD. it maked by Excel. */
/* Fix value (from Grande model) : 30, 90, 110, 140, 170, 200, 220, 240, 260, 280, 300 */
static char s6e63m0_22_300[] = { 0x18, 0x08, 0x24, 0x5F, 0x50, 0x2D, 0xB6, 0xB9, 0xA7, 0xAD, 0xB1, 0x9F, 0xBE, 0xC0, 0xB5, 0x00, 0xA0, 0x00, 0xA4, 0x00, 0xDB };
static char s6e63m0_22_290[] = { 0x18, 0x08, 0x24, 0x61, 0x53, 0x30, 0xB6, 0xB9, 0xA7, 0xAC, 0xB1, 0x9E, 0xBF, 0xC0, 0xB6, 0x00, 0x9E, 0x00, 0xA1, 0x00, 0xD8 };
static char s6e63m0_22_280[] = { 0x18, 0x08, 0x24, 0x64, 0x56, 0x33, 0xB6, 0xBA, 0xA8, 0xAC, 0xB1, 0x9D, 0xC1, 0xC1, 0xB7, 0x00, 0x9C, 0x00, 0x9F, 0x00, 0xD6 };
static char s6e63m0_22_270[] = { 0x18, 0x08, 0x24, 0x65, 0x57, 0x33, 0xB6, 0xBA, 0xA7, 0xAD, 0xB2, 0x9E, 0xC1, 0xC1, 0xB7, 0x00, 0x99, 0x00, 0x9C, 0x00, 0xD3 };
static char s6e63m0_22_260[] = { 0x18, 0x08, 0x24, 0x66, 0x58, 0x34, 0xB6, 0xBA, 0xA7, 0xAF, 0xB3, 0xA0, 0xC1, 0xC2, 0xB7, 0x00, 0x97, 0x00, 0x9A, 0x00, 0xD1 };
static char s6e63m0_22_250[] = { 0x18, 0x08, 0x24, 0x64, 0x56, 0x32, 0xB7, 0xBA, 0xA8, 0xAF, 0xB3, 0xA0, 0xC1, 0xC2, 0xB7, 0x00, 0x94, 0x00, 0x97, 0x00, 0xD5 };
static char s6e63m0_22_240[] = { 0x18, 0x08, 0x24, 0x62, 0x54, 0x30, 0xB9, 0xBB, 0xA9, 0xB0, 0xB3, 0xA1, 0xC1, 0xC3, 0xB7, 0x00, 0x91, 0x00, 0x95, 0x00, 0xDA };
static char s6e63m0_22_230[] = { 0x18, 0x08, 0x24, 0x62, 0x53, 0x30, 0xB8, 0xBB, 0xA9, 0xB0, 0xB4, 0xA1, 0xC2, 0xC3, 0xB7, 0x00, 0x8E, 0x00, 0x91, 0x00, 0xCE };
static char s6e63m0_22_220[] = { 0x18, 0x08, 0x24, 0x63, 0x53, 0x31, 0xB8, 0xBC, 0xA9, 0xB0, 0xB5, 0xA2, 0xC4, 0xC4, 0xB8, 0x00, 0x8B, 0x00, 0x8E, 0x00, 0xC2 };
static char s6e63m0_22_210[] = { 0x18, 0x08, 0x24, 0x64, 0x54, 0x32, 0xB9, 0xBC, 0xAA, 0xB0, 0xB5, 0xA2, 0xC4, 0xC5, 0xB8, 0x00, 0x88, 0x00, 0x8B, 0x00, 0xBE };
static char s6e63m0_22_200[] = { 0x18, 0x08, 0x24, 0x66, 0x55, 0x34, 0xBA, 0xBD, 0xAB, 0xB1, 0xB5, 0xA3, 0xC5, 0xC6, 0xB9, 0x00, 0x85, 0x00, 0x88, 0x00, 0xBA };
static char s6e63m0_22_190[] = { 0x18, 0x08, 0x24, 0x67, 0x54, 0x35, 0xBA, 0xBD, 0xAB, 0xB2, 0xB5, 0xA4, 0xC5, 0xC6, 0xBA, 0x00, 0x81, 0x00, 0x84, 0x00, 0xB5 };
static char s6e63m0_22_180[] = { 0x18, 0x08, 0x24, 0x68, 0x54, 0x36, 0xBA, 0xBD, 0xAB, 0xB3, 0xB6, 0xA5, 0xC6, 0xC7, 0xBB, 0x00, 0x7E, 0x00, 0x81, 0x00, 0xB0 };
static char s6e63m0_22_170[] = { 0x18, 0x08, 0x24, 0x69, 0x54, 0x37, 0xBB, 0xBE, 0xAC, 0xB4, 0xB7, 0xA6, 0xC7, 0xC8, 0xBC, 0x00, 0x7B, 0x00, 0x7E, 0x00, 0xAB };
static char s6e63m0_22_160[] = { 0x18, 0x08, 0x24, 0x6A, 0x54, 0x38, 0xBB, 0xBE, 0xAC, 0xB5, 0xB8, 0xA7, 0xC7, 0xC8, 0xBC, 0x00, 0x77, 0x00, 0x7A, 0x00, 0xA6 };
static char s6e63m0_22_150[] = { 0x18, 0x08, 0x24, 0x6B, 0x54, 0x39, 0xBB, 0xBE, 0xAC, 0xB6, 0xB9, 0xA8, 0xC8, 0xC8, 0xBD, 0x00, 0x74, 0x00, 0x76, 0x00, 0xA2 };
static char s6e63m0_22_140[] = { 0x18, 0x08, 0x24, 0x6C, 0x54, 0x3A, 0xBC, 0xBF, 0xAC, 0xB7, 0xBB, 0xA9, 0xC9, 0xC9, 0xBE, 0x00, 0x71, 0x00, 0x73, 0x00, 0x9E };
static char s6e63m0_22_130[] = { 0x18, 0x08, 0x24, 0x6D, 0x53, 0x3B, 0xBD, 0xBF, 0xAD, 0xB7, 0xBB, 0xA9, 0xCA, 0xCA, 0xBF, 0x00, 0x6D, 0x00, 0x6F, 0x00, 0x98 };
static char s6e63m0_22_120[] = { 0x18, 0x08, 0x24, 0x6E, 0x52, 0x3C, 0xBE, 0xC0, 0xAE, 0xB8, 0xBB, 0xAA, 0xCB, 0xCB, 0xC0, 0x00, 0x69, 0x00, 0x6B, 0x00, 0x92 };
static char s6e63m0_22_110[] = { 0x18, 0x08, 0x24, 0x70, 0x51, 0x3E, 0xBF, 0xC1, 0xAF, 0xB9, 0xBC, 0xAB, 0xCC, 0xCC, 0xC2, 0x00, 0x65, 0x00, 0x67, 0x00, 0x8D };
static char s6e63m0_22_100[] = { 0x18, 0x08, 0x24, 0x71, 0x4D, 0x3D, 0xBF, 0xC1, 0xB0, 0xBA, 0xBD, 0xAB, 0xCD, 0xCD, 0xC3, 0x00, 0x61, 0x00, 0x62, 0x00, 0x87 };
static char s6e63m0_22_90[] = { 0x18, 0x08, 0x24, 0x73, 0x4A, 0x3D, 0xC0, 0xC2, 0xB1, 0xBB, 0xBE, 0xAC, 0xCE, 0xCF, 0xC5, 0x00, 0x5D, 0x00, 0x5E, 0x00, 0x82 };
static char s6e63m0_22_80[] = { 0x18, 0x08, 0x24, 0x73, 0x65, 0x3D, 0xC1, 0xC2, 0xB1, 0xBC, 0xBF, 0xAD, 0xCF, 0xD0, 0xC6, 0x00, 0x57, 0x00, 0x57, 0x00, 0x79 };
static char s6e63m0_22_70[] = { 0x18, 0x08, 0x24, 0x74, 0x80, 0x3D, 0xC2, 0xC2, 0xB2, 0xBE, 0xC1, 0xAF, 0xD0, 0xD1, 0xC7, 0x00, 0x51, 0x00, 0x50, 0x00, 0x71 };
static char s6e63m0_22_60[] = { 0x18, 0x08, 0x24, 0x75, 0x9B, 0x3D, 0xC4, 0xC2, 0xB3, 0xBF, 0xC2, 0xB1, 0xD1, 0xD3, 0xC8, 0x00, 0x4B, 0x00, 0x4A, 0x00, 0x69 };
static char s6e63m0_22_50[] = { 0x18, 0x08, 0x24, 0x76, 0xB6, 0x3D, 0xC5, 0xC2, 0xB4, 0xC1, 0xC4, 0xB2, 0xD2, 0xD4, 0xC9, 0x00, 0x45, 0x00, 0x43, 0x00, 0x61 };
static char s6e63m0_22_40[] = { 0x18, 0x08, 0x24, 0x77, 0xD1, 0x3D, 0xC6, 0xC2, 0xB5, 0xC2, 0xC5, 0xB4, 0xD3, 0xD5, 0xCA, 0x00, 0x3F, 0x00, 0x3C, 0x00, 0x59 };
static char s6e63m0_22_30[] = { 0x18, 0x08, 0x24, 0x78, 0xEC, 0x3D, 0xC8, 0xC2, 0xB6, 0xC4, 0xC7, 0xB6, 0xD5, 0xD7, 0xCC, 0x00, 0x39, 0x00, 0x36, 0x00, 0x51 };
static char s6e63m0_22_20[] = { 0x18, 0x08, 0x24, 0x78, 0xEC, 0x3D, 0xC9, 0xC2, 0xB6, 0xC5, 0xC8, 0xB7, 0xD6, 0xD8, 0xCD, 0x00, 0x33, 0x00, 0x2F, 0x00, 0x48 };
static char s6e63m0_22_10[] = { 0x18, 0x08, 0x24, 0x79, 0xEC, 0x3D, 0xCA, 0xC2, 0xB7, 0xC7, 0xCA, 0xB9, 0xD7, 0xD9, 0xCE, 0x00, 0x2D, 0x00, 0x28, 0x00, 0x40 };

static char s6e63m0_22_0[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static char *s6e63m0_22_div10[] = {
	s6e63m0_22_0,		/* 0 */
	s6e63m0_22_10,
	s6e63m0_22_20,
	s6e63m0_22_30,		/* 3 */
	s6e63m0_22_40,		/* 4 */
	s6e63m0_22_50,		/* 5 */
	s6e63m0_22_60,		/* 6 */
	s6e63m0_22_70,		/* 7 */
	s6e63m0_22_80,		/* 8 */
	s6e63m0_22_90,		/* 9 */
	s6e63m0_22_100,		/* 10 */
	s6e63m0_22_110,		/* 11 */
	s6e63m0_22_120,		/* 12 */
	s6e63m0_22_130,		/* 13 */
	s6e63m0_22_140,		/* 14 */
	s6e63m0_22_150,		/* 15 */
	s6e63m0_22_160,		/* 16 */
	s6e63m0_22_170,		/* 17 */
	s6e63m0_22_180,		/* 18 */
	s6e63m0_22_190,		/* 19 */
	s6e63m0_22_200,		/* 20 */
	s6e63m0_22_210,		/* 21 */
	s6e63m0_22_220,		/* 22 */
	s6e63m0_22_230,		/* 23 */
	s6e63m0_22_240,		/* 24 */
	s6e63m0_22_250,		/* 25 */
	s6e63m0_22_260,		/* 26 */
	s6e63m0_22_270,		/* 27 */
	s6e63m0_22_280,		/* 28 */
	s6e63m0_22_290,		/* 29 */
	s6e63m0_22_300,		/* 30 */
};

#define gamma_reg_update_pos 2
/* GAMMA SET FROM SMD */
char gamma_reg_set[] = {	/* 300cd */
	0xFA,
	0x02, 0x18, 0x08, 0x24, 0x6B, 0x76, 0x57, 0xBD,
	0xC3, 0xB5, 0xB4, 0xBB, 0xAC, 0xC5, 0xC9, 0xC0,
	0x00, 0xB7, 0x00, 0xAB, 0x00, 0xCF
};
static char gamma_update_set[] = { 0xFA, 0x03 };

struct dsi_cmd_desc ams367_gamma_set_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_reg_set), gamma_reg_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_update_set), gamma_update_set}
	,
};

/********************* ACL *******************/

static char acl_on_set[] = { 0xC0, 0x01 };
static char acl_off_set[] = { 0xC0, 0x00 };

struct dsi_cmd_desc acl_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

struct dsi_cmd_desc acl_off_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_off_set), acl_off_set}
	,
};

static char acl_cond_50_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x08, 0x0F, 0x16, 0x1D, 0x24, 0x2A, 0x31,
	0x38, 0x3F, 0x46
};

struct dsi_cmd_desc acl_cond_50_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_50_set), acl_cond_50_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

static char acl_cond_48_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x08, 0x0E, 0x15, 0x1B, 0x22, 0x29, 0x2F,
	0x36, 0x3C, 0x43
};

struct dsi_cmd_desc acl_cond_48_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_48_set), acl_cond_48_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

static char acl_cond_47_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x07, 0x0E, 0x14, 0x1B, 0x21, 0x27, 0x2E,
	0x34, 0x3B, 0x41
};

struct dsi_cmd_desc acl_cond_47_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_47_set), acl_cond_47_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

static char acl_cond_45_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x07, 0x0D, 0x13, 0x19, 0x1F, 0x25, 0x2B,
	0x31, 0x37, 0x3D
};

struct dsi_cmd_desc acl_cond_45_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_45_set), acl_cond_45_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

static char acl_cond_43_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x07, 0x0C, 0x12, 0x18, 0x1E, 0x23, 0x29,
	0x2F, 0x34, 0x3A
};

struct dsi_cmd_desc acl_cond_43_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_43_set), acl_cond_43_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

static char acl_cond_40_set[] = { 0xC1,
	0x4D, 0x96, 0x1D, 0x00, 0x00, 0x01, 0xDF, 0x00,
	0x00, 0x03, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x06, 0x0C, 0x11, 0x16, 0x1C, 0x21, 0x26,
	0x2B, 0x31, 0x36
};

struct dsi_cmd_desc acl_cond_40_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_cond_40_set), acl_cond_40_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(acl_on_set), acl_on_set}
	,
};

struct lcd_lux_command_set acl_match_table[] = {
	{0, "0+", 0, acl_off_cmds},
	{30, "30+", 0, acl_cond_40_cmds},	/* need 33percent */
	{40, "40+", 0, acl_cond_40_cmds},
};

/********************* ELVSS *******************/

static char elvss_cond_set[] = { 0xB2, 0x10, 0x10, 0x10, 0x10 };
static char dynamic_elvss_on_set[] = { 0xB1, 0x0B };
static char dynamic_elvss_off_set[] = { 0xB1, 0x0A };

struct dsi_cmd_desc elvss_cond_set_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(elvss_cond_set), elvss_cond_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dynamic_elvss_on_set), dynamic_elvss_on_set}
	,
};

struct dsi_cmd_desc dynamic_elvss_off_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(dynamic_elvss_off_set), dynamic_elvss_off_set}
	,
};

struct lcd_lux_command_set elvss_match_table[] = {
	{0, "0+", 12, elvss_cond_set_cmds},
	{30, "30+", 12, elvss_cond_set_cmds},
	{110, "110+", 8, elvss_cond_set_cmds},
	{170, "170+", 6, elvss_cond_set_cmds},
	{210, "210+", 0, elvss_cond_set_cmds},
};

/* after Power-On */

static char when_read_id_set[] = { 0xD5,
	0xE7, 0x14, 0x60, 0x17, 0x0A, 0x49, 0xC3, 0x8F,
	0x19, 0x64, 0x91, 0x84, 0x76, 0x20, 0x43, 0x00,
};

static char key_command_set[] = { 0xF0, 0x5A, 0x5A };
static char level2_key_set[] = { 0xF1, 0x5A, 0x5A };

/* LCD initialize set after SleepOut(11h) */

static char panel_cond_set1[] = {
	0xF8,
	0x01, 0x2D, 0x2D, 0x08, 0x08, 0x61, 0xB7, 0x72,
	0x86, 0x1E, 0x3B, 0x0F, 0x00, 0x00
	    /* 9th 86 -> datasheet value = 0x9A */
};

static char display_cond_set1[] = {
	0xF2,
	0x02, 0x03, 0x1C, 0x10, 0x10
	    /* 0x02, 0x9, 0x69, 0x14, 0x10, */
	    /* 02(number if line=800), 03=VBP, 1C=VFP, 10=HBP, 10=HFP */
};

static char display_cond_set2[] = {
	0xF7,
	0x00, 0x00, 0x00,
	/* 1st 00h -> LCD vertical flag */
};

static char gamma_setting_300cd_set[] = { 0xFA,
	0x02, 0x18, 0x08, 0x24, 0x6B, 0x76, 0x57, 0xBD,
	0xC3, 0xB5, 0xB4, 0xBB, 0xAC, 0xC5, 0xC9, 0xC0,
	0x00, 0xB7, 0x00, 0xAB, 0x00, 0xCF
};

/* need gamma_update_set after gamma_reg_set */

static char etc_cond_set1[] = { 0xF6, 0x00, 0x8E, 0x0F };
static char etc_cond_set2[] = { 0xB3, 0x6C };

static char etc_cond_set3[] = {
	0xB5,
	0x2C, 0x12, 0x0C, 0x0A, 0x10, 0x0E, 0x17, 0x13,
	0x1F, 0x1A, 0x2A, 0x24, 0x1F, 0x1B, 0x1A, 0x17,
	0x2B, 0x26, 0x22, 0x20, 0x3A, 0x34, 0x30, 0x2C,
	0x29, 0x26, 0x25, 0x23, 0x21, 0x20, 0x1E, 0x1E
};

static char etc_cond_set4[] = {
	0xB6,
	0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x44, 0x44,
	0x55, 0x55, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66
};

static char etc_cond_set5[] = {
	0xB7,
	0x2C, 0x12, 0x0C, 0x0A, 0x10, 0x0E, 0x17, 0x13,
	0x1F, 0x1A, 0x2A, 0x24, 0x1F, 0x1B, 0x1A, 0x17,
	0x2B, 0x26, 0x22, 0x20, 0x3A, 0x34, 0x30, 0x2C,
	0x29, 0x26, 0x25, 0x23, 0x21, 0x20, 0x1E, 0x1E
};

static char etc_cond_set6[] = {
	0xB8,
	0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x44, 0x44,
	0x55, 0x55, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66
};

static char etc_cond_set7[] = {
	0xB9,
	0x2C, 0x12, 0x0C, 0x0A, 0x10, 0x0E, 0x17, 0x13,
	0x1F, 0x1A, 0x2A, 0x24, 0x1F, 0x1B, 0x1A, 0x17,
	0x2B, 0x26, 0x22, 0x20, 0x3A, 0x34, 0x30, 0x2C,
	0x29, 0x26, 0x25, 0x23, 0x21, 0x20, 0x1E, 0x1E
};

static char etc_cond_set8[] = {
	0xBA,
	0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x44, 0x44,
	0x55, 0x55, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66
};

struct dsi_cmd_desc ams367_after_read_id_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(when_read_id_set), when_read_id_set}
	,
};

struct dsi_cmd_desc ams367_lcd_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(key_command_set), key_command_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level2_key_set), level2_key_set}
	,
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(sleep_out_cmd), sleep_out_cmd}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(panel_cond_set1), panel_cond_set1}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(display_cond_set1), display_cond_set1}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(display_cond_set2), display_cond_set2}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_setting_300cd_set), gamma_setting_300cd_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_update_set), gamma_update_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set1), etc_cond_set1}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set2), etc_cond_set2}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set3), etc_cond_set3}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set4), etc_cond_set4}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set5), etc_cond_set5}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set6), etc_cond_set6}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set7), etc_cond_set7}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(etc_cond_set8), etc_cond_set8}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(elvss_cond_set), elvss_cond_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 120, sizeof(dynamic_elvss_on_set), dynamic_elvss_on_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(display_on_cmd), display_on_cmd}
	,
};

struct dsi_cmd_desc ams367_ready_to_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off_cmd), display_off_cmd}
	,
};

struct dsi_cmd_desc ams367_panel_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on_cmd), display_on_cmd}
	,
};

struct dsi_cmd_desc ams367_panel_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off_cmd), display_off_cmd}
	,
};

struct dsi_cmd_desc ams367_late_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(normal_mode_on_cmd), normal_mode_on_cmd}
	,
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_on_cmd), display_on_cmd}
	,
};

struct dsi_cmd_desc ams367_early_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(sleep_in_cmd), sleep_in_cmd}
	,
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(all_pixel_off_cmd), all_pixel_off_cmd}
	,
};

struct dsi_cmd_desc ams367_gamma_300cd_set_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_setting_300cd_set), gamma_setting_300cd_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(gamma_update_set), gamma_update_set}
	,
};

struct dsi_cmd_desc mtp_read_enable_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(key_command_set), key_command_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level2_key_set), level2_key_set}
	,
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(when_read_id_set), when_read_id_set}		
	,
};

static int set_gamma_level(int bl_level, enum gamma_mode_list gamma_mode);

static struct mipi_panel_data mipi_pd = {
	.panel_name = "SMD_AMS367\n",
	.ready_to_on = {ams367_lcd_on_cmds, ARRAY_SIZE(ams367_lcd_on_cmds)},
	.ready_to_off = {ams367_ready_to_off_cmds, ARRAY_SIZE(ams367_ready_to_off_cmds)},
	.on = {ams367_panel_on_cmds, ARRAY_SIZE(ams367_panel_on_cmds)},
	.off = {ams367_panel_off_cmds, ARRAY_SIZE(ams367_panel_off_cmds)},
	.late_on = {ams367_late_on_cmds, ARRAY_SIZE(ams367_late_on_cmds)},
	.early_off = {ams367_early_off_cmds, ARRAY_SIZE(ams367_early_off_cmds)},
#ifdef USE_ACL
	.acl_on = {acl_on_cmds, ARRAY_SIZE(acl_on_cmds)},
	.acl_off = {acl_off_cmds, ARRAY_SIZE(acl_off_cmds)},
	.acl_update = {acl_cond_50_cmds,
		       ARRAY_SIZE(acl_cond_50_cmds)},
	.set_acl = set_acl_on_level,
#endif

#ifdef USE_ELVSS
	.elvss_update = {elvss_cond_set_cmds,
			 ARRAY_SIZE(elvss_cond_set_cmds)},
	.set_elvss = set_elvss_level,
#endif
/*      .gamma_update = {ams367_gamma_300cd_set_cmds, ARRAY_SIZE(ams367_gamma_300cd_set_cmds)}, */
	.gamma_update = {ams367_gamma_set_cmds, ARRAY_SIZE(ams367_gamma_set_cmds)},

	.set_gamma = set_gamma_level,
	.lcd_current_cd_idx = -1,
	.mtp_read_enable = {mtp_read_enable_cmds, ARRAY_SIZE(mtp_read_enable_cmds)},
};

#ifdef USE_ACL
static int set_acl_on_level(int bl_level)
{
	DEBUG_THIS();
	return 0;
}

static int set_elvss_level(int bl_level)
{
	DEBUG_THIS();
	return 0;
}
#endif

void reset_gamma_level(void)
{
	DEBUG_THIS();
}

int get_gamma_lux(int bl_level)
{
	int lux;

	lux = (bl_level > 250 ? 300 : bl_level);

	return lux;
}

static int set_gamma_level(int bl_level, enum gamma_mode_list gamma_mode)
{
	int pos;

	/* exception code : smartdimming is not implemented yet */
	gamma_mode = GAMMA_2_2;

	if (gamma_mode == GAMMA_SMART) {
		mipi_pd.smart_ea8868[mipi_pd.lcd_no].brightness_level = get_gamma_lux(bl_level);
		DEBUG_STR("Smart : bl_level=%d, gamma=%d", bl_level, mipi_pd.smart_ea8868[mipi_pd.lcd_no].brightness_level);
		generate_gamma(&(mipi_pd.smart_ea8868[mipi_pd.lcd_no]), gamma_reg_set + gamma_reg_update_pos, GAMMA_SET_MAX);
	} else {
		pos = (bl_level > 250 ? ARRAY_SIZE(s6e63m0_22_div10) - 1 : bl_level / 10);

		DEBUG_STR_CLEAR();
		DEBUG_STR_ADD_PRINTF("bl_level=%d, pos=%d", bl_level, pos);
		DEBUG_STR_SHOW();

		memcpy(gamma_reg_set + gamma_reg_update_pos, s6e63m0_22_div10[pos], sizeof(s6e63m0_22_0));
	}

	return 0;
}

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0x5B, 0x28, 0x0C, 0x00, 0x33, 0x3A, 0x10, 0x2C, 0x14, 0x03, 0x04},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xee, 0x02, 0x86, 0x00},
	/* pll control */
	{0x0, 0x7f, 0x31, 0xda, 0x00, 0x50, 0x48, 0x63,
	 0x41, 0x0f, 0x01,
	 0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01},
};



int is_S6E88A(void);
static int __init mipi_video_ams367_oled_wvga_pt_init(void)
{
	int ret;

	if( is_S6E88A() )
	{
		pr_info("%s: IGNORE\n", __func__);
		return -ENODEV;
	}

#ifdef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (msm_fb_detect_client("mipi_video_ams367_oled_wvga"))
		return 0;
#endif

	pinfo.xres = 480;
	pinfo.yres = 800;
	pinfo.mode2_xres = 0;
	pinfo.mode2_yres = 0;
	pinfo.mode2_bpp = 0;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;

	pinfo.lcdc.h_back_porch = 83;
	pinfo.lcdc.h_front_porch = 200;
	pinfo.lcdc.h_pulse_width = 2;

	pinfo.lcdc.v_back_porch = 1;
	pinfo.lcdc.v_front_porch = 13;
	pinfo.lcdc.v_pulse_width = 2;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0x404040;	/* gray */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 255;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 343500000;
	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = FALSE;
	pinfo.mipi.hbp_power_stop = FALSE;
	pinfo.mipi.hsa_power_stop = FALSE;
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.traffic_mode = DSI_BURST_MODE;
	pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;

	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	pinfo.mipi.dlane_swap = 0x01;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;

	pinfo.mipi.tx_eot_append = FALSE;
	pinfo.mipi.t_clk_post = 0x19;
	pinfo.mipi.t_clk_pre = 0x2D;
	pinfo.mipi.stream = 0;	/* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 60;
	pinfo.mipi.force_clk_lane_hs = 1;
	pinfo.mipi.esc_byte_ratio = 3;
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	ret = ams367_mipi_samsung_device_register(&pinfo, MIPI_DSI_PRIM, MIPI_DSI_PANEL_WVGA_PT, &mipi_pd);
	if (ret)
		pr_info("%s: failed to register device!\n", __func__);

	pr_info("%s:\n", __func__);
	return ret;
}

module_init(mipi_video_ams367_oled_wvga_pt_init);
