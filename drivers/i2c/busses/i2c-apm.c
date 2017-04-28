/*
 * Synopsys DesignWare I2C adapter driver (master only).
 *
 * Based on the TI DAVINCI I2C adapter driver.
 *
 * Copyright (C) 2006 Texas Instruments.
 * Copyright (C) 2007 MontaVista Software Inc.
 * Copyright (C) 2009 Provigent Ltd.
 * Copyright (c) 2010 Applied Micro Circuits Corporation.
 * 
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/of_i2c.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/slab.h>

/*
 * Registers offset
 */
#define DW_IC_CON		0x0
#define DW_IC_TAR		0x4
#define DW_IC_DATA_CMD		0x10
#define DW_IC_SS_SCL_HCNT	0x14
#define DW_IC_SS_SCL_LCNT	0x18
#define DW_IC_FS_SCL_HCNT	0x1c
#define DW_IC_FS_SCL_LCNT	0x20
#define DW_IC_INTR_STAT		0x2c
#define DW_IC_INTR_MASK		0x30
#define DW_IC_RAW_INTR_STAT	0x34
#define DW_IC_RX_TL		0x38
#define DW_IC_TX_TL		0x3c
#define DW_IC_CLR_INTR		0x40
#define DW_IC_CLR_RX_UNDER	0x44
#define DW_IC_CLR_RX_OVER	0x48
#define DW_IC_CLR_TX_OVER	0x4c
#define DW_IC_CLR_RD_REQ	0x50
#define DW_IC_CLR_TX_ABRT	0x54
#define DW_IC_CLR_RX_DONE	0x58
#define DW_IC_CLR_ACTIVITY	0x5c
#define DW_IC_CLR_STOP_DET	0x60
#define DW_IC_CLR_START_DET	0x64
#define DW_IC_CLR_GEN_CALL	0x68
#define DW_IC_ENABLE		0x6c
#define DW_IC_STATUS		0x70
#define DW_IC_TXFLR		0x74
#define DW_IC_RXFLR		0x78
#define DW_IC_COMP_PARAM_1	0xf4
#define DW_IC_TX_ABRT_SOURCE	0x80

#define DW_IC_CON_MASTER		0x1
#define DW_IC_CON_SPEED_STD		0x2
#define DW_IC_CON_SPEED_FAST		0x4
#define DW_IC_CON_10BITADDR_MASTER	0x10
#define DW_IC_CON_RESTART_EN		0x20
#define DW_IC_CON_SLAVE_DISABLE		0x40

#define DW_IC_INTR_RX_UNDER	0x001
#define DW_IC_INTR_RX_OVER	0x002
#define DW_IC_INTR_RX_FULL	0x004
#define DW_IC_INTR_TX_OVER	0x008
#define DW_IC_INTR_TX_EMPTY	0x010
#define DW_IC_INTR_RD_REQ	0x020
#define DW_IC_INTR_TX_ABRT	0x040
#define DW_IC_INTR_RX_DONE	0x080
#define DW_IC_INTR_ACTIVITY	0x100
#define DW_IC_INTR_STOP_DET	0x200
#define DW_IC_INTR_START_DET	0x400
#define DW_IC_INTR_GEN_CALL	0x800

#define DW_IC_INTR_DEFAULT_MASK		(DW_IC_INTR_RX_FULL | \
					 DW_IC_INTR_TX_EMPTY | \
					 DW_IC_INTR_TX_ABRT | \
					 DW_IC_INTR_STOP_DET)

#define DW_IC_STATUS_ACTIVITY	0x1

#define DW_IC_ERR_TX_ABRT	0x1

/*
 * status codes
 */
#define STATUS_IDLE			0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS		0x2

#define TIMEOUT			20 /* ms */

/*
 * hardware abort codes from the DW_IC_TX_ABRT_SOURCE register
 *
 * only expected abort codes are listed here
 * refer to the datasheet for the full list
 */
