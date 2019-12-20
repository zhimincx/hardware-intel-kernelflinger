#ifndef ZIRCON_BOOTLOADER_INCLUDE_ZIRCON_H_
#define ZIRCON_BOOTLOADER_INCLUDE_ZIRCON_H_

#include <efi.h>
#include <lib.h>

#define efi_handle EFI_HANDLE
#define efi_guid EFI_GUID
#define efi_status EFI_STATUS

#define efi_system_table EFI_SYSTEM_TABLE
#define efi_configuration_table EFI_CONFIGURATION_TABLE
#define efi_boot_services EFI_BOOT_SERVICES

#define efi_memory_descriptor EFI_MEMORY_DESCRIPTOR
#define efi_runtime_services EFI_RUNTIME_SERVICES

#define PciRootBridgeIoProtocol gEfiPciRootBridgeIoProtocolGuid

typedef uint64_t efi_physical_addr;
typedef uint64_t efi_virtual_addr;

#define KERNEL_ZONE_BASE 0x100000
#define KERNEL_ZONE_SIZE (6 * 1024 * 1024)

#endif  // ZIRCON_BOOTLOADER_INCLUDE_CTYPE_H_
