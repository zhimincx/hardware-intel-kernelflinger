/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ANDROID_VB1_H_
#define _ANDROID_VB1_H_

#include <openssl/x509.h>
#include "targets.h"

typedef X509 VBDATA;

EFI_STATUS prepend_slot_command_line(CHAR16 **cmdline16,
        enum boot_target boot_target,
        VBDATA *vb_data);

UINTN get_vb_cmdlen(VBDATA *vb_data);

char *get_vb_cmdline(VBDATA *vb_data);

#endif