#define ABRT_7B_ADDR_NOACK	0
#define ABRT_10ADDR1_NOACK	1
#define ABRT_10ADDR2_NOACK	2
#define ABRT_TXDATA_NOACK	3
#define ABRT_GCALL_NOACK	4
#define ABRT_GCALL_READ		5
#define ABRT_SBYTE_ACKDET	7
#define ABRT_SBYTE_NORSTRT	9
#define ABRT_10B_RD_NORSTRT	10
#define ABRT_MASTER_DIS		11
#define ARB_LOST		12

#define DW_IC_TX_ABRT_7B_ADDR_NOACK	(1UL << ABRT_7B_ADDR_NOACK)
#define DW_IC_TX_ABRT_10ADDR1_NOACK	(1UL << ABRT_10ADDR1_NOACK)
#define DW_IC_TX_ABRT_10ADDR2_NOACK	(1UL << ABRT_10ADDR2_NOACK)
#define DW_IC_TX_ABRT_TXDATA_NOACK	(1UL << ABRT_TXDATA_NOACK)
#define DW_IC_TX_ABRT_GCALL_NOACK	(1UL << ABRT_GCALL_NOACK)
#define DW_IC_TX_ABRT_GCALL_READ	(1UL << ABRT_GCALL_READ)
#define DW_IC_TX_ABRT_SBYTE_ACKDET	(1UL << ABRT_SBYTE_ACKDET)
#define DW_IC_TX_ABRT_SBYTE_NORSTRT	(1UL << ABRT_SBYTE_NORSTRT)
#define DW_IC_TX_ABRT_10B_RD_NORSTRT	(1UL << ABRT_10B_RD_NORSTRT)
#define DW_IC_TX_ABRT_MASTER_DIS	(1UL << ABRT_MASTER_DIS)
#define DW_IC_TX_ARB_LOST		(1UL << ARB_LOST)

#define DW_IC_TX_ABRT_NOACK		(DW_IC_TX_ABRT_7B_ADDR_NOACK | \
					 DW_IC_TX_ABRT_10ADDR1_NOACK | \
					 DW_IC_TX_ABRT_10ADDR2_NOACK | \
					 DW_IC_TX_ABRT_TXDATA_NOACK | \
					 DW_IC_TX_ABRT_GCALL_NOACK)

static char *abort_sources[] = {
	[ABRT_7B_ADDR_NOACK] =
		"slave address not acknowledged (7bit mode)",
	[ABRT_10ADDR1_NOACK] =
		"first address byte not acknowledged (10bit mode)",
	[ABRT_10ADDR2_NOACK] =
		"second address byte not acknowledged (10bit mode)",
	[ABRT_TXDATA_NOACK] =
		"data not acknowledged",
	[ABRT_GCALL_NOACK] =
		"no acknowledgement for a general call",
	[ABRT_GCALL_READ] =
		"read after general call",
	[ABRT_SBYTE_ACKDET] =
		"start byte acknowledged",
	[ABRT_SBYTE_NORSTRT] =
		"trying to send start byte when restart is disabled",
	[ABRT_10B_RD_NORSTRT] =
		"trying to read when restart is disabled (10bit mode)",
	[ABRT_MASTER_DIS] =
		"trying to use disabled adapter",
	[ARB_LOST] =
		"lost arbitration",
};

/**
 * struct dw_i2c_dev - private i2c-designware data
 * @dev: driver model device node
 * @base: IO registers pointer
 * @cmd_complete: tx completion indicator
 * @lock: protect this struct and IO registers
 * @clk: input reference clock
 * @cmd_err: run time hadware error code
 * @msgs: points to an array of messages currently being transfered
 * @msgs_num: the number of elements in msgs
 * @msg_write_idx: the element index of the current tx message in the msgs
 *	array
 * @tx_buf_len: the length of the current tx buffer
 * @tx_buf: the current tx buffer
 * @msg_read_idx: the element index of the current rx message in the msgs
 *	array
 * @rx_buf_len: the length of the current rx buffer
 * @rx_buf: the current rx buffer
 * @msg_err: error status of the current transfer
 * @status: i2c master status, one of STATUS_*
 * @abort_source: copy of the TX_ABRT_SOURCE register
 * @irq: interrupt number for the i2c master
 * @adapter: i2c subsystem adapter node
 * @tx_fifo_depth: depth of the hardware tx fifo
 * @rx_fifo_depth: depth of the hardware rx fifo
 */
