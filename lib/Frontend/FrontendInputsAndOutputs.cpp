//===--- FrontendInputsAndOutputs.cpp -------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "swift/Frontend/FrontendInputsAndOutputs.h"

#include "swift/AST/DiagnosticsFrontend.h"
#include "swift/Basic/FileTypes.h"
#include "swift/Basic/PrimarySpecificPaths.h"
#include "swift/Basic/Range.h"
#include "swift/Frontend/FrontendOptions.h"
#include "swift/Option/Options.h"
#include "swift/Parse/Lexer.h"
#include "swift/Strings.h"
#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/Path.h"

using namespace swift;
using namespace llvm::opt;

// Constructors

FrontendInputsAndOutputs::FrontendInputsAndOutputs(
    const FrontendInputsAndOutputs &other) {
  for (InputFile input : other.AllInputs)
    addInput(input);
  IsSingleThreadedWMO = other.IsSingleThreadedWMO;
  ShouldRecoverMissingInputs = other.ShouldRecoverMissingInputs;
}

FrontendInputsAndOutputs &FrontendInputsAndOutputs::
operator=(const FrontendInputsAndOutputs &other) {
  clearInputs();
  for (InputFile input : other.AllInputs)
    addInput(input);
  IsSingleThreadedWMO = other.IsSingleThreadedWMO;
  ShouldRecoverMissingInputs = other.ShouldRecoverMissingInputs;
  return *this;
}

// All inputs:

std::vector<std::string> FrontendInputsAndOutputs::getInputFilenames() const {
  std::vector<std::string> filenames;
  for (auto &input : AllInputs) {
    filenames.push_back(input.getFileName());
  }
  return filenames;
}

bool FrontendInputsAndOutputs::isReadingFromStdin() const {
  return hasSingleInput() && getFilenameOfFirstInput() == "-";
}

const std::string &FrontendInputsAndOutputs::getFilenameOfFirstInput() const {
  assert(hasInputs());
  const InputFile &inp = AllInputs[0];
  const std::string &f = inp.getFileName();
  assert(!f.empty());
  return f;
}

bool FrontendInputsAndOutputs::forEachInput(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  for (const InputFile &input : AllInputs)
    if (fn(input))
      return true;
  return false;
}

// Primaries:

const InputFile &FrontendInputsAndOutputs::firstPotentialPrimaryInput() const {
  assert(!PotentialPrimaryInputsInOrder.empty());
  return AllInputs[PotentialPrimaryInputsInOrder.front()];
}

const InputFile &FrontendInputsAndOutputs::firstCurrentPrimaryInput() const {
  assert(!CurrentPrimaryInputsInOrder.empty());
  return AllInputs[CurrentPrimaryInputsInOrder.front()];
}

const InputFile &FrontendInputsAndOutputs::lastPotentialPrimaryInput() const {
  assert(!PotentialPrimaryInputsInOrder.empty());
  return AllInputs[PotentialPrimaryInputsInOrder.back()];
}

const InputFile &FrontendInputsAndOutputs::lastCurrentPrimaryInput() const {
  assert(!CurrentPrimaryInputsInOrder.empty());
  return AllInputs[CurrentPrimaryInputsInOrder.back()];
}



bool FrontendInputsAndOutputs::forEachPotentialPrimaryInput(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  for (unsigned i : PotentialPrimaryInputsInOrder)
    if (fn(AllInputs[i]))
      return true;
  return false;
}

bool FrontendInputsAndOutputs::forEachCurrentPrimaryInput(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  for (unsigned i : PotentialPrimaryInputsInOrder)
    if (fn(AllInputs[i]))
      return true;
  return false;
}


bool FrontendInputsAndOutputs::forEachCurrentPrimaryInputWithIndex(
    llvm::function_ref<bool(const InputFile &, unsigned index)> fn) const {
  for (unsigned i : CurrentPrimaryInputsInOrder)
    if (fn(AllInputs[i], i))
      return true;
  return false;
}

