/*
 * ambhw/vin.h
 *
 * History:
 *	2007/01/27 - [Charles Chiou] created file
 *
 * Copyright (C) 2006-2011, Ambarella, Inc.
 */

#ifndef __AMBHW_VIN_H__
#define __AMBHW_VIN_H__

#include <ambhw/chip.h>
#include <ambhw/busaddr.h>

/****************************************************/
/* Capabilities based on chip revision              */
/****************************************************/

#if (CHIP_REV == A1)  || (CHIP_REV == A2)  || \
    (CHIP_REV == A2S) || (CHIP_REV == A2M) || (CHIP_REV == A2Q) || \
    (CHIP_REV == A5L)
#define VIN_DIRECT_DSP_INTERFACE	0
#else
#define VIN_DIRECT_DSP_INTERFACE	1
#endif

#if (CHIP_REV == A1)
#define VIN_SMEM_PREVIEW_INSTANCES	0
#elif (CHIP_REV == A2) || (CHIP_REV == A2S) || (CHIP_REV == A2M) || \
      (CHIP_REV == A2Q)
#define VIN_SMEM_PREVIEW_INSTANCES	1
#else
#define VIN_SMEM_PREVIEW_INSTANCES	2
#endif

#if (CHIP_REV == A5) || (CHIP_REV == A6) || (CHIP_REV == A5S) || \
    (CHIP_REV == A7) || (CHIP_REV == I1) || (CHIP_REV == A7L) || \
    (CHIP_REV == AMB_S2)
#define VIN_SUPPORT_SLVS_MLVS		1
#else
#define VIN_SUPPORT_SLVS_MLVS		0
#endif

#if (CHIP_REV == A7) || (CHIP_REV == I1) || (CHIP_REV == A7L) || \
    (CHIP_REV == AMB_S2)
#define VIN_SPPORT_SEPARATE_SLVS_MLVS	1
#else
#define VIN_SPPORT_SEPARATE_SLVS_MLVS	0
#endif

#if (CHIP_REV == A7) || (CHIP_REV == AMB_S2)
#define VIN_SLVS_LANES_MAX		12
#elif (CHIP_REV == I1) || (CHIP_REV == A7L)
#define VIN_SLVS_LANES_MAX		8
#endif

#if (CHIP_REV == A7) || (CHIP_REV == AMB_S2)
#define VIN_CFA_MAX_WIDTH		4096
#else
#define VIN_CFA_MAX_WIDTH		3200
#endif

#if (CHIP_REV == A5S) || (CHIP_REV == I1) || (CHIP_REV == A7L) || \
    (CHIP_REV == AMB_S2)
#define VIN_SUPPORT_MIPI		1
#else
#define VIN_SUPPORT_MIPI		0
#endif

#if (CHIP_REV == I1) || (CHIP_REV == A7L) || (CHIP_REV == AMB_S2)
#define VIN_SUPPORT_CLK_SELECTION	1
#else
#define VIN_SUPPORT_CLK_SELECTION	0
#endif

#if (CHIP_REV == A7L) || (CHIP_REV == AMB_S2)
#define VIN_SUPPORT_AMBA_MIPI_PHY	1
#else
#define VIN_SUPPORT_AMBA_MIPI_PHY	0
#endif

#if (CHIP_REV == AMB_S2)
#define VIN_SUPPORT_DUAL_SENSOR_INTERFACES	1
#define VIN_SUPPORT_BIT_WIDE_MASTER_SYNC	1
#define VIN_SUPPORT_SLVS_SYNC_CODE_REMAP	1
#else
#define VIN_SUPPORT_DUAL_SENSOR_INTERFACES	0	
#define VIN_SUPPORT_BIT_WIDE_MASTER_SYNC	0
#define VIN_SUPPORT_SLVS_SYNC_CODE_REMAP	0
#endif

#if (CHIP_REV == AMB_S2)
#define VIN_INSTANCES			2
#else
#define VIN_INSTANCES			1
#endif

/****************************************************/
/* Controller registers definitions                 */
/****************************************************/

#if (VIN_DIRECT_DSP_INTERFACE == 0)

#define S_CTRL_OFFSET			0x00
#define S_STATUS_OFFSET			0x08
#define S_HV_WIDTH_OFFSET		0x0c
#define S_H_OFFSET_OFFSET		0x10
#define S_HV_OFFSET			0x14
#define S_MIN_HV_OFFSET			0x18
#define S_TRIGGER0_OFFSET		0x1c
#define S_TRIGGER1_OFFSET		0x20
#define S_BLHW_AVGV_OFFSET		0x24

