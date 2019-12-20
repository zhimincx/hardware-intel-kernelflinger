// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "osboot.h"

static efi_guid AcpiTableGUID = ACPI_TABLE_GUID;
static efi_guid Acpi2TableGUID = ACPI_20_TABLE_GUID;
static efi_guid SmbiosTableGUID = SMBIOS_TABLE_GUID;
static efi_guid Smbios3TableGUID = SMBIOS3_TABLE_GUID;
static uint8_t ACPI_RSD_PTR[8] = "RSD PTR ";
static uint8_t SmbiosAnchor[4] = "_SM_";
static uint8_t Smbios3Anchor[5] = "_SM3_";

uint64_t find_acpi_root(efi_handle img, efi_system_table* sys) {
  efi_configuration_table* cfgtab = sys->ConfigurationTable;
  for (size_t i = 0; i < sys->NumberOfTableEntries; i++) {
    if (CompareGuid(&cfgtab[i].VendorGuid, &AcpiTableGUID) &&
        CompareGuid(&cfgtab[i].VendorGuid, &Acpi2TableGUID)) {
      // not an ACPI table
      continue;
    }
    if (memcmp(cfgtab[i].VendorTable, ACPI_RSD_PTR, 8)) {
      // not the Root Description Pointer
      continue;
    }
    return (uint64_t)cfgtab[i].VendorTable;
  }
  return 0;
}

uint64_t find_smbios(efi_handle img, efi_system_table* sys) {
  efi_configuration_table* cfgtab = sys->ConfigurationTable;
  for (size_t i = 0; i < sys->NumberOfTableEntries; i++) {
    if (!CompareGuid(&cfgtab[i].VendorGuid, &SmbiosTableGUID)) {
      if (!memcmp(cfgtab[i].VendorTable, SmbiosAnchor, sizeof(SmbiosAnchor))) {
        return (uint64_t)cfgtab[i].VendorTable;
      }
    } else if (!CompareGuid(&cfgtab[i].VendorGuid, &Smbios3TableGUID)) {
      if (!memcmp(cfgtab[i].VendorTable, Smbios3Anchor, sizeof(Smbios3Anchor))) {
        return (uint64_t)cfgtab[i].VendorTable;
      }
    }
  }
  return 0;
}

static void get_bit_range(uint32_t mask, int* high, int* low) {
  *high = -1;
  *low = -1;
  int idx = 0;
  while (mask) {
    if (*low < 0 && (mask & 1))
      *low = idx;
    idx++;
    mask >>= 1;
  }
  *high = idx - 1;
}