struct dw_i2c_dev {
	struct device		*dev;
	void __iomem		*base;
	struct completion	cmd_complete;
	struct mutex		lock;
	struct clk		*clk;
	int			cmd_err;
	struct i2c_msg		*msgs;
	int			msgs_num;
	int			msg_write_idx;
	u32			tx_buf_len;
	u8			*tx_buf;
	int			msg_read_idx;
	u32			rx_buf_len;
	u8			*rx_buf;
	int			msg_err;
	unsigned int		status;
	u32			abort_source;
	int			irq;
	struct i2c_adapter	adapter;
	unsigned int		tx_fifo_depth;
	unsigned int		rx_fifo_depth;
	void (*write)(u32 , volatile void __iomem *);
	u32  (*read)(const volatile void __iomem *) ;

};

static u32
i2c_dw_scl_hcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset)
{
	/*
	 * DesignWare I2C core doesn't seem to have solid strategy to meet
	 * the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
	 * will result in violation of the tHD;STA spec.
	 */
	if (cond)
		/*
		 * Conditional expression:
		 *
		 *   IC_[FS]S_SCL_HCNT + (1+4+3) >= IC_CLK * tHIGH
		 *
		 * This is based on the DW manuals, and represents an ideal
		 * configuration.  The resulting I2C bus speed will be
		 * faster than any of the others.
		 *
		 * If your hardware is free from tHD;STA issue, try this one.
		 */
		return (ic_clk * tSYMBOL + 5000) / 10000 - 8 + offset;
	else
		/*
		 * Conditional expression:
		 *
		 *   IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD;STA + tf)
		 *
		 * This is just experimental rule; the tHD;STA period turned
		 * out to be proportinal to (_HCNT + 3).  With this setting,
		 * we could meet both tHIGH and tHD;STA timing specs.
		 *
		 * If unsure, you'd better to take this alternative.
		 *
		 * The reason why we need to take into account "tf" here,
		 * is the same as described in i2c_dw_scl_lcnt().
		 */
		return (ic_clk * (tSYMBOL + tf) + 5000) / 10000 - 3 + offset;
}

static u32 i2c_dw_scl_lcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset)
{
	/*
	 * Conditional expression:
	 *
	 *   IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
	 *
	 * DW I2C core starts counting the SCL CNTs for the LOW period
	 * of the SCL clock (tLOW) as soon as it pulls the SCL line.
	 * In order to meet the tLOW timing spec, we need to take into
	 * account the fall time of SCL signal (tf).  Default tf value
	 * should be 0.3 us, for safety.
	 */
	return ((ic_clk * (tLOW + tf) + 5000) / 10000) - 1 + offset;
}

/**
 * i2c_dw_init() - initialize the designware i2c master hardware
 * @dev: device private data
 *
 * This functions configures and enables the I2C master.
 * This function is called during I2C init function, and in case of timeout at
 * run time.
 */
