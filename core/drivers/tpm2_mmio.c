/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 *
 */

#include <assert.h>
#include <delay.h>
#include <initcall.h>
#include <io.h>
#include <keep.h>
#include <kernel/panic.h>
#include <kernel/tee_time.h>
#include <platform_config.h>
#include <tpm2.h>
#include <tpm2_mmio.h>
#include <trace.h>
#include <util.h>

