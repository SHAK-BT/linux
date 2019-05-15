/*
 * ELVEES Avico (a.k.a. VELcore-01) driver - Bitstream functions
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
#include "avico-bitstream.h"

static void write_delimiter(struct bitstream *bs)
{
	const uint8_t delimiter[4] = { 0, 0, 0, 1 };

	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);
	WARN_ON(bs->freebits != 8);

	if (bs->p + 4 > bs->end) {
		bs->p = bs->end + 1;
		return;
	}

	memcpy(bs->p, delimiter, 4);
	bs->p += 4;

	/* \note Function call to convert_to_nalu(prior_to_sps_bytes + 5) in
	 * strom-ve causes one byte skip after the synchronization marker.
	 * I do not think this is necessary. Otherwise set bs->nulls to -1. */
	bs->nulls = 0;
}

/* With 8-bit buffer
 * 32-bit buffer is more complex for bit-stuffing */
static void writeu(struct bitstream *bs, uint8_t bits, uint32_t value)
{
	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);

	while (bits) {
		unsigned const writebits = min(bs->freebits, bits);
		unsigned const vmask = ((1 << writebits) - 1) << (bits -
								  writebits);

		/* Number of top writebits */
		unsigned const write = value >> (bits - writebits);

		if (bs->p > bs->end)
			return;

		bs->cb <<= writebits;
		bs->cb |= write;   /* Write to bs->cb */
		value &= ~vmask;   /* Clean written bits in value */
		bits -= writebits; /* Decrease number of bits to be written */
		bs->freebits -= writebits; /* Increase current bit offset */

		if (bs->freebits == 0) {
			bs->freebits = 8;
			/* \dontknow */
			if (bs->nulls == 2 && !(bs->cb & 0xfc)) {
				*bs->p++ = 0x03;
				bs->nulls = 0;
			}
			if (bs->cb == 0)
				bs->nulls++;
			else
				bs->nulls = 0;
			*bs->p++ = bs->cb;
			bs->cb = 0;
		}
	}
}

static void writeue(struct bitstream *bs, uint32_t value)
{
	unsigned const bits = 32 - __builtin_clz(value + 1);

	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);
	WARN_ON(value == U32_MAX);

	writeu(bs, bits * 2 - 1, value + 1);
}

static void writese(struct bitstream *bs, int32_t value)
{
	uint32_t const uvalue = value <= 0 ? (-2 * value) : (2 * value - 1);

	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);

	writeue(bs, uvalue);
}

static void write_trailing_bits(struct bitstream *bs)
{
	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);

	writeu(bs, 1, 1);
	if (bs->freebits != 8)
		writeu(bs, bs->freebits, 0);

	WARN_ON(bs->freebits != 8);
}

void avico_bitstream_init(struct avico_ctx *ctx, void *ptr, unsigned int size)
{
	struct bitstream *const bs = &ctx->bs;

	pr_devel("avico_bitstream_init: ptr = %p, size = %u\n", ptr, size);

	bs->start = (uint8_t *)ptr;
	bs->end = bs->start + size - 1;
	bs->p = bs->start;
	bs->cb = 0;
	bs->freebits = 8;
	bs->nulls = 0;
}

#define SYNTAX_ELEMENT_U(_name, _enable, _width, _value) { \
	.type = SYNTAX_ELEMENT_TYPE_U, \
	.enable = _enable, \
	.width = _width, \
	.value = _value }
#define SYNTAX_ELEMENT_UE(_name, _enable, _value) { \
	.type = SYNTAX_ELEMENT_TYPE_UE, \
	.enable = _enable, \
	.value = _value }

enum syntax_element_type {
	SYNTAX_ELEMENT_TYPE_U,
	SYNTAX_ELEMENT_TYPE_UE,
	SYNTAX_ELEMENT_TYPE_SE
};

struct syntax_element {
	enum syntax_element_type type;
	bool enable;
	uint8_t width;
	uint32_t value;
};