static void i2c_dw_init(struct dw_i2c_dev *dev)
{
	u32 input_clock_khz = clk_get_rate(dev->clk) / 1000;
	u32 ic_con, hcnt, lcnt;
	dev_dbg(dev->dev, "i2c clock_khz %d\n", input_clock_khz);

	/* Disable the adapter */
	dev->write(0, dev->base + DW_IC_ENABLE);

	/* set standard and fast speed deviders for high/low periods */

	/* Standard-mode */
	hcnt = i2c_dw_scl_hcnt(input_clock_khz,
				40,	/* tHD;STA = tHIGH = 4.0 us */
				3,	/* tf = 0.3 us */
				0,	/* 0: DW default, 1: Ideal */
				0);	/* No offset */
	lcnt = i2c_dw_scl_lcnt(input_clock_khz,
				47,	/* tLOW = 4.7 us */
				3,	/* tf = 0.3 us */
				0);	/* No offset */
	dev->write(hcnt, dev->base + DW_IC_SS_SCL_HCNT);
	dev->write(lcnt, dev->base + DW_IC_SS_SCL_LCNT);
	dev_dbg(dev->dev, "Standard-mode HCNT:LCNT = %d:%d\n", hcnt, lcnt);

	/* Fast-mode */
	hcnt = i2c_dw_scl_hcnt(input_clock_khz,
				6,	/* tHD;STA = tHIGH = 0.6 us */
				3,	/* tf = 0.3 us */
				0,	/* 0: DW default, 1: Ideal */
				0);	/* No offset */
	lcnt = i2c_dw_scl_lcnt(input_clock_khz,
				13,	/* tLOW = 1.3 us */
				3,	/* tf = 0.3 us */
				0);	/* No offset */
	dev->write(hcnt, dev->base + DW_IC_FS_SCL_HCNT);
	dev->write(lcnt, dev->base + DW_IC_FS_SCL_LCNT);
	dev_dbg(dev->dev, "Fast-mode HCNT:LCNT = %d:%d\n", hcnt, lcnt);

	/* Configure Tx/Rx FIFO threshold levels */
	dev->write(dev->tx_fifo_depth - 1, dev->base + DW_IC_TX_TL);
	dev->write(0, dev->base + DW_IC_RX_TL);

	/* configure the i2c master */
	ic_con = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE |
		DW_IC_CON_RESTART_EN | DW_IC_CON_SPEED_FAST;
	dev->write(ic_con, dev->base + DW_IC_CON);
}

/*
 * Waiting for bus not busy
 */
static int i2c_dw_wait_bus_not_busy(struct dw_i2c_dev *dev)
{
	int timeout = TIMEOUT;

	while (dev->read(dev->base + DW_IC_STATUS) & DW_IC_STATUS_ACTIVITY) {
		if (timeout <= 0) {
			dev_warn(dev->dev, "timeout waiting for bus ready\n");
			return -ETIMEDOUT;
		}
		timeout--;
		mdelay(1);
	}

	return 0;
}

static void i2c_dw_xfer_init(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 ic_con;

	/* Disable the adapter */
	dev->write(0, dev->base + DW_IC_ENABLE);

	/* set the slave (target) address */
	dev->write(msgs[dev->msg_write_idx].addr, dev->base + DW_IC_TAR);

	/* if the slave address is ten bit address, enable 10BITADDR */
	ic_con = dev->read(dev->base + DW_IC_CON);
	if (msgs[dev->msg_write_idx].flags & I2C_M_TEN)
		ic_con |= DW_IC_CON_10BITADDR_MASTER;
	else
		ic_con &= ~DW_IC_CON_10BITADDR_MASTER;
	dev->write(ic_con, dev->base + DW_IC_CON);

	/* Enable the adapter */
	dev->write(1, dev->base + DW_IC_ENABLE);

	/* Enable interrupts */
	dev->write(DW_IC_INTR_DEFAULT_MASK, dev->base + DW_IC_INTR_MASK);
}

/*
 * Initiate (and continue) low level master read/write transaction.
 * This function is only called from i2c_dw_isr, and pumping i2c_msg
 * messages into the tx buffer.  Even if the size of i2c_msg data is
 * longer than the size of the tx buffer, it handles everything.
 */