bool FrontendInputsAndOutputs::forEachNonPotentialPrimaryInput(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  return forEachInput([&](const InputFile &f) -> bool {
    return f.isPotentialPrimaryInput() ? false : fn(f);
  });
}

void FrontendInputsAndOutputs::assertMustNotBeMoreThanOnePrimaryInput() const {
  assert(!hasMultiplePotentialPrimaryInputs() &&
         "have not implemented >1 primary input yet");
}

void FrontendInputsAndOutputs::
    assertMustNotBeMoreThanOnePrimaryInputUnlessBatchModeChecksHaveBeenBypassed()
        const {
  if (!areBatchModeChecksBypassed())
    assertMustNotBeMoreThanOnePrimaryInput();
}

const InputFile *FrontendInputsAndOutputs::getUniquePotentialPrimaryInput() const {
  assertMustNotBeMoreThanOnePrimaryInput();
  return PotentialPrimaryInputsInOrder.empty()
             ? nullptr
             : &AllInputs[PotentialPrimaryInputsInOrder.front()];
}

const InputFile &
FrontendInputsAndOutputs::getRequiredUniquePrimaryInput() const {
  if (const auto *input = getUniquePotentialPrimaryInput())
    return *input;
  llvm_unreachable("No primary when one is required");
}

std::string FrontendInputsAndOutputs::getStatsFileMangledInputName() const {
  // Use the first primary, even if there are multiple primaries.
  // That's enough to keep the file names unique.
  return isWholeModule() ? "all" : firstPotentialPrimaryInput().getFileName();
}

bool FrontendInputsAndOutputs::isInputPrimary(StringRef file) const {
  return primaryInputNamed(file) != nullptr;
}

unsigned FrontendInputsAndOutputs::numberOfPrimaryInputsEndingWith(
    StringRef extension) const {
  unsigned n = 0;
  (void)forEachPotentialPrimaryInput([&](const InputFile &input) -> bool {
    if (llvm::sys::path::extension(input.getFileName()).endswith(extension))
      ++n;
    return false;
  });
  return n;
}

// Input queries

bool FrontendInputsAndOutputs::shouldTreatAsLLVM() const {
  if (hasSingleInput()) {
    StringRef InputExt = llvm::sys::path::extension(getFilenameOfFirstInput());
    switch (file_types::lookupTypeForExtension(InputExt)) {
    case file_types::TY_LLVM_BC:
    case file_types::TY_LLVM_IR:
      return true;
    default:
      return false;
    }
  }
  return false;
}

bool FrontendInputsAndOutputs::shouldTreatAsModuleInterface() const {
  if (!hasSingleInput())
    return false;

  StringRef InputExt = llvm::sys::path::extension(getFilenameOfFirstInput());
  file_types::ID InputType = file_types::lookupTypeForExtension(InputExt);
  return InputType == file_types::TY_SwiftModuleInterfaceFile;
}

bool FrontendInputsAndOutputs::shouldTreatAsSIL() const {
  if (hasSingleInput()) {
    // If we have exactly one input filename, and its extension is "sil",
    // treat the input as SIL.
    StringRef extension = llvm::sys::path::extension(getFilenameOfFirstInput());
    return file_types::lookupTypeForExtension(extension) == file_types::TY_SIL;
  }
  // If we have one primary input and it's a filename with extension "sil",
  // treat the input as SIL.
  const unsigned silPrimaryCount = numberOfPrimaryInputsEndingWith(
      file_types::getExtension(file_types::TY_SIL));
  if (silPrimaryCount == 0)
    return false;
  if (silPrimaryCount == potentialPrimaryInputCount()) {
    // Not clear what to do someday with multiple primaries
    assertMustNotBeMoreThanOnePrimaryInput();
    return true;
  }
  llvm_unreachable("Either all primaries or none must end with .sil");
}

bool FrontendInputsAndOutputs::shouldTreatAsObjCHeader() const {
  if (hasSingleInput()) {
    StringRef InputExt = llvm::sys::path::extension(getFilenameOfFirstInput());
    switch (file_types::lookupTypeForExtension(InputExt)) {
    case file_types::TY_ObjCHeader:
      return true;
    default:
      return false;
    }
  }
  return false;
}