#if (VIN_SMEM_PREVIEW_INSTANCES == 1)
#define S_VOUT_START_OFFSET		0x28
#endif /* VIN_SMEM_PREVIEW_INSTANCES == 1 */

#define S_CAP_START_OFFSET		0x2c
#define S_CAP_END_OFFSET		0x30
#define S_BLHW_CTRL_OFFSET		0x34
#define S_BLHW_AVGH_OFFSET		0x38
#define S_BLHWOO_OFFSET			0x3c
#define S_BLHWOE_OFFSET			0x40
#define S_BLHWEO_OFFSET			0x44
#define S_BLHWEE_OFFSET			0x48
#define S_BLSWOO_OFFSET			0x4c
#define S_BLSWOE_OFFSET			0x50
#define S_BLSWEO_OFFSET			0x54
#define S_BLSWEE_OFFSET			0x58
#define S_DEBUG_FIFO_COUNT_OFFSET	0x64
#define S_DEBUG_FIFO_DATA_OFFSET	0x80

#define S_CTRL_REG			VIN_REG(S_CTRL_OFFSET)
#define S_STATUS_REG			VIN_REG(S_STATUS_OFFSET)
#define S_HV_WIDTH			VIN_REG(S_HV_WIDTH_OFFSET)
#define S_H_OFFSET			VIN_REG(S_H_OFFSET_OFFSET)
#define S_HV				VIN_REG(S_HV_OFFSET)
#define S_MIN_HV_REG			VIN_REG(S_MIN_HV_OFFSET)
#define S_TRIGGER0_REG			VIN_REG(S_TRIGGER0_OFFSET)
#define S_TRIGGER1_REG			VIN_REG(S_TRIGGER1_OFFSET)
#define S_BLHW_AVGV_REG 		VIN_REG(S_BLHW_AVGV_OFFSET)

#if (VIN_SMEM_PREVIEW_INSTANCES == 1)
#define S_VOUT_START_REG		VIN_REG(S_VOUT_START_OFFSET)
#endif /* VIN_SMEM_PREVIEW_INSTANCES == 1 */

#define S_CAP_START_REG			VIN_REG(S_CAP_START_OFFSET)
#define S_CAP_END_REG			VIN_REG(S_CAP_END_OFFSET)
#define S_BLHW_CTRL_REG			VIN_REG(S_BLHW_CTRL_OFFSET)
#define S_BLHW_AVGH_REG			VIN_REG(S_BLHW_AVGH_OFFSET)
#define S_BLHWOO_REG			VIN_REG(S_BLHWOO_OFFSET)
#define S_BLHWOE_REG			VIN_REG(S_BLHWOE_OFFSET)
#define S_BLHWEO_REG			VIN_REG(S_BLHWEO_OFFSET)
#define S_BLHWEE_REG			VIN_REG(S_BLHWEE_OFFSET)
#define S_BLSWOO_REG			VIN_REG(S_BLSWOO_OFFSET)
#define S_BLSWOE_REG			VIN_REG(S_BLSWOE_OFFSET)
#define S_BLSWEO_REG			VIN_REG(S_BLSWEO_OFFSET)
#define S_BLSWEE_REG			VIN_REG(S_BLSWEE_OFFSET)
#define S_DEBUG_FIFO_COUNT_REG		VIN_REG(S_DEBUG_FIFO_COUNT_OFFSET)
#define S_DEBUG_FIFO_DATA_REG		VIN_REG(S_DEBUG_FIFO_DATA_OFFSET)

#endif /* VIN_DIRECT_DSP_INTERFACE == 0 */

#if (VIN_DIRECT_DSP_INTERFACE == 1)
#define S_CTRL_INDEX			0x00
#define S_INPUTCFG_INDEX		0x01
#define S_STATUS_INDEX			0x02
#define S_VWIDTH_INDEX			0x03
#define S_HWIDTH_INDEX			0x04
#define S_HOFFSETTOP_INDEX		0x05
#define S_HOFFSETBTM_INDEX		0x06
#define S_V_INDEX			0x07
#define S_H_INDEX			0x08
#define S_MINV_INDEX			0x09
#define S_MINH_INDEX			0x0A
#define S_TRIGGER_0_START_INDEX		0x0B
#define S_TRIGGER_0_END_INDEX		0x0C
#define S_TRIGGER_1_START_INDEX		0x0D
#define S_TRIGGER_1_END_INDEX		0x0E
#define S_VOUT_START_0_INDEX		0x0F
#define S_VOUT_START_1_INDEX		0x10
#define S_CAP_START_V_INDEX		0x11
#define S_CAP_START_H_INDEX		0x12
#define S_CAP_END_V_INDEX		0x13
#define S_CAP_END_H_INDEX		0x14
#define S_BLANK_LENGTH_H_INDEX		0x15
#define S_TIMEOUT_V_LOW_INDEX		0x16
#define S_TIMEOUT_V_HIGH_INDEX		0x17
#define S_TIMEOUT_H_LOW_INDEX		0x18
#define S_TIMEOUT_H_HIGH_INDEX		0x19