static void
i2c_dw_xfer_msg(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	u32 intr_mask;
	int tx_limit, rx_limit;
	u32 addr = msgs[dev->msg_write_idx].addr;
	u32 buf_len = dev->tx_buf_len;
	u8 *buf = dev->tx_buf;;

	intr_mask = DW_IC_INTR_DEFAULT_MASK;

	for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {
		/*
		 * if target address has changed, we need to
		 * reprogram the target address in the i2c
		 * adapter when we are done with this transfer
		 */
		if (msgs[dev->msg_write_idx].addr != addr) {
			dev_err(dev->dev,
				"%s: invalid target address\n", __func__);
			dev->msg_err = -EINVAL;
			break;
		}

		if (msgs[dev->msg_write_idx].len == 0) {
			dev_err(dev->dev,
				"%s: invalid message length\n", __func__);
			dev->msg_err = -EINVAL;
			break;
		}

		if (!(dev->status & STATUS_WRITE_IN_PROGRESS)) {
			/* new i2c_msg */
			buf = msgs[dev->msg_write_idx].buf;
			buf_len = msgs[dev->msg_write_idx].len;
		}

		tx_limit = dev->tx_fifo_depth - dev->read(dev->base + DW_IC_TXFLR);
		rx_limit = dev->rx_fifo_depth - dev->read(dev->base + DW_IC_RXFLR);

		while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
			if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {
				dev->write(0x100, dev->base + DW_IC_DATA_CMD);
				rx_limit--;
			} else
				dev->write(*buf++, dev->base + DW_IC_DATA_CMD);
			tx_limit--; buf_len--;
		}

		dev->tx_buf = buf;
		dev->tx_buf_len = buf_len;

		if (buf_len > 0) {
			/* more bytes to be written */
			dev->status |= STATUS_WRITE_IN_PROGRESS;
			break;
		} else
			dev->status &= ~STATUS_WRITE_IN_PROGRESS;
	}

	/*
	 * If i2c_msg index search is completed, we don't need TX_EMPTY
	 * interrupt any more.
	 */
	if (dev->msg_write_idx == dev->msgs_num)
		intr_mask &= ~DW_IC_INTR_TX_EMPTY;

	if (dev->msg_err)
		intr_mask = 0;

	dev->write(intr_mask, dev->base + DW_IC_INTR_MASK);
}

static void
i2c_dw_read(struct dw_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	int rx_valid;

	for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
		u32 len;
		u8 *buf;

		if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		if (!(dev->status & STATUS_READ_IN_PROGRESS)) {
			len = msgs[dev->msg_read_idx].len;
			buf = msgs[dev->msg_read_idx].buf;
		} else {
			len = dev->rx_buf_len;
			buf = dev->rx_buf;
		}

		rx_valid = dev->read(dev->base + DW_IC_RXFLR);

		for (; len > 0 && rx_valid > 0; len--, rx_valid--)
			*buf++ = dev->read(dev->base + DW_IC_DATA_CMD);

		if (len > 0) {
			dev->status |= STATUS_READ_IN_PROGRESS;
			dev->rx_buf_len = len;
			dev->rx_buf = buf;
			return;
		} else
			dev->status &= ~STATUS_READ_IN_PROGRESS;
	}
}

static int i2c_dw_handle_tx_abort(struct dw_i2c_dev *dev)
{
	unsigned long abort_source = dev->abort_source;
	int i;

	if (abort_source & DW_IC_TX_ABRT_NOACK) {
		for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
			dev_dbg(dev->dev,
				"%s: %s\n", __func__, abort_sources[i]);
		return -EREMOTEIO;
	}

	for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
		dev_err(dev->dev, "%s: %s\n", __func__, abort_sources[i]);

	if (abort_source & DW_IC_TX_ARB_LOST)
		return -EAGAIN;
	else if (abort_source & DW_IC_TX_ABRT_GCALL_READ)
		return -EINVAL; /* wrong msgs[] data */
	else
		return -EIO;
}

/*
 * Prepare controller for a transaction and call i2c_dw_xfer_msg
 */