void avico_bitstream_write_sps_pps(struct avico_ctx *ctx)
{
	int i;
	struct bitstream *const bs = &ctx->bs;

	/* \bug On the stack. Bad idea. */
	struct syntax_element sps_elements[] = {
		SYNTAX_ELEMENT_U("forbidden_zero_bit",   1, 1, 0),
		SYNTAX_ELEMENT_U("nal_ref_idc",          1, 2,
				 NALU_PRIOR_HIGHEST),
		SYNTAX_ELEMENT_U("nal_unit_type",        1, 5, NALU_SPS),

		/* H.264 Profile. 66 for (Constrained) Baseline */
		SYNTAX_ELEMENT_U("profile_idc",          1, 8, 66),
		SYNTAX_ELEMENT_U("constraint_set0_flag", 1, 1, 0),

		/* Constrained profile */
		SYNTAX_ELEMENT_U("constraint_set1_flag", 1, 1, 1),
		SYNTAX_ELEMENT_U("constraint_set2_flag", 1, 1, 0),
		SYNTAX_ELEMENT_U("constraint_set3_flag", 1, 1, 0),
		SYNTAX_ELEMENT_U("constraint_set4_flag", 1, 1, 0),
		SYNTAX_ELEMENT_U("constraint_set5_flag", 1, 1, 0),
		SYNTAX_ELEMENT_U("reserved_zero_2bits",  1, 2, 0),

		/* H.264 Level 4.0 */
		SYNTAX_ELEMENT_U("level_idc",            1, 8, 40),

		/* \todo Missing unessential elements */
		SYNTAX_ELEMENT_UE("seq_parameter_set_id", 1, ctx->sps),
		SYNTAX_ELEMENT_UE("log2_max_frame_num_minus4", 1, 0),
		SYNTAX_ELEMENT_UE("pic_order_cnt_type",   1, ctx->poc_type),
		/* \todo Missing unessential elements */
		SYNTAX_ELEMENT_UE("num_ref_frames",       1, 1),
		SYNTAX_ELEMENT_U("gaps_in_frame_num_value_allowed_flag", 1, 1,
				 0),
		SYNTAX_ELEMENT_UE("pic_width_in_mbs_minus1",        1,
				  ctx->mbx - 1),
		SYNTAX_ELEMENT_UE("pic_height_in_map_units_minus1", 1,
				  ctx->mby - 1),
		SYNTAX_ELEMENT_U("frame_mbs_only_flag", 1, 1, 1),
		/* \todo Missing unessential elements */
		SYNTAX_ELEMENT_U("direct_8x8_inference_flag", 1, 1, 0),

		/* \todo Support crop */
		SYNTAX_ELEMENT_U("frame_cropping_flag", 1, 1, 0),
		/* \todo Missing unessential elements */

		SYNTAX_ELEMENT_U("vui_parameters_present_flag",         1, 1,
				 1),
		SYNTAX_ELEMENT_U("vui_aspect_ratio_present_flag",       1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_video_signal_type_present_flag",  1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_overscan_info_present_flag",      1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_chroma_loc_info_present_flag",    1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_timing_info_present_flag",        1, 1,
				 1),
		SYNTAX_ELEMENT_U("vui_num_units_in_tick",               1, 32,
				 ctx->timeperframe.numerator),
		SYNTAX_ELEMENT_U("vui_time_scale",                      1, 32,
				 ctx->timeperframe.denominator * 2),
		SYNTAX_ELEMENT_U("vui_fixed_frame_rate_flag",           1, 1,
				 1),
		SYNTAX_ELEMENT_U("vui_nal_hrd_parameters_present_flag", 1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_vcl_hrd_parameters_present_flag", 1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_pic_struct_present_flag",         1, 1,
				 0),
		SYNTAX_ELEMENT_U("vui_bitstream_restriction_flag",      1, 1, 0)
	};

	/* Write SPS */
	write_delimiter(bs);

	for (i = 0; i < ARRAY_SIZE(sps_elements); i++) {
		/* \todo Error checking */
		if (!sps_elements[i].enable)
			continue;
		switch (sps_elements[i].type) {
		case SYNTAX_ELEMENT_TYPE_U:
			writeu(bs, sps_elements[i].width,
			       sps_elements[i].value);
			break;
		case SYNTAX_ELEMENT_TYPE_UE:
			writeue(bs, sps_elements[i].value);
			break;
		default:
			/* \todo Error */
			continue;
		}
	}

	write_trailing_bits(bs);

	/* Write PPS */
	write_delimiter(bs);
	writeu(bs, 1, 0); /* forbidden_zero_bit */
	writeu(bs, 2, NALU_PRIOR_HIGHEST);
	writeu(bs, 5, NALU_PPS);

	writeue(bs, ctx->pps); /* pic_parameter_set_id */
	writeue(bs, ctx->sps); /* seq_parameter_set_id */
	writeu(bs, 1, 0); /* entropy_coding_mode_flag. 0 for CAVLC */
	writeu(bs, 1, 0); /* bottom_field_pic_order_in_frame_present_flag */
	writeue(bs, 0);   /* num_slice_groups - 1 */

	writeue(bs, 0);   /* num_ref_idx_l0_default_active - 1 */
	writeue(bs, 0);   /* num_ref_idx_l1_default_active - 1 */
	writeu(bs, 1, 0); /* weighted_pred_flag */
	writeu(bs, 2, 0); /* weighted_bipred_idc */

	ctx->pps_qp = ctx->qp_i;
	writese(bs, ctx->pps_qp - 26); /* pic_init_qp - 26 */
	writese(bs, ctx->pps_qp - 26); /* pic_init_qs - 26 */
	writese(bs, ctx->qpc_offset); /* chroma_qp_index_offset */

	/* \todo DBF should depend on off_a and off_b (see Rolschikov's code */
	writeu(bs, 1, !ctx->dbf);  /* dbf_control_present_flag */
	writeu(bs, 1, 0); /* constrained_intra_pred_flag */
	writeu(bs, 1, 0); /* redundant_pic_cnt_present_flag */

	write_trailing_bits(bs);
}