bool FrontendInputsAndOutputs::areAllNonPrimariesSIB() const {
  for (const InputFile &input : AllInputs) {
    if (input.isPotentialPrimaryInput())
      continue;
    StringRef extension = llvm::sys::path::extension(input.getFileName());
    if (file_types::lookupTypeForExtension(extension) != file_types::TY_SIB) {
      return false;
    }
  }
  return true;
}

bool FrontendInputsAndOutputs::verifyInputs(DiagnosticEngine &diags,
                                            bool treatAsSIL,
                                            bool isREPLRequested,
                                            bool isNoneRequested) const {
  if (isREPLRequested) {
    if (hasInputs()) {
      diags.diagnose(SourceLoc(), diag::error_repl_requires_no_input_files);
      return true;
    }
  } else if (treatAsSIL) {
    if (isWholeModule()) {
      if (inputCount() != 1) {
        diags.diagnose(SourceLoc(), diag::error_mode_requires_one_input_file);
        return true;
      }
    } else {
      assertMustNotBeMoreThanOnePrimaryInput();
      // If we have the SIL as our primary input, we can waive the one file
      // requirement as long as all the other inputs are SIBs.
      if (!areAllNonPrimariesSIB()) {
        diags.diagnose(SourceLoc(),
                       diag::error_mode_requires_one_sil_multi_sib);
        return true;
      }
    }
  } else if (!isNoneRequested && !hasInputs()) {
    diags.diagnose(SourceLoc(), diag::error_mode_requires_an_input_file);
    return true;
  }
  return false;
}

// Changing inputs

void FrontendInputsAndOutputs::clearInputs() {
  AllInputs.clear();
  PotentialPrimaryInputsByName.clear();
  PotentialPrimaryInputsInOrder.clear();
}

void FrontendInputsAndOutputs::addInput(const InputFile &input) {
  const unsigned index = AllInputs.size();
  AllInputs.push_back(input);
  if (input.isCurrentPrimaryInput()) {
    CurrentPrimaryInputsInOrder.push_back(index);
  }
  if (input.isPotentialPrimaryInput()) {
    PotentialPrimaryInputsInOrder.push_back(index);
    PotentialPrimaryInputsByName.insert({AllInputs.back().getFileName(), index});
  }

}

void FrontendInputsAndOutputs::addInputFile(StringRef file,
                                            llvm::MemoryBuffer *buffer) {
  addInput(InputFile(file, false, false, buffer));
}

void FrontendInputsAndOutputs::addPrimaryInputFile(StringRef file,
                                                   llvm::MemoryBuffer *buffer) {
  addInput(InputFile(file, false, true, buffer));
}

// Outputs

unsigned FrontendInputsAndOutputs::countOfInputsProducingMainOutputs() const {
  return isSingleThreadedWMO()
             ? 1
             : hasPotentialPrimaryInputs() ? potentialPrimaryInputCount() : inputCount();
}

const InputFile &FrontendInputsAndOutputs::firstInputPotentiallyProducingOutput() const {
  return isSingleThreadedWMO()
             ? firstInput()
             : hasPotentialPrimaryInputs() ? firstPotentialPrimaryInput() : firstInput();
}

const InputFile &FrontendInputsAndOutputs::lastInputPotentiallyProducingOutput() const {
  return isSingleThreadedWMO()
             ? firstInput()
             : hasPotentialPrimaryInputs() ? lastPotentialPrimaryInput() : lastInput();
}

bool FrontendInputsAndOutputs::forEachInputProducingAMainOutputFile(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  return isSingleThreadedWMO()
             ? fn(firstInput())
             : hasPotentialPrimaryInputs() ? forEachPotentialPrimaryInput(fn) : forEachInput(fn);
}