#define S_CTRL_OFFSET			(S_CTRL_INDEX << 2)
#define S_INPUTCFG_OFFSET		(S_INPUTCFG_INDEX << 2)
#define S_STATUS_OFFSET			(S_STATUS_INDEX << 2)
#define S_VWIDTH_OFFSET			(S_VWIDTH_INDEX << 2)
#define S_HWIDTH_OFFSET			(S_HWIDTH_INDEX << 2)
#define S_HOFFSETTOP_OFFSET		(S_HOFFSETTOP_INDEX << 2)
#define S_HOFFSETBTM_OFFSET		(S_HOFFSETBTM_INDEX << 2)
#define S_V_OFFSET			(S_V_INDEX << 2)
#define S_H_OFFSET			(S_H_INDEX << 2)
#define S_MINV_OFFSET			(S_MINV_INDEX << 2)
#define S_MINH_OFFSET			(S_MINH_INDEX << 2)
#define S_TRIGGER_0_START_OFFSET	(S_TRIGGER_0_START_INDEX << 2)
#define S_TRIGGER_0_END_OFFSET		(S_TRIGGER_0_END_INDEX << 2)
#define S_TRIGGER_1_START_OFFSET	(S_TRIGGER_1_START_INDEX << 2)
#define S_TRIGGER_1_END_OFFSET		(S_TRIGGER_1_END_INDEX << 2)
#define S_VOUT_START_0_OFFSET		(S_VOUT_START_0_INDEX << 2)
#define S_VOUT_START_1_OFFSET		(S_VOUT_START_1_INDEX << 2)
#define S_CAP_START_V_OFFSET		(S_CAP_START_V_INDEX << 2)
#define S_CAP_START_H_OFFSET		(S_CAP_START_H_INDEX << 2)
#define S_CAP_END_V_OFFSET		(S_CAP_END_V_INDEX << 2)
#define S_CAP_END_H_OFFSET		(S_CAP_END_H_INDEX << 2)
#define S_BLANK_LENGTH_H_OFFSET		(S_BLANK_LENGTH_H_INDEX << 2)
#define S_TIMEOUT_V_LOW_OFFSET		(S_TIMEOUT_V_LOW_INDEX << 2)
#define S_TIMEOUT_V_HIGH_OFFSET		(S_TIMEOUT_V_HIGH_INDEX << 2)
#define S_TIMEOUT_H_LOW_OFFSET		(S_TIMEOUT_H_LOW_INDEX << 2)
#define S_TIMEOUT_H_HIGH_OFFSET		(S_TIMEOUT_H_HIGH_INDEX << 2)

#define S_CTRL_REG			DSP_VIN_DEBUG_REG(S_CTRL_OFFSET)
#define S_INPUTCFG_REG			DSP_VIN_DEBUG_REG(S_INPUTCFG_OFFSET)
#define S_STATUS_REG			DSP_VIN_DEBUG_REG(S_STATUS_OFFSET)
#define S_VWIDTH_REG			DSP_VIN_DEBUG_REG(S_VWIDTH_OFFSET)
#define S_HWIDTH_REG			DSP_VIN_DEBUG_REG(S_HWIDTH_OFFSET)
#define S_V_REG				DSP_VIN_DEBUG_REG(S_V_OFFSET)
#define S_H_REG				DSP_VIN_DEBUG_REG(S_H_OFFSET)
#define S_MINV_REG			DSP_VIN_DEBUG_REG(S_MINV_OFFSET)
#define S_MINH_REG			DSP_VIN_DEBUG_REG(S_MINH_OFFSET)
#define S_CAPSTARTV_REG			DSP_VIN_DEBUG_REG(S_CAP_START_V_OFFSET)
#define S_CAPSTARTH_REG			DSP_VIN_DEBUG_REG(S_CAP_START_H_OFFSET)
#define S_CAPENDV_REG			DSP_VIN_DEBUG_REG(S_CAP_END_V_OFFSET)
#define S_CAPENDH_REG			DSP_VIN_DEBUG_REG(S_CAP_END_H_OFFSET)

