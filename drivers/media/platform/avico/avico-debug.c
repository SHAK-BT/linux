/*
 * ELVEES Avico (a.k.a. VELcore-01) driver - Debug helpers
 *
 * Copyright 2015 ELVEES NeoTek JSC
 * Copyright 2017 RnD Center "ELVEES", JSC
 *
 * Author: Anton Leontiev <aleontiev@elvees.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/types.h>

#include "avico.h"

#define dump(name, base, offset) \
	pr_info(name " = %08x\n", ioread32((base) + (offset)))

#define dumphex(name, buf, offset, size) \
	print_hex_dump(KERN_INFO, name ": ", DUMP_PREFIX_NONE, 32, 4, \
		       (buf) + (offset), (size), false)

void avico_regs_ctrl_dump(void __iomem *const base)
{
	dump("ctrl.events  ", base, AVICO_CTRL_EVENTS);
	dump("ctrl.mski_cpu", base, AVICO_CTRL_MSKI_CPU);
	dump("ctrl.msko_cpu", base, AVICO_CTRL_MSKO_CPU);
	dump("ctrl.msko_dsp", base, AVICO_CTRL_MSKO_DSP);
	dump("ctrl.vram_cfg", base, AVICO_CTRL_VRAM_CFG);
	dump("ctrl.msk_int ", base, AVICO_CTRL_MSK_INT);
	dump("ctrl.msk_ev  ", base, AVICO_CTRL_MSK_EV);
}

void avico_regs_thread_dump(void __iomem *const base)
{
	dump("thread.task  ", base, AVICO_THREAD_TASK);
	dump("thread.adr   ", base, AVICO_THREAD_ADR);
	dump("thread.mbpos ", base, AVICO_THREAD_MBPOS);
	dump("thread.frmn  ", base, AVICO_THREAD_FRMN);
	dump("thread.cfg   ", base, AVICO_THREAD_CFG);
	dump("thread.on    ", base, AVICO_THREAD_ON);
	dump("thread.off   ", base, AVICO_THREAD_OFF);
	dump("thread.status", base, AVICO_THREAD_STATUS);
	dump("thread.smbpos", base, AVICO_THREAD_SMBPOS);
}

void avico_regs_vdma_sys_dump(void __iomem *const base)
{
	dump("vdma.sys.run   ", base, AVICO_VDMA_SYS_RUN);
	dump("vdma.sys.run   ", base, AVICO_VDMA_SYS_RUN);
	dump("vdma.sys.busy  ", base, AVICO_VDMA_SYS_BUSY);
	dump("vdma.sys.active", base, AVICO_VDMA_SYS_ACTV);
	dump("vdma.sys.done  ", base, AVICO_VDMA_SYS_DONE);
	dump("vdma.sys.ready ", base, AVICO_VDMA_SYS_READY);
	dump("vdma.sys.mode  ", base, AVICO_VDMA_SYS_MODE);
}

void avico_regs_vdma_channel_dump(void __iomem *const base)
{
	dump("vdma.channel.a0e   ", base, AVICO_VDMA_CHANNEL_A0E);
	dump("vdma.channel.aecur ", base, AVICO_VDMA_CHANNEL_AECUR);
	dump("vdma.channel.a0i   ", base, AVICO_VDMA_CHANNEL_A0I);
	dump("vdma.channel.aicur ", base, AVICO_VDMA_CHANNEL_AICUR);
	dump("vdma.channel.beidx ", base, AVICO_VDMA_CHANNEL_BEIDX);
	dump("vdma.channel.ceidx ", base, AVICO_VDMA_CHANNEL_CEIDX);
	dump("vdma.channel.heidx ", base, AVICO_VDMA_CHANNEL_HEIDX);
	dump("vdma.channel.veidx ", base, AVICO_VDMA_CHANNEL_VEIDX);
	dump("vdma.channel.biidx ", base, AVICO_VDMA_CHANNEL_BIIDX);
	dump("vdma.channel.ciidx ", base, AVICO_VDMA_CHANNEL_CIIDX);
	dump("vdma.channel.hiidx ", base, AVICO_VDMA_CHANNEL_HIIDX);
	dump("vdma.channel.viidx ", base, AVICO_VDMA_CHANNEL_VIIDX);
	dump("vdma.channel.acnt  ", base, AVICO_VDMA_CHANNEL_ACNT);
	dump("vdma.channel.bccnt ", base, AVICO_VDMA_CHANNEL_BCCNT);
	dump("vdma.channel.hvecnt", base, AVICO_VDMA_CHANNEL_HVECNT);
	dump("vdma.channel.hvicnt", base, AVICO_VDMA_CHANNEL_HVICNT);
	dump("vdma.channel.run   ", base, AVICO_VDMA_CHANNEL_RUN);
	dump("vdma.channel.done  ", base, AVICO_VDMA_CHANNEL_DONE);
	dump("vdma.channel.imrdy ", base, AVICO_VDMA_CHANNEL_IMRDY);
	dump("vdma.channel.cfg   ", base, AVICO_VDMA_CHANNEL_CFG);
}

void avico_regs_vdma_dump(void __iomem *const base, unsigned channel)
{
	avico_regs_vdma_sys_dump(base + AVICO_VDMA_SYS_BASE);
	avico_regs_vdma_channel_dump(base + AVICO_VDMA_CHANNEL_BASE(channel++));
	avico_regs_vdma_channel_dump(base + AVICO_VDMA_CHANNEL_BASE(channel++));
	avico_regs_vdma_channel_dump(base + AVICO_VDMA_CHANNEL_BASE(channel++));
	avico_regs_vdma_channel_dump(base + AVICO_VDMA_CHANNEL_BASE(channel));
}

void avico_regs_ec_taskctrc_dump(void __iomem *const base)
{
	dump("ec.taskcrtc.task       ", base, AVICO_TASKCTRC_TASK);
	dump("ec.taskcrtc.cs         ", base, AVICO_TASKCTRC_CS);
	dump("ec.taskcrtc.mbpos      ", base, AVICO_TASKCTRC_MBPOS);
	dump("ec.taskcrtc.frmn       ", base, AVICO_TASKCTRC_FRMN);
	dump("ec.taskcrtc.dma_cbs_len", base, AVICO_TASKCTRC_DMALEN);
}

void avico_regs_ec_vramctrc_dump(void __iomem *const base)
{
	dump("ec.vramctrc.base_cp0", base, AVICO_VRAMCTRC_BASE_CP0);
	dump("ec.vramctrc.base_cp1", base, AVICO_VRAMCTRC_BASE_CP1);
	dump("ec.vramctrc.base_cp2", base, AVICO_VRAMCTRC_BASE_CP2);
	dump("ec.vramctrc.base_cp3", base, AVICO_VRAMCTRC_BASE_CP3);
	dump("ec.vramctrc.base_cpn", base, AVICO_VRAMCTRC_BASE_CPN);
	dump("ec.vramctrc.base_cqc", base, AVICO_VRAMCTRC_BASE_CQC);
	dump("ec.vramctrc.base_cbs", base, AVICO_VRAMCTRC_BASE_CBS);
	dump("ec.vramctrc.size_cp0", base, AVICO_VRAMCTRC_SIZE_CP0);
	dump("ec.vramctrc.size_cp1", base, AVICO_VRAMCTRC_SIZE_CP1);
	dump("ec.vramctrc.size_cp2", base, AVICO_VRAMCTRC_SIZE_CP2);
	dump("ec.vramctrc.size_cp3", base, AVICO_VRAMCTRC_SIZE_CP3);
	dump("ec.vramctrc.size_cqc", base, AVICO_VRAMCTRC_SIZE_CQC);
	dump("ec.vramctrc.size_cbs", base, AVICO_VRAMCTRC_SIZE_CBS);
	dump("ec.vramctrc.addr_cp0", base, AVICO_VRAMCTRC_ADDR_CP0);
	dump("ec.vramctrc.addr_cp1", base, AVICO_VRAMCTRC_ADDR_CP1);
	dump("ec.vramctrc.addr_cp2", base, AVICO_VRAMCTRC_ADDR_CP2);
	dump("ec.vramctrc.addr_cp3", base, AVICO_VRAMCTRC_ADDR_CP3);
	dump("ec.vramctrc.addr_cpn", base, AVICO_VRAMCTRC_ADDR_CPN);
	dump("ec.vramctrc.addr_cpa", base, AVICO_VRAMCTRC_ADDR_CPA);
	dump("ec.vramctrc.addr_cpb", base, AVICO_VRAMCTRC_ADDR_CPB);
	dump("ec.vramctrc.addr_cqc", base, AVICO_VRAMCTRC_ADDR_CQC);
	dump("ec.vramctrc.addr_cbs", base, AVICO_VRAMCTRC_ADDR_CBS);
}

void avico_regs_ec_cavlc_dump(void __iomem *const base)
{
	dump("ec.cavlc.cr", base, AVICO_CAVLC_CR);
	dump("ec.cavlc.sr", base, AVICO_CAVLC_SR);
}

void avico_regs_ec_hdrc_dump(void __iomem *const base)
{
	dump("ec.hdrc.cr", base, AVICO_HDRC_CR);
}

void avico_regs_ec_packer_dump(void __iomem *const base)
{
	dump("ec.packer.cr             ", base, AVICO_PACKER_CR);
	dump("ec.packer.cbs_total_len  ", base, AVICO_PACKER_CBS_TOTAL_LEN);
	dump("ec.packer.cbs_extbits    ", base, AVICO_PACKER_CBS_EXTBITS);
	dump("ec.packer.cbs_extbits_len", base, AVICO_PACKER_CBS_EXTBITS_LEN);
	dump("ec.packer.cbs_stuff_mode ", base, AVICO_PACKER_CBS_STUFF_MODE);
	dump("ec.packer.cbs_stuff_pos  ", base, AVICO_PACKER_CBS_STUFF_POS);
}

void avico_regs_ec_regc_dump(void __iomem *const base)
{
	uint8_t buf[0x200];

	memcpy_fromio(buf, base, 0x200 / 4);

	dumphex("ec.regc.cp0", buf, 0x00, 0x10);
	dumphex("ec.regc.cp1", buf, 0x10, 0x10);
	dumphex("ec.regc.cp2", buf, 0x20, 0x10);
	dumphex("ec.regc.cp3", buf, 0x30, 0x10);
	dumphex("ec.regc.cpn", buf, 0x40, 0x10);
	dumphex("ec.regc.cpa", buf, 0x50, 0x10);
	dumphex("ec.regc.cpb", buf, 0x60, 0x10);
	dumphex("ec.regc.cqc", buf, 0x80, 0x80);
	dumphex("ec.regc.cbs", buf, 0x100, 0x100);
}

void avico_regs_ec_dump(void __iomem *const base)
{
	avico_regs_ec_regc_dump(base + AVICO_EC_REGC);
	avico_regs_ec_cavlc_dump(base + AVICO_EC_CAVLC);
	avico_regs_ec_hdrc_dump(base + AVICO_EC_HDRC);
	avico_regs_ec_packer_dump(base + AVICO_EC_PACKER);
	avico_regs_ec_vramctrc_dump(base + AVICO_EC_VRAMCTRC);
	avico_regs_ec_taskctrc_dump(base + AVICO_EC_TASKCTRC);
}

void avico_regs_dump(void __iomem *const base, unsigned const thread)
{
	avico_regs_ctrl_dump(base + AVICO_CTRL_BASE);
	avico_regs_thread_dump(base + AVICO_THREAD_BASE(thread));
	avico_regs_vdma_dump(base + AVICO_VDMA_BASE, thread * 4);
	avico_regs_ec_dump(base + AVICO_EC_BASE(thread));
}