void FrontendInputsAndOutputs::setMainAndSupplementaryOutputs(
    ArrayRef<std::string> outputFiles,
    ArrayRef<SupplementaryOutputPaths> supplementaryOutputs,
    ArrayRef<std::string> outputFilesForIndexUnits) {
  if (outputFilesForIndexUnits.empty())
    outputFilesForIndexUnits = outputFiles;

  assert(outputFiles.size() == outputFilesForIndexUnits.size() &&
         "Must have one index unit output path per main output");

  if (AllInputs.empty()) {
    assert(outputFiles.empty() && "Cannot have outputs without inputs");
    assert(supplementaryOutputs.empty() &&
           "Cannot have supplementary outputs without inputs");
    return;
  }
  if (hasPotentialPrimaryInputs()) {
    const auto N = potentialPrimaryInputCount();
    assert(outputFiles.size() == N && "Must have one main output per primary");
    assert(supplementaryOutputs.size() == N &&
           "Must have one set of supplementary outputs per primary");
    (void)N;

    unsigned i = 0;
    for (auto &input : AllInputs) {
      if (input.isPotentialPrimaryInput()) {
        input.setPrimarySpecificPaths(PrimarySpecificPaths(
            outputFiles[i], outputFilesForIndexUnits[i], input.getFileName(),
            supplementaryOutputs[i]));
        ++i;
      }
    }
    return;
  }
  assert(supplementaryOutputs.size() == 1 &&
         "WMO only ever produces one set of supplementary outputs");
  if (outputFiles.size() == 1) {
    AllInputs.front().setPrimarySpecificPaths(PrimarySpecificPaths(
        outputFiles.front(), outputFilesForIndexUnits.front(),
        firstInputPotentiallyProducingOutput().getFileName(),
        supplementaryOutputs.front()));
    return;
  }
  assert(outputFiles.size() == AllInputs.size() &&
         "Multi-threaded WMO requires one main output per input");
  for (auto i : indices(AllInputs))
    AllInputs[i].setPrimarySpecificPaths(PrimarySpecificPaths(
        outputFiles[i], outputFilesForIndexUnits[i], outputFiles[i],
        i == 0 ? supplementaryOutputs.front() : SupplementaryOutputPaths()));
}

std::vector<std::string> FrontendInputsAndOutputs::copyOutputFilenames() const {
  std::vector<std::string> outputs;
  (void)forEachInputProducingAMainOutputFile(
      [&](const InputFile &input) -> bool {
        outputs.push_back(input.outputFilename());
        return false;
      });
  return outputs;
}

std::vector<std::string>
FrontendInputsAndOutputs::copyIndexUnitOutputFilenames() const {
  std::vector<std::string> outputs;
  (void)forEachInputProducingAMainOutputFile(
      [&](const InputFile &input) -> bool {
        outputs.push_back(input.indexUnitOutputFilename());
        return false;
      });
  return outputs;
}

void FrontendInputsAndOutputs::forEachOutputFilename(
    llvm::function_ref<void(StringRef)> fn) const {
  (void)forEachInputProducingAMainOutputFile(
      [&](const InputFile &input) -> bool {
        fn(input.outputFilename());
        return false;
      });
}

std::string FrontendInputsAndOutputs::getSingleOutputFilename() const {
  assertMustNotBeMoreThanOnePrimaryInputUnlessBatchModeChecksHaveBeenBypassed();
  return hasInputs() ? lastInputPotentiallyProducingOutput().outputFilename()
                     : std::string();
}

std::string FrontendInputsAndOutputs::getSingleIndexUnitOutputFilename() const {
  assertMustNotBeMoreThanOnePrimaryInputUnlessBatchModeChecksHaveBeenBypassed();
  return hasInputs() ? lastInputPotentiallyProducingOutput().indexUnitOutputFilename()
                     : std::string();
}

bool FrontendInputsAndOutputs::isOutputFilenameStdout() const {
  return getSingleOutputFilename() == "-";
}

bool FrontendInputsAndOutputs::isOutputFileDirectory() const {
  return hasNamedOutputFile() &&
         llvm::sys::fs::is_directory(getSingleOutputFilename());
}

