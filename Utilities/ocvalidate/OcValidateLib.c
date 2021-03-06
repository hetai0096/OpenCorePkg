/** @file
  Copyright (C) 2018, vit9696. All rights reserved.
  Copyright (C) 2020, PMheart. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "ocvalidate.h"
#include "OcValidateLib.h"

INT64
GetCurrentTimestamp (
  VOID
  )
{
  struct timeval Time;
  //
  // Get current time.
  //
  gettimeofday (&Time, NULL);
  //
  // Return milliseconds.
  //
  return Time.tv_sec * 1000LL + Time.tv_usec / 1000LL;
}

BOOLEAN
AsciiFileSystemPathIsLegal (
  IN  CONST CHAR8  *Path
  )
{
  UINTN  Index;
  UINTN  PathLength;

  PathLength = AsciiStrLen (Path);

  for (Index = 0; Index < PathLength; ++Index) {
    //
    // Skip allowed characters (0-9, A-Z, a-z, '_', '-', '.', '/', and '\').
    //
    if (IsAsciiNumber (Path[Index])
      || IsAsciiAlpha (Path[Index])
      || Path[Index] == '_'
      || Path[Index] == '-'
      || Path[Index] == '.'
      || Path[Index] == '/'
      || Path[Index] == '\\') {
      continue;
    }

    //
    // Disallowed characters matched.
    //
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
AsciiCommentIsLegal (
  IN  CONST CHAR8  *Comment
  )
{
  UINTN  Index;
  UINTN  CommentLength;

  CommentLength = AsciiStrLen (Comment);

  for (Index = 0; Index < CommentLength; ++Index) {
    //
    // Unprintable characters matched.
    //
    if (IsAsciiPrint (Comment[Index]) == 0) {
      return FALSE;
    }
  }

  return TRUE;
}

BOOLEAN
AsciiIdentifierIsLegal (
  IN  CONST CHAR8    *Identifier,
  IN  BOOLEAN        IsKernelIdentifier
  )
{
  UINTN  Index;
  UINTN  IdentifierLength;

  if (IsKernelIdentifier) {
    //
    // Kernel patch only requires Kernel->Patch->Identifier set to kernel.
    //
    if (AsciiStrCmp (Identifier, "kernel") == 0) {
      return TRUE;
    }
  } else {
    //
    // Any and Apple are two fixed values accepted by Booter->Patch.
    // TODO: Drop empty string support in OC.
    //
    if (AsciiStrCmp (Identifier, "Any") == 0
      || AsciiStrCmp (Identifier, "Apple") == 0) {
      return TRUE;
    }
    //
    // For customised bootloader, it must have .efi suffix.
    //
    if (!OcAsciiEndsWith (Identifier, ".efi", TRUE)) {
      return FALSE;
    }
  }

  //
  // There must be a dot for a sane Identifier.
  //
  if (OcAsciiStrChr (Identifier, '.') == NULL) {
    return FALSE;
  }

  IdentifierLength = AsciiStrLen (Identifier);

  for (Index = 0; Index < IdentifierLength; ++Index) {
    //
    // Skip allowed characters (0-9, A-Z, a-z, '_', '-', and '.').
    // FIXME: Discuss what exactly is legal for identifiers, or update the allowed list on request.
    //
    if (IsAsciiNumber (Identifier[Index])
      || IsAsciiAlpha (Identifier[Index])
      || Identifier[Index] == '_'
      || Identifier[Index] == '-'
      || Identifier[Index] == '.') {
      continue;
    }

    //
    // Disallowed characters matched.
    //
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
AsciiArchIsLegal (
  IN  CONST CHAR8    *Arch,
  IN  BOOLEAN        IsKernelArch
  )
{
  //
  // Special mode for Kernel->Scheme->KernelArch.
  //
  if (IsKernelArch) {
    //
    // Auto and i386-user32 are two special values allowed in KernelArch.
    //
    if (AsciiStrCmp (Arch, "Auto") == 0
      || AsciiStrCmp (Arch, "i386-user32") == 0) {
      return TRUE;
    }
  } else {
    //
    // Any is only allowed in non-KernelArch mode.
    //
    if (AsciiStrCmp (Arch, "Any") == 0) {
      return TRUE;
    }
  }

  //
  // i386 and x86_64 are allowed in both modes.
  // TODO: Do not allow empty string in OC.
  //
  if (AsciiStrCmp (Arch, "i386") != 0
    && AsciiStrCmp (Arch, "x86_64") != 0) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
AsciiPropertyIsLegal (
  IN  CONST CHAR8  *Property
  )
{
  //
  // Like comments, properties can be anything printable.
  // Calling sanitiser for comments to reduce code duplication.
  //
  return AsciiCommentIsLegal (Property);
}

BOOLEAN
AsciiUefiDriverIsLegal (
  IN  CONST CHAR8  *Driver
  )
{
  UINTN  Index;
  UINTN  DriverLength;

  //
  // If an EFI driver does not contain .efi suffix,
  // then it must be illegal.
  //
  if (!OcAsciiEndsWith (Driver, ".efi", TRUE)) {
    return FALSE;
  }

  DriverLength = AsciiStrLen (Driver);

  for (Index = 0; Index < DriverLength; ++Index) {
    //
    // NOTE: Skip '#' as it is treated as comments and thus is legal.
    //
    if (Driver[0] == '#') {
      continue;
    }

    //
    // Skip allowed characters (0-9, A-Z, a-z, '_', '-', '.', '/').
    //
    if (IsAsciiNumber (Driver[Index])
      || IsAsciiAlpha (Driver[Index])
      || Driver[Index] == '_'
      || Driver[Index] == '-'
      || Driver[Index] == '.'
      || Driver[Index] == '/') {
      continue;
    }

    //
    // Disallowed characters matched.
    //
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
AsciiDevicePathIsLegal (
  IN  CONST CHAR8  *AsciiDevicePath
  )
{
  BOOLEAN                   RetVal;
  CHAR16                    *UnicodeDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *TextualDevicePath;

  RetVal = TRUE;

  //
  // Convert ASCII device path to Unicode format.
  //
  UnicodeDevicePath = AsciiStrCopyToUnicode (AsciiDevicePath, 0);
  if (UnicodeDevicePath != NULL) {
    //
    // Firstly, convert Unicode device path to binary.
    //
    DevicePath = ConvertTextToDevicePath (UnicodeDevicePath);
    if (DevicePath != NULL) {
      //
      // Secondly, convert binary back to Unicode device path.
      //
      TextualDevicePath = ConvertDevicePathToText (DevicePath, FALSE, FALSE);
      if (TextualDevicePath != NULL) {
        //
        // If the results before and after conversion do not match,
        // then the original device path is borked.
        //
        if (OcStriCmp (UnicodeDevicePath, TextualDevicePath) != 0) {
          DEBUG ((
            DEBUG_WARN,
            "Original path: %s\nPath after internal conversion: %s\n\n",
            UnicodeDevicePath,
            TextualDevicePath
            ));
          //
          // Do not return immediately in order to free properly.
          //
          RetVal = FALSE;
        }
        FreePool (TextualDevicePath);
      }
      FreePool (DevicePath);
    }
    FreePool (UnicodeDevicePath);
  }

  return RetVal;
}

BOOLEAN
DataHasProperMasking (
  IN  CONST VOID   *Data,
  IN  CONST VOID   *Mask,
  IN  UINTN        Size
  )
{
  CONST UINT8  *ByteData;
  CONST UINT8  *ByteMask;
  UINTN        Index;

  ByteData = Data;
  ByteMask = Mask;

  for (Index = 0; Index < Size; ++Index) {
    if ((ByteData[Index] & ~ByteMask[Index]) != 0) {
      return FALSE;
    }
  }

  return TRUE;
}

UINT32
ValidatePatch (
  IN   CONST   CHAR8   *PatchSection,
  IN   UINT32          PatchIndex,
  IN   BOOLEAN         FindSizeCanBeZero,
  IN   CONST   UINT8   *Find,
  IN   UINT32          FindSize,
  IN   CONST   UINT8   *Replace,
  IN   UINT32          ReplaceSize,
  IN   CONST   UINT8   *Mask,
  IN   UINT32          MaskSize,
  IN   CONST   UINT8   *ReplaceMask,
  IN   UINT32          ReplaceMaskSize
  )
{
  UINT32  ErrorCount;

  ErrorCount = 0;

  if (!FindSizeCanBeZero && FindSize != ReplaceSize) {
    DEBUG ((
      DEBUG_WARN,
      "%a[%u] has different Find and Replace size (%u vs %u)!\n",
      PatchSection,
      PatchIndex,
      FindSize,
      ReplaceSize
      ));
    ++ErrorCount;
  }

  if (MaskSize > 0) {
    if (MaskSize != FindSize) {
      DEBUG ((
        DEBUG_WARN,
        "%a[%u] has Mask set but its size is different from Find (%u vs %u)!\n",
        PatchSection,
        PatchIndex,
        MaskSize,
        FindSize
        ));
      ++ErrorCount;
    } else if (!DataHasProperMasking (Find, Mask, FindSize)) {
      DEBUG ((
        DEBUG_WARN,
        "%a[%u]->Find requires Mask to be active for corresponding bits!\n",
        PatchSection,
        PatchIndex
        ));
      ++ErrorCount;
    }
  }
  
  if (ReplaceMaskSize > 0) {
    if (ReplaceMaskSize != ReplaceSize) {
      DEBUG ((
        DEBUG_WARN,
        "%a[%u] has ReplaceMask set but its size is different from Replace (%u vs %u)!\n",
        PatchSection,
        PatchIndex,
        ReplaceMaskSize,
        ReplaceSize
        ));
      ++ErrorCount;
    } else if (!DataHasProperMasking (Replace, ReplaceMask, ReplaceSize)) {
      DEBUG ((
        DEBUG_WARN,
        "%a[%u]->Replace requires ReplaceMask to be active for corresponding bits!\n",
        PatchSection,
        PatchIndex
        ));
      ++ErrorCount;
    }
  }

  return ErrorCount;
}

UINT32
ReportError (
  IN  CONST CHAR8  *FuncName,
  IN  UINT32       ErrorCount
  )
{
  if (ErrorCount != 0) {
    DEBUG ((DEBUG_WARN, "%a returns %u %a!\n", FuncName, ErrorCount, ErrorCount > 1 ? "errors" : "error"));
  } else {
    DEBUG ((DEBUG_VERBOSE, "%a returns no errors!\n", FuncName));
  }

  return ErrorCount;
}