#if (VIN_SUPPORT_MIPI == 1)
#define S_MIPI_CTRL1_INDEX		0x1A
#define S_MIPI_CTRL2_INDEX		0x1B
#define S_MIPI_B_DPHYCTRL_INDEX		0x1C
#define S_MIPI_S_DPHYCTRL_INDEX		0x1D
#define S_MIPI_ERROR_STATUS_INDEX	0x1E
#define S_MIPI_CRC_ERROR_FRM_INDEX	0x1F

#define S_MIPI_CTRL1_OFFSET		(S_MIPI_CTRL1_INDEX << 2)
#define S_MIPI_CTRL2_OFFSET		(S_MIPI_CTRL2_INDEX << 2)
#define S_MIPI_B_DPHYCTRL_OFFSET	(S_MIPI_B_DPHYCTRL_INDEX << 2)
#define S_MIPI_S_DPHYCTRL_OFFSET	(S_MIPI_S_DPHYCTRL_INDEX << 2)
#define S_MIPI_ERROR_STATUS_OFFSET	(S_MIPI_ERROR_STATUS_INDEX << 2)
#define S_MIPI_CRC_ERROR_FRM_OFFSET	(S_MIPI_CRC_ERROR_FRM_INDEX << 2)

#define S_MIPI_CTRL1_REG		DSP_VIN_DEBUG_REG(S_MIPI_CTRL1_OFFSET)
#define S_MIPI_CTRL2_REG		DSP_VIN_DEBUG_REG(S_MIPI_CTRL2_OFFSET)
#define S_MIPI_B_DPHYCTRL_REG		DSP_VIN_DEBUG_REG(S_MIPI_B_DPHYCTRL_OFFSET)
#define S_MIPI_S_DPHYCTRL_REG		DSP_VIN_DEBUG_REG(S_MIPI_S_DPHYCTRL_OFFSET)
#define S_MIPI_ERROR_STATUS_REG		DSP_VIN_DEBUG_REG(S_MIPI_ERROR_STATUS_OFFSET)
#define S_MIPI_CRC_ERROR_FRM_REG	DSP_VIN_DEBUG_REG(S_MIPI_CRC_ERROR_FRM_OFFSET)

#if (VIN_SUPPORT_CLK_SELECTION == 1)
/* A7L */
#if (VIN_SUPPORT_AMBA_MIPI_PHY == 1)
#define S_MIPI_PHY_RESET_INDEX		0x34
#define S_MIPI_FRM_NUM_INDEX		0x0205
#define S_MIPI_PHY_STATUS0_INDEX	0x0206
#define S_MIPI_PHY_STATUS1_INDEX	0x0207

/* I1 */
#else
#define S_MIPI_FRM_NUM_INDEX		0x2F
#define S_MIPI_PHY_STATUS0_INDEX	0x30
#define S_MIPI_PHY_STATUS1_INDEX	0x31
#endif /* VIN_SUPPORT_AMBA_MIPI_PHY == 1 */

#define S_MIPI_FRM_NUM_OFFSET		(S_MIPI_FRM_NUM_INDEX << 2)
#define S_MIPI_PHY_STATUS0_OFFSET	(S_MIPI_PHY_STATUS0_INDEX << 2)
#define S_MIPI_PHY_STATUS1_OFFSET	(S_MIPI_PHY_STATUS1_INDEX << 2)

#define S_MIPI_FRM_NUM_REG		DSP_VIN_DEBUG_REG(S_MIPI_FRM_NUM_OFFSET)
#define S_MIPI_PHY_STATUS0_REG		DSP_VIN_DEBUG_REG(S_MIPI_PHY_STATUS0_OFFSET)
#define S_MIPI_PHY_STATUS1_REG		DSP_VIN_DEBUG_REG(S_MIPI_PHY_STATUS1_OFFSET)
/* A5S */
#else
#define S_MIPI_FRM_NUM_INDEX		0x25
#define S_MIPI_PHY_STATUS_INDEX		0x26

#define S_MIPI_FRM_NUM_OFFSET		(S_MIPI_FRM_NUM_INDEX << 2)
#define S_MIPI_PHY_STATUS_OFFSET	(S_MIPI_PHY_STATUS_INDEX << 2)