static int
i2c_dw_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct dw_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	dev_dbg(dev->dev, "%s: msgs: %d\n", __func__, num);

	mutex_lock(&dev->lock);

	INIT_COMPLETION(dev->cmd_complete);
	dev->msgs = msgs;
	dev->msgs_num = num;
	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->status = STATUS_IDLE;
	dev->abort_source = 0;

	ret = i2c_dw_wait_bus_not_busy(dev);
	if (ret < 0)
		goto done;

	/* start the transfers */
	i2c_dw_xfer_init(dev);

	/* wait for tx to complete */
	ret = wait_for_completion_interruptible_timeout(&dev->cmd_complete, HZ);
	if (ret == 0) {
		dev_err(dev->dev, "controller timed out\n");
		i2c_dw_init(dev);
		ret = -ETIMEDOUT;
		goto done;
	} else if (ret < 0)
		goto done;

	if (dev->msg_err) {
		ret = dev->msg_err;
		goto done;
	}

	/* no error */
	if (likely(!dev->cmd_err)) {
		/* Disable the adapter */
		dev->write(0, dev->base + DW_IC_ENABLE);
		ret = num;
		goto done;
	}

	/* We have an error */
	if (dev->cmd_err == DW_IC_ERR_TX_ABRT) {
		ret = i2c_dw_handle_tx_abort(dev);
		goto done;
	}
	ret = -EIO;

done:
	mutex_unlock(&dev->lock);

	return ret;
}

static u32 i2c_dw_func(struct i2c_adapter *adap)
{
	return	I2C_FUNC_I2C |
		I2C_FUNC_10BIT_ADDR |
		I2C_FUNC_SMBUS_BYTE |
		I2C_FUNC_SMBUS_BYTE_DATA |
		I2C_FUNC_SMBUS_WORD_DATA |
		I2C_FUNC_SMBUS_I2C_BLOCK;
}

static u32 i2c_dw_read_clear_intrbits(struct dw_i2c_dev *dev)
{
	u32 stat;

	/*
	 * The IC_INTR_STAT register just indicates "enabled" interrupts.
	 * Ths unmasked raw version of interrupt status bits are available
	 * in the IC_RAW_INTR_STAT register.
	 *
	 * That is,
	 *   stat = readl(IC_INTR_STAT);
	 * equals to,
	 *   stat = readl(IC_RAW_INTR_STAT) & readl(IC_INTR_MASK);
	 *
	 * The raw version might be useful for debugging purposes.
	 */
	stat = dev->read(dev->base + DW_IC_INTR_STAT);

	/*
	 * Do not use the IC_CLR_INTR register to clear interrupts, or
	 * you'll miss some interrupts, triggered during the period from
	 * readl(IC_INTR_STAT) to readl(IC_CLR_INTR).
	 *
	 * Instead, use the separately-prepared IC_CLR_* registers.
	 */
	if (stat & DW_IC_INTR_RX_UNDER)
		dev->read(dev->base + DW_IC_CLR_RX_UNDER);
	if (stat & DW_IC_INTR_RX_OVER)
		dev->read(dev->base + DW_IC_CLR_RX_OVER);
	if (stat & DW_IC_INTR_TX_OVER)
		dev->read(dev->base + DW_IC_CLR_TX_OVER);
	if (stat & DW_IC_INTR_RD_REQ)
		dev->read(dev->base + DW_IC_CLR_RD_REQ);
	if (stat & DW_IC_INTR_TX_ABRT) {
		/*
		 * The IC_TX_ABRT_SOURCE register is cleared whenever
		 * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
		 */
		dev->abort_source = dev->read(dev->base + DW_IC_TX_ABRT_SOURCE);
		dev->read(dev->base + DW_IC_CLR_TX_ABRT);
	}
	if (stat & DW_IC_INTR_RX_DONE)
		dev->read(dev->base + DW_IC_CLR_RX_DONE);
	if (stat & DW_IC_INTR_ACTIVITY)
		dev->read(dev->base + DW_IC_CLR_ACTIVITY);
	if (stat & DW_IC_INTR_STOP_DET)
		dev->read(dev->base + DW_IC_CLR_STOP_DET);
	if (stat & DW_IC_INTR_START_DET)
		dev->read(dev->base + DW_IC_CLR_START_DET);
	if (stat & DW_IC_INTR_GEN_CALL)
		dev->read(dev->base + DW_IC_CLR_GEN_CALL);

	return stat;
}

