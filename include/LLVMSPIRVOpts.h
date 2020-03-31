//===- LLVMSPIRVOpts.h - Specify options for translation --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright (c) 2019 Intel Corporation. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimers.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimers in the documentation
// and/or other materials provided with the distribution.
// Neither the names of Advanced Micro Devices, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from this
// Software without specific prior written permission.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
// THE SOFTWARE.
//
//===----------------------------------------------------------------------===//
/// \file LLVMSPIRVOpts.h
///
/// This files declares helper classes to handle SPIR-V versions and extensions.
///
//===----------------------------------------------------------------------===//
#ifndef SPIRV_LLVMSPIRVOPTS_H
#define SPIRV_LLVMSPIRVOPTS_H

#include <cassert>
#include <cstdint>
#include <map>
#include <unordered_map>

namespace SPIRV {

enum class VersionNumber : uint32_t {
  // See section 2.3 of SPIR-V spec: Physical Layout of a SPIR_V Module and
  // Instruction
  SPIRV_1_0 = 0x00010000,
  SPIRV_1_1 = 0x00010100,
  // TODO: populate this enum with the latest versions (up to 1.4) once
  // translator get support of correponding features
  MinimumVersion = SPIRV_1_0,
  MaximumVersion = SPIRV_1_1
};

enum class ExtensionID : uint32_t {
  First,
#define EXT(X) X,
#include "LLVMSPIRVExtensions.inc"
#undef EXT
  Last,
};

enum class BIsRepresentation : uint32_t {
  OpenCL12,
  OpenCL20
  // TODO: consider targeting SPIR-V friendly IR for some non-OpenCL backends,
  // if there are any
};

/// \brief Helper class to manage SPIR-V translation
class TranslatorOpts {
public:
  using ExtensionsStatusMap = std::map<ExtensionID, bool>;

  TranslatorOpts() = default;

  TranslatorOpts(VersionNumber Max, const ExtensionsStatusMap &Map = {},
                 bool ArgNameMD = false)
      : MaxVersion(Max), ExtStatusMap(Map), GenKernelArgNameMD(ArgNameMD) {}

  bool isAllowedToUseVersion(VersionNumber RequestedVersion) const {
    return RequestedVersion <= MaxVersion;
  }

  bool isAllowedToUseExtension(ExtensionID Extension) const {
    auto I = ExtStatusMap.find(Extension);
    if (ExtStatusMap.end() == I)
      return false;

    return I->second;
  }

  VersionNumber getMaxVersion() const { return MaxVersion; }

  bool isGenArgNameMDEnabled() const { return GenKernelArgNameMD; }

  void enableAllExtensions() {
#define EXT(X) ExtStatusMap[ExtensionID::X] = true;
#include "LLVMSPIRVExtensions.inc"
#undef EXT
  }

  void enableGenArgNameMD() { GenKernelArgNameMD = true; }

  void setSpecConst(uint32_t SpecId, uint64_t SpecValue) {
    ExternalSpecialization[SpecId] = SpecValue;
  }

  bool getSpecializationConstant(uint32_t SpecId, uint64_t &Value) const {
    auto It = ExternalSpecialization.find(SpecId);
    if (It == ExternalSpecialization.end())
      return false;
    Value = It->second;
    return true;
  }

  void setDesiredBIsRepresentation(BIsRepresentation Value) {
    DesiredRepresentationOfBIs = Value;
  }

  BIsRepresentation getDesiredBIsRepresentation() const {
    return DesiredRepresentationOfBIs;
  }

private:
  // Common translation options
  VersionNumber MaxVersion = VersionNumber::MaximumVersion;
  ExtensionsStatusMap ExtStatusMap;
  // SPIR-V to LLVM translation options
  bool GenKernelArgNameMD = false;
  std::unordered_map<uint32_t, uint64_t> ExternalSpecialization;
  // Version of OpenCL C, which should be used while translating from SPIR-V to
  // back to LLVM IR
  BIsRepresentation DesiredRepresentationOfBIs = BIsRepresentation::OpenCL12;
};

} // namespace SPIRV

#endif // SPIRV_LLVMSPIRVOPTS_H