#define S_MIPI_FRM_NUM_REG		DSP_VIN_DEBUG_REG(S_MIPI_FRM_NUM_OFFSET)
#define S_MIPI_PHY_STATUS_REG		DSP_VIN_DEBUG_REG(S_MIPI_PHY_STATUS_OFFSET)
#endif /* VIN_SUPPORT_CLK_SELECTION == 1 */

#endif /* VIN_SUPPORT_MIPI == 1 */

#if (VIN_SPPORT_SEPARATE_SLVS_MLVS == 1)
#define SLVS_CTRL_INDEX			0x20
#define SLVS_FRAME_INDEX		0x21
#define SLVS_ACTIVE_FRAME_INDEX		0x22
#define SLVS_VSYNC_MAX_INDEX		0x23
#define SLVS_STATUS_INDEX		0x28
#define SLVS_LINE_SYNC_TIMEOUT_INDEX	0x29
#define SLVS_DEBUG_INDEX		0x2A
#define SLVS_LANE_MUX_SELECT_0_INDEX	0x24
#define SLVS_LANE_MUX_SELECT_1_INDEX	0x25

#define SLVS_CTRL_OFFSET		(SLVS_CTRL_INDEX << 2)
#define SLVS_FRAME_OFFSET		(SLVS_FRAME_INDEX << 2)
#define SLVS_ACTIVE_FRAME_OFFSET	(SLVS_ACTIVE_FRAME_INDEX << 2)
#define SLVS_VSYNC_MAX_OFFSET		(SLVS_VSYNC_MAX_INDEX << 2)
#define SLVS_LINE_SYNC_TIMEOUT_OFFSET	(SLVS_LINE_SYNC_TIMEOUT_INDEX << 2)
#define SLVS_LANE_MUX_SELECT_0_OFFSET	(SLVS_LANE_MUX_SELECT_0_INDEX << 2)
#define SLVS_LANE_MUX_SELECT_1_OFFSET	(SLVS_LANE_MUX_SELECT_1_INDEX << 2)

#if (VIN_SLVS_LANES_MAX > 8)
#define SLVS_LANE_MUX_SELECT_2_INDEX	0x26
#define SLVS_LANE_MUX_SELECT_2_OFFSET	(SLVS_LANE_MUX_SELECT_2_INDEX << 2)
#endif /* VIN_SLVS_LANES_MAX > 8 */

#else

#define SLVS_STATUS_INDEX		0x24

#endif /* VIN_SPPORT_SEPARATE_SLVS_MLVS == 1 */

#if (VIN_SUPPORT_BIT_WIDE_MASTER_SYNC == 1)
#define S_VSYNCSTART_INDEX		0x45
#define S_VSYNCEND_INDEX		0x46

#define S_VSYNCHSTART_OFFSET		(S_VSYNCHSTART_INDEX << 2)
#define S_VSYNCHEND_OFFSET		(S_VSYNCHEND_INDEX << 2)
#endif /* VIN_SUPPORT_BIT_WIDE_MASTER_SYNC == 1 */

#if (VIN_SUPPORT_SLVS_SYNC_CODE_REMAP == 1)
#define SLVS_CTRL_1_INDEX		0x40
#define SLVS_SAV_VZERO_MAP_INDEX	0x41
#define SLVS_SAV_VONE_MAP_INDEX		0x42
#define SLVS_EAV_VZERO_MAP_INDEX	0x43
#define SLVS_EAV_VONE_MAP_INDEX		0x44

#define SLVS_CTRL_1_OFFSET		(SLVS_CTRL_1_INDEX << 2)
#define SLVS_SAV_VZERO_MAP_OFFSET	(SLVS_SAV_VZERO_MAP_INDEX << 2)
#define SLVS_SAV_VONE_MAP_OFFSET	(SLVS_SAV_VONE_MAP_INDEX << 2)
#define SLVS_EAV_VZERO_MAP_OFFSET	(SLVS_EAV_VZERO_MAP_INDEX << 2)
#define SLVS_EAV_VONE_MAP_OFFSET	(SLVS_EAV_VONE_MAP_INDEX << 2)
#endif /* VIN_SUPPORT_SLVS_SYNC_CODE_REMAP == 1 */

#endif /* VIN_DIRECT_DSP_INTERFACE == 1 */

#endif /*  __AMBHW_VIN_H__ */