/*
 * Interrupt service routine. This gets called whenever an I2C interrupt
 * occurs.
 */
static irqreturn_t i2c_dw_isr(int this_irq, void *dev_id)
{
	struct dw_i2c_dev *dev = dev_id;
	u32 stat;

	stat = i2c_dw_read_clear_intrbits(dev);
	dev_dbg(dev->dev, "%s: stat=0x%x\n", __func__, stat);

	if (stat & DW_IC_INTR_TX_ABRT) {
		dev->cmd_err |= DW_IC_ERR_TX_ABRT;
		dev->status = STATUS_IDLE;

		/*
		 * Anytime TX_ABRT is set, the contents of the tx/rx
		 * buffers are flushed.  Make sure to skip them.
		 */
		dev->write(0, dev->base + DW_IC_INTR_MASK);
		goto tx_aborted;
	}

	if (stat & DW_IC_INTR_RX_FULL)
		i2c_dw_read(dev);

	if (stat & DW_IC_INTR_TX_EMPTY)
		i2c_dw_xfer_msg(dev);

	/*
	 * No need to modify or disable the interrupt mask here.
	 * i2c_dw_xfer_msg() will take care of it according to
	 * the current transmit status.
	 */

tx_aborted:
	if ((stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_STOP_DET)) || dev->msg_err)
		complete(&dev->cmd_complete);

	return IRQ_HANDLED;
}

static struct i2c_algorithm i2c_dw_algo = {
	.master_xfer	= i2c_dw_xfer,
	.functionality	= i2c_dw_func,
};

static int __devinit dw_i2c_of_probe(struct platform_device *pdev)
{
	struct dw_i2c_dev *i2c;
	struct i2c_adapter *adap;
	int result = 0;
	u32 param1;
	const u32 *devid;

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;
	
	i2c->dev = &pdev->dev; /* for debug and error output */
  
	i2c->base = of_iomap(pdev->dev.of_node, 0);
	if (!i2c->base) {
		dev_err(i2c->dev, "failed to map controller\n");
		result = -ENOMEM;
		goto fail_map;
	}
	
	i2c->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (i2c->irq != NO_IRQ) { /* i2c->irq = NO_IRQ implies polling */
		result = request_irq(i2c->irq, i2c_dw_isr,
				     0, "i2c-dw", i2c);
		if (result < 0) {
			dev_err(i2c->dev, "failed to attach interrupt\n");
			goto fail_request;
		}
	}

#ifdef CONFIG_APM86xxx
	devid = of_get_property(pdev->dev.of_node, "bus_num", NULL);
	if (*devid == 0) {
		i2c->clk = clk_get(&pdev->dev, "i2c0");
		i2c->write = writel_be;
		i2c->read = readl_be;
	} else {
		i2c->clk = clk_get(&pdev->dev, "i2c1");
		i2c->write = writel;
		i2c->read = readl;
	}
#else
#ifdef __BIG_ENDIAN
	i2c->write = writel_be;
	i2c->read = readl_be;
#else
	i2c->write = writel;
	i2c->read = readl;
#endif
#endif
	if (IS_ERR(i2c->clk)) {
		result = -ENODEV;
		goto fail_clk;
	}
	clk_enable(i2c->clk);
	
	init_completion(&i2c->cmd_complete);
	mutex_init(&i2c->lock);
	param1 = in_be32(i2c->base + DW_IC_COMP_PARAM_1);
	i2c->tx_fifo_depth = ((param1 >> 16) & 0xff) + 1;
	i2c->rx_fifo_depth = ((param1 >> 8)  & 0xff) + 1;

	i2c_dw_init(i2c);
	out_be32(i2c->base + DW_IC_INTR_MASK,0); /* disable IRQ */
	dev_set_drvdata(&pdev->dev, i2c);
	adap = &i2c->adapter;
	i2c_set_adapdata(adap, i2c);
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	adap->dev.parent = &pdev->dev;
	adap->dev.of_node = pdev->dev.of_node;
	strlcpy(adap->name, "Synopsys DesignWare I2C adapter",
			sizeof(adap->name));
	adap->algo = &i2c_dw_algo;
	adap->dev.parent = &pdev->dev;
	
	result = i2c_add_adapter(adap);
	if (result) {
		dev_err(i2c->dev, "failure adding adapter\n");
		goto fail_add;
	}
	
	/* Now register all the child nodes */
	of_i2c_register_devices(adap);

	printk("EVERYTHNIG WENT GREAT\n");
	return 0;

fail_add:	
	dev_set_drvdata(&pdev->dev, NULL);
	clk_disable(i2c->clk);
	clk_put(i2c->clk);
	i2c->clk = NULL;
fail_clk:
	free_irq(i2c->irq, i2c);
fail_request:
	irq_dispose_mapping(i2c->irq);
	iounmap(i2c->base);
fail_map:
	kfree(i2c);
	printk("FAILED TO LAOD I2C :%d\n", result);
	return result;
}