void avico_bitstream_write_slice_header(struct avico_ctx *ctx)
{
	struct bitstream *bs;

	WARN_ON(ctx == NULL);
	bs = &ctx->bs;

	WARN_ON(bs == NULL);
	WARN_ON(bs->p == NULL);
	WARN_ON(bs->freebits != 8);

	write_delimiter(bs);
	writeu(bs, 1, 0); /* forbidden_zero_bit */
	writeu(bs, 2, ctx->idr ? NALU_PRIOR_HIGHEST : NALU_PRIOR_HIGH);
	writeu(bs, 5, ctx->idr ? NALU_IDR : NALU_SLICE);

	writeue(bs, 0); /* first_mb_addr_in_slice */
	writeue(bs, ctx->frame_type); /* frame_type */
	writeue(bs, ctx->pps);        /* pps_id */
	writeu(bs, 4, ctx->frame); /* frame_num */

	if (ctx->idr)
		writeue(bs, ctx->frame & 0x01); /* idr_pic_id */

	/* POC type is always 2 */
	/* \todo Support different POC types */

	if (ctx->frame_type == VE_FR_P) {
		writeu(bs, 1, 0); /* num_ref_idx_active_override_flag */
		writeu(bs, 1, 0); /* ref_pic_list_l0_reordering_flag */
	}

	if (ctx->idr) {
		writeu(bs, 1, 0); /* no_output_of_prior_pics_flag */
		writeu(bs, 1, 0); /* long_term_reference_flag */
	} else {
		writeu(bs, 1, 0); /* adaptive_ref_pic_buffering_flag */
	}

	if (ctx->frame_type == VE_FR_I)
		writese(bs, ctx->qp_i - ctx->pps_qp); /* slice_qp_delta */
	else
		writese(bs, ctx->qp_p - ctx->pps_qp); /* slice_qp_delta */

	/* \todo DBF should depend on off_a and off_b (see Rolschikov's code */
	if (!ctx->dbf)
		writeue(bs, ctx->dbf ? 0 : 1); /* disable_dbf_flag */
}

void avico_bitstream_get64(struct avico_ctx *ctx, uint32_t data[2],
			   unsigned int bits[2])
{
	struct bitstream *bs = &ctx->bs;
	uint8_t *p = (uint8_t *)((uintptr_t)bs->p & ~0x7);
	int b = (bs->p - p) * 8 + 8 - bs->freebits;

	*bs->p = bs->cb;

	data[0] = data[1] = 0;

	if (b)
		memcpy(data, p, b / 8 + 1);
	bits[0] = min(b, 32);
	bits[1] = max(b - 32, 0);
}

int avico_bitstream_ecd_stuff_pos(struct avico_ctx *ctx)
{
	/* \todo Here I assume that start is at least 8-byte aligned */
	uint8_t *p = ctx->bs.p;
	int pos = (uintptr_t)p % 4;

	/* As our bitstream always start with synchronization marker we can not
	 * cross bitstream buffer boundary. */
	while (pos != -2 && *(--p) == 0)
		pos--;

	return pos;
}

void avico_bitstream_cut64(struct avico_ctx *ctx)
{
	ctx->bs.p = IS_ALIGNED((uintptr_t)ctx->bs.p, 8) ? ctx->bs.p :
			PTR_ALIGN(ctx->bs.p - 8, 8);
	ctx->bs.freebits = 8;
	ctx->bs.cb = 0;
	ctx->bs.nulls = 0;
}

void avico_bitstream_dump(struct bitstream *bs)
{
	size_t len = bs->p - bs->start + (bs->freebits == 8 ? 0 : 1);
	*bs->p = bs->cb;
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 1, bs->start,
		       len, true);
	if (bs->freebits != 8)
		pr_info("Bits in last byte: %u\n", 8 - bs->freebits);
}