bool FrontendInputsAndOutputs::hasNamedOutputFile() const {
  return hasInputs() && !isOutputFilenameStdout();
}

// Supplementary outputs

unsigned
FrontendInputsAndOutputs::countOfFilesProducingSupplementaryOutput() const {
  return hasPotentialPrimaryInputs() ? potentialPrimaryInputCount() : hasInputs() ? 1 : 0;
}

bool FrontendInputsAndOutputs::forEachInputProducingSupplementaryOutput(
    llvm::function_ref<bool(const InputFile &)> fn) const {
  return hasPotentialPrimaryInputs() ? forEachPotentialPrimaryInput(fn)
                            : hasInputs() ? fn(firstInput()) : false;
}

bool FrontendInputsAndOutputs::hasSupplementaryOutputPath(
    llvm::function_ref<const std::string &(const SupplementaryOutputPaths &)>
        extractorFn) const {
  return forEachInputProducingSupplementaryOutput([&](const InputFile &input)
                                                      -> bool {
    return !extractorFn(input.getPrimarySpecificPaths().SupplementaryOutputs)
                .empty();
  });
}

bool FrontendInputsAndOutputs::hasDependenciesPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.DependenciesFilePath;
      });
}
bool FrontendInputsAndOutputs::hasReferenceDependenciesPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ReferenceDependenciesFilePath;
      });
}
bool FrontendInputsAndOutputs::hasObjCHeaderOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ObjCHeaderOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasLoadedModuleTracePath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.LoadedModuleTracePath;
      });
}
bool FrontendInputsAndOutputs::hasModuleOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ModuleOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasModuleDocOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ModuleDocOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasModuleSourceInfoOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ModuleSourceInfoOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasModuleInterfaceOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ModuleInterfaceOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasPrivateModuleInterfaceOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.PrivateModuleInterfaceOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasModuleSummaryOutputPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.ModuleSummaryOutputPath;
      });
}
bool FrontendInputsAndOutputs::hasTBDPath() const {
  return hasSupplementaryOutputPath(
      [](const SupplementaryOutputPaths &outs) -> const std::string & {
        return outs.TBDPath;
      });
}

bool FrontendInputsAndOutputs::hasDependencyTrackerPath() const {
  return hasDependenciesPath() || hasReferenceDependenciesPath() ||
         hasLoadedModuleTracePath();
}

const PrimarySpecificPaths &
FrontendInputsAndOutputs::getPrimarySpecificPathsForAtMostOnePrimary() const {
  assertMustNotBeMoreThanOnePrimaryInputUnlessBatchModeChecksHaveBeenBypassed();
  static auto emptyPaths = PrimarySpecificPaths();
  return hasInputs() ? firstInputPotentiallyProducingOutput().getPrimarySpecificPaths()
                     : emptyPaths;
}

const PrimarySpecificPaths &
FrontendInputsAndOutputs::getPrimarySpecificPathsForPrimary(
    StringRef filename) const {
  const InputFile *f = primaryInputNamed(filename);
  return f->getPrimarySpecificPaths();
}

unsigned
FrontendInputsAndOutputs::indexForPrimaryInputNamed(StringRef name) const {
  assert(!name.empty() && "input files have names");
  StringRef correctedFile =
      InputFile::convertBufferNameFromLLVM_getFileOrSTDIN_toSwiftConventions(
          name);
  auto iterator = PotentialPrimaryInputsByName.find(correctedFile);
  if (iterator == PotentialPrimaryInputsByName.end())
    return AllInputs.size();
  return iterator->second;
}

const InputFile *
FrontendInputsAndOutputs::primaryInputNamed(StringRef name) const {
  const unsigned index = indexForPrimaryInputNamed(name);
  if (index >= AllInputs.size())
    return nullptr;
  const InputFile *f = &AllInputs[index];
  assert(f->isPotentialPrimaryInput() && "PrimaryInputsByName should only include primaries");
  return f;
}