static int __devexit dw_i2c_of_remove(struct platform_device *pdev)
{
	struct dw_i2c_dev *i2c = dev_get_drvdata(&pdev->dev);
	
	i2c_del_adapter(&i2c->adapter);
	dev_set_drvdata(&pdev->dev, NULL);

	clk_disable(i2c->clk);
	clk_put(i2c->clk);
	i2c->clk = NULL;
	
    /* Disable the adapter */
	out_be32(i2c->base + DW_IC_ENABLE,0);

	if (i2c->irq != NO_IRQ)
		free_irq(i2c->irq, i2c);

	irq_dispose_mapping(i2c->irq);
	iounmap(i2c->base);
	kfree(i2c);
	return 0;
};

#ifdef CONFIG_PM
static int dw_of_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
        struct dw_i2c_dev *i2c = dev_get_drvdata(&pdev->dev);
        /* Disable the adapter */
        out_be32(i2c->base + DW_IC_ENABLE,0);
        return 0;
}
static int dw_of_i2c_resume(struct platform_device *pdev)
{
        struct dw_i2c_dev *i2c = dev_get_drvdata(&pdev->dev);
        i2c_dw_init(i2c);
        return 0;
}
#else
#define dw_of_i2c_suspend          NULL
#define dw_of_i2c_resume           NULL
#endif

static const struct of_device_id dw_i2c_of_match[] = {
	{.compatible = "apm,apm86xxx-i2c", } ,
	{}
};

static struct platform_driver dw_i2c_driver = {
	.probe		= dw_i2c_of_probe,
	.remove		= __devexit_p(dw_i2c_of_remove),
	.suspend 	= dw_of_i2c_suspend,
    	.resume 	= dw_of_i2c_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "i2c_designware",
		.of_match_table	= dw_i2c_of_match,
	},
};

static int __init dw_i2c_init_driver(void)
{
	return platform_driver_register(&dw_i2c_driver);
}
module_init(dw_i2c_init_driver);


static void __exit dw_i2c_exit_driver(void)
{
	platform_driver_unregister(&dw_i2c_driver);
}
module_exit(dw_i2c_exit_driver);

MODULE_AUTHOR("Baruch Siach <baruch@tkos.co.il>");
MODULE_DESCRIPTION("Synopsys DesignWare I2C bus adapter");
MODULE_LICENSE("GPL");

