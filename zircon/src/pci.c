// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <efi.h>
#include <libkernelflinger/protocol.h>

#include "zircon.h"

typedef struct {
  uint8_t descriptor;
  uint16_t len;
  uint8_t res_type;
  uint8_t gen_flags;
  uint8_t specific_flags;
  uint64_t addrspace_granularity;
  uint64_t addrrange_minimum;
  uint64_t addrrange_maximum;
  uint64_t addr_tr_offset;
  uint64_t addr_len;
} __attribute__((packed)) acpi_addrspace_desc64_t;

#define ACPI_ADDRESS_SPACE_DESCRIPTOR 0x8A
#define ACPI_END_TAG_DESCRIPTOR 0x79

#define ACPI_ADDRESS_SPACE_TYPE_BUS 0x02

#define PciRootBridgeIoProtocol gEfiPciRootBridgeIoProtocolGuid
efi_status xefi_find_pci_mmio(uint8_t cls, uint8_t sub, uint8_t ifc,
                              uint64_t* mmio) {
  size_t num_handles;
  efi_handle* handles;
  efi_status status =
  status = uefi_call_wrapper(BS->LocateHandleBuffer, 5, ByProtocol,
                          &PciRootBridgeIoProtocol, NULL, &num_handles, &handles);
  if (EFI_ERROR(status)) {
    efi_perror(status, L"Could not find PCI root bridge IO protocol");
    return status;
  }

  for (size_t i = 0; i < num_handles; i++) {
    debug(L"handle %zu\n", i);
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* iodev;
    status = handle_protocol(handles[i], &PciRootBridgeIoProtocol, (VOID**)&iodev);
    if (EFI_ERROR(status)) {
      efi_perror(status, L"Could not get protocol for handle %zu", i);
      continue;
    }
    acpi_addrspace_desc64_t* descriptors;
    status = iodev->Configuration(iodev, (void**)&descriptors);
    if (EFI_ERROR(status)) {
      efi_perror(status, L"Could not get configuration for handle %zu", i);
      continue;
    }

    uint16_t min_bus, max_bus;
    while (descriptors->descriptor != ACPI_END_TAG_DESCRIPTOR) {
      min_bus = (uint16_t)descriptors->addrrange_minimum;
      max_bus = (uint16_t)descriptors->addrrange_maximum;

      if (descriptors->res_type != ACPI_ADDRESS_SPACE_TYPE_BUS) {
        descriptors++;
        continue;
      }

      for (int bus = min_bus; bus <= max_bus; bus++) {
        for (int dev = 0; dev < PCI_MAX_DEVICE; dev++) {
          for (int func = 0; func < PCI_MAX_FUNC; func++) {
            PCI_TYPE00 pci_hdr;
            memset(&pci_hdr, 0, sizeof(pci_hdr));
            uint64_t address = (uint64_t)((bus << 24) + (dev << 16) + (func << 8));
            status =
              iodev->Pci.Read(iodev, EfiPciIoWidthUint16, address, sizeof(pci_hdr) / 2, &pci_hdr);
            if (EFI_ERROR(status)) {
              efi_perror(status, L"could not read pci configuration for bus %d dev %d func %d", bus, dev, func);
              continue;
            }
            if (pci_hdr.Hdr.VendorId == 0xffff)
              break;
            if ((pci_hdr.Hdr.ClassCode[2] == cls) && (pci_hdr.Hdr.ClassCode[1] == sub) &&
                (pci_hdr.Hdr.ClassCode[0] == ifc)) {
              uint64_t n = ((uint64_t)pci_hdr.Device.Bar[0]) | ((uint64_t)pci_hdr.Device.Bar[1]) << 32UL;
              *mmio = n & 0xFFFFFFFFFFFFFFF0UL;
              status = EFI_SUCCESS;
              goto found_it;
            }
            if (!(pci_hdr.Hdr.HeaderType & 0x80) && func == 0) {
              break;
            }
          }
        }
      }
      descriptors++;
    }
  }

  status = EFI_NOT_FOUND;
found_it:
  BS->FreePool(handles);
  return status;
}
