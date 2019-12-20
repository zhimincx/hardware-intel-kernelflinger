// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ZIRCON_BOOTLOADER_SRC_OSBOOT_H_
#define ZIRCON_BOOTLOADER_SRC_OSBOOT_H_

#include <stdint.h>

#include "zircon.h"

#define PAGE_SIZE (4096)
#define PAGE_MASK (PAGE_SIZE - 1)

#define BYTES_TO_PAGES(n) (((n) + PAGE_MASK) / PAGE_SIZE)

// Ensure there are some pages preceding the
// Ramdisk so that the kernel start code can
// use them to prepend bootdata items if desired.
#define FRONT_PAGES (8)
#define FRONT_BYTES (PAGE_SIZE * FRONT_PAGES)

#define CMDLINE_MAX PAGE_SIZE

efi_status boot_kernel(efi_handle img, efi_system_table* sys, void* image, size_t sz, void* ramdisk,
                size_t rsz);

uint64_t find_acpi_root(efi_handle img, efi_system_table* sys);
uint64_t find_smbios(efi_handle img, efi_system_table* sys);

efi_status zedboot(efi_handle img, efi_system_table* sys, void* image, size_t sz);

#define IMAGE_INVALID 0
#define IMAGE_EMPTY 1
#define IMAGE_KERNEL 2
#define IMAGE_RAMDISK 3
#define IMAGE_COMBO 4

unsigned identify_image(void* image, size_t sz);

// sz may be just one block or sector
// if the header looks like a kernel image, return expected size
// otherwise returns 0
size_t image_getsize(void* imageheader, size_t sz);


#endif  // ZIRCON_BOOTLOADER_SRC_OSBOOT_H_
