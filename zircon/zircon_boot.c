#include <efi.h>
#include <libkernelflinger/targets.h>
#include <libkernelflinger/protocol.h>
#include <libkernelflinger/uefi_utils.h>
#include <libkernelflinger/pci.h>
#include <log.h>

#include "osboot.h"
#include "zircon_image.h"
#include <cmdline.h>
#include <libkernelflinger/gpt.h>

static EFI_STATUS zircon_read_cmdline(CHAR16 *file, OUT CHAR8 **data, OUT UINTN *size)
{
	EFI_STATUS ret;
	EFI_FILE_IO_INTERFACE *io;
	EFI_LOADED_IMAGE *g_loaded_image = NULL;

	uefi_call_wrapper(BS->HandleProtocol, 3, g_parent_image, &LoadedImageProtocol, (void **)&g_loaded_image);
	ret = handle_protocol(g_loaded_image->DeviceHandle, &FileSystemProtocol, (void **)&io);
	if (EFI_ERROR(ret))
		return ret;

	ret = uefi_read_file(io, file, (VOID **)data, size);
	if (EFI_ERROR(ret)) {
		error(L"read file failed: %s", file);
		return ret;
	}

	debug(L"file size of '%s' = %d\n", file, *size);
	return ret;
}

static EFI_STATUS zircon_image_load_partition(EFI_GUID *guid, const CHAR16 *label, OUT VOID **image, OUT UINTN *image_size)
{
        UINT32 MediaId;
	UINTN zircon_img_sz;
	VOID *zircon_img;
	EFI_STATUS ret;
	zircon_kernel_t zircon_kernel;
	struct gpt_partition_interface gpart;
        UINT64 partition_start;

	ret = gpt_get_partition_by_label(label, &gpart, LOGICAL_UNIT_USER);
	if (EFI_ERROR(ret)) {
		efi_perror(ret, L"Partition %s not found", label);
		return ret;
	}
        MediaId = gpart.bio->Media->MediaId;
        partition_start = gpart.part.starting_lba * gpart.bio->Media->BlockSize;

        debug(L"Reading zircon image header");
        ret = uefi_call_wrapper(gpart.dio->ReadDisk, 5, gpart.dio, MediaId,
                                partition_start,
                                sizeof(zircon_kernel), &zircon_kernel);
        if (EFI_ERROR(ret)) {
                efi_perror(ret, L"ReadDisk (header)");
                return ret;
        }

	zircon_img_sz = image_getsize(&zircon_kernel, sizeof(zircon_kernel_t));

	zircon_img = AllocatePool(zircon_img_sz);
	if (!zircon_img) {
		error(L"Alloc memory for %s image failed", label);
		return EFI_OUT_OF_RESOURCES;
	}
	debug(L"Reading %s image: %d bytes", label, zircon_img_sz);
	ret = uefi_call_wrapper(gpart.dio->ReadDisk, 5, gpart.dio, MediaId,
				partition_start, zircon_img_sz, zircon_img);
	if (EFI_ERROR(ret)) {
		efi_perror(ret, L"ReadDisk Error for %s image read", label);
		FreePool(zircon_img);
		return ret;
	}
	*image = zircon_img;
	*image_size = zircon_img_sz;
	return EFI_SUCCESS;
}

#define ZIRCON_A_GUID { 0xde30cc86, 0x1f4a, 0x4a31, \
    {0x93, 0xc4, 0x66, 0xf1, 0x47, 0xd3, 0x3e, 0x05} }
#define ZIRCON_A_NAME L"ZIRCON-A"

#define ZIRCON_B_GUID { 0x23cc04df, 0xc278, 0x4ce7, \
    {0x84, 0x71, 0x89, 0x7d, 0x1a, 0x4b, 0xcd, 0xf7} }
#define ZIRCON_B_NAME L"ZIRCON-B"

#define ZIRCON_R_GUID { 0xa0e5cf57, 0x2def, 0x46be, \
    {0xa8, 0x0c, 0xa2, 0x06, 0x7c, 0x37, 0xcd, 0x49} }
#define ZIRCON_R_NAME L"ZIRCON-R"

static EFI_STATUS load_zircon_image(enum boot_target boot_target, OUT VOID **zircon_img, OUT UINTN *zircon_img_sz)
{
	EFI_STATUS ret = EFI_NOT_FOUND;
	const EFI_GUID zircon_a_guid = ZIRCON_A_GUID;
	const EFI_GUID zircon_r_guid = ZIRCON_R_GUID;

	if (boot_target == NORMAL_BOOT) {
		ret = zircon_image_load_partition((EFI_GUID *)&zircon_a_guid, ZIRCON_A_NAME, zircon_img, zircon_img_sz);
	} else if (boot_target == RECOVERY) {
		ret = zircon_image_load_partition((EFI_GUID *)&zircon_r_guid, ZIRCON_R_NAME, zircon_img, zircon_img_sz);
	}
	if ((*zircon_img) != NULL)
		ret = EFI_SUCCESS;

	return ret;
}

EFI_STATUS zircon_boot(EFI_HANDLE image, EFI_SYSTEM_TABLE *sys_table, enum boot_target boot_target)
{
	EFI_STATUS ret;
	VOID *zircon_img = NULL;
	UINTN zircon_img_sz = 0;

	/* PCI Device (0x0C, 0x03, 0x30): USB3 XHCI Controller.
	 * Map PCIe configuration to memory address space (of the CPU) */
	UINT64 mmio;
	if (xefi_find_pci_mmio(0x0C, 0x03, 0x30, &mmio) == EFI_SUCCESS) {
		CHAR8 tmp[32];
		efi_snprintf(tmp, sizeof(tmp), (CHAR8*)"%#llx", mmio);
		cmdline_set("xdc.mmio", tmp);
	}

	// Load the cmdline
	UINTN csz = 0;
	CHAR8 *cmdline_file;
	ret = zircon_read_cmdline(L"cmdline", &cmdline_file, &csz);
	if (EFI_ERROR(ret) || (cmdline_file == NULL)) {
		efi_perror(ret, L"Failed to get cmdline");
	} else {
		cmdline_append(cmdline_file, csz);
	}

	ret = load_zircon_image(boot_target, &zircon_img, &zircon_img_sz);
	if (EFI_ERROR(ret)) {
		efi_perror(ret, L"issue loading boot image");
		return ret;
	}

	ret = zedboot(image, sys_table, zircon_img, zircon_img_sz);
	if (EFI_ERROR(ret)) {
		efi_perror(ret, L"Failed to start boot image");
		return ret;
	}
	return ret;
}
