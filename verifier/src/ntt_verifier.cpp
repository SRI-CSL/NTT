/**
 *  Abstract Interpretation-based verifier built on the top of Clam
 *  (https://github.com/seahorn/clam) specialized for NTT
 *  procedures. The NTT procedures are bitcode programs annotated
 *  already with assertions.
**/
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/Function.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/SourceMgr.h"

// Seadsa
#include "seadsa/AllocWrapInfo.hh"
#include "seadsa/DsaLibFuncInfo.hh"
#include "seadsa/support/RemovePtrToInt.hh"

// Clam support
#include "clam/Passes.hh"
#include "clam/Clam.hh"
#include "clam/CfgBuilder.hh"
#include "clam/HeapAbstraction.hh"
#include "clam/SeaDsaHeapAbstraction.hh"
#include "clam/RegisterAnalysis.hh"
#include "clam/Support/NameValues.hh"

// Abstract domains
#include "crab/domains/array_adaptive.hpp"
#include "crab/domains/abstract_domain_params.hpp"
#include "ntt_intervals.hpp"

using namespace clam;
using namespace llvm;

namespace clam {
namespace CrabDomain {
constexpr Type NTT_INTERVALS(1, "ntt-intervals", "ntt-intervals", false, false);  
} //end CrabDomain

using ntt_interval_domain_t = ntt_verifier::ntt_interval_domain<number_t, varname_t>;
using array_ntt_interval_domain_t =
  crab::domains::array_adaptive_domain<ntt_interval_domain_t>;
} //end clam

static void registerDomain() {
  auto &map = DomainRegistry::getFactoryMap();		
  clam::clam_abstract_domain val(std::move(clam::array_ntt_interval_domain_t()));	
  map.insert({clam::CrabDomain::NTT_INTERVALS, val});	       
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    llvm::errs() << "Not found input file\n";
    llvm::errs() << "Usage " << argv[0] << " " << "FILE.bc\n";
    return 1;
  }
  
  //////////////////////////////////////  
  // Get module from LLVM file
  //////////////////////////////////////  
  LLVMContext Context;
  SMDiagnostic Err;
  std::unique_ptr<Module> module = parseIRFile(argv[1], Err, Context);
  if (!module) {
    Err.print(argv[0], errs());
    return 1;
  }
  
  const auto& tripleS = module->getTargetTriple();
  Twine tripleT(tripleS);
  Triple triple(tripleT);
  TargetLibraryInfoWrapperPass  TLIW(triple);  


  ///////////////////////////////////////////
  // Optimize the bitcode for verification
  ///////////////////////////////////////////  
  llvm::legacy::PassManager pass_manager;
  // kill unused internal global
  pass_manager.add(llvm::createGlobalDCEPass());
  pass_manager.add(clam::createRemoveUnreachableBlocksPass());
  // -- promote alloca's to registers
  pass_manager.add(llvm::createPromoteMemoryToRegisterPass());
  // -- ensure one single exit point per function
  pass_manager.add(llvm::createUnifyFunctionExitNodesPass());
  // -- remove unreachable blocks
  pass_manager.add(clam::createRemoveUnreachableBlocksPass());
  // -- remove switch constructions
  pass_manager.add(llvm::createLowerSwitchPass());
  // cleanup after lowering switches
  pass_manager.add(llvm::createCFGSimplificationPass());
  // -- lower constant expressions to instructions
  pass_manager.add(clam::createLowerCstExprPass());
  // cleanup after lowering constant expressions
  pass_manager.add(llvm::createDeadCodeEliminationPass());
  // -- lower ULT and ULE instructions
  pass_manager.add(clam::createLowerUnsignedICmpPass());
  // cleanup unnecessary and unreachable blocks
  pass_manager.add(llvm::createCFGSimplificationPass());
  pass_manager.add(clam::createRemoveUnreachableBlocksPass());
  // -- remove ptrtoint and inttoptr instructions
  pass_manager.add(seadsa::createRemovePtrToIntPass());
  // -- must be the last ones before running crab.
  pass_manager.add(clam::createLowerSelectPass());
  // -- ensure one single exit point per function
  //    LowerUnsignedICmpPass and LowerSelect can add multiple
  //    returns.
  pass_manager.add(llvm::createUnifyFunctionExitNodesPass());
  pass_manager.add(new clam::NameValues());
  pass_manager.run(*module.get());
  
  //////////////////////////////////////
  // Run seadsa -- pointer analysis
  //////////////////////////////////////  
  CallGraph cg(*module);
  seadsa::AllocWrapInfo allocWrapInfo(&TLIW);
  allocWrapInfo.initialize(*module, nullptr);
  seadsa::DsaLibFuncInfo dsaLibFuncInfo;
  dsaLibFuncInfo.initialize(*module);
  std::unique_ptr<HeapAbstraction> mem(new SeaDsaHeapAbstraction(
		*module, cg, TLIW, allocWrapInfo, dsaLibFuncInfo, true));

  //////////////////////////////////////  
  // Clam -- abstract interpreter
  //////////////////////////////////////
  
  /// Translation from LLVM to CrabIR
  CrabBuilderParams cparams;
  cparams.setPrecision(clam::CrabBuilderPrecision::SINGLETON_MEM);
  CrabBuilderManager man(cparams, TLIW, std::move(mem));
  
  /// Set Crab parameters
  AnalysisParams aparams;
  aparams.dom = CrabDomain::NTT_INTERVALS;
  aparams.run_inter = true;
  aparams.check = clam::CheckerKind::ASSERTION;
  // disable Clam/Crab warnings
  crab::CrabEnableWarningMsg(false);
  // set parameters of the array adaptive domain
  crab::domains::array_adaptive_domain_params p(false/*is_smashable*/,
						false/*smash_at_nonzero_offset*/,
						1024/*max_smashable_cells*/,
						1024/*max_array_size*/);
  crab::domains::crab_domain_params_man::get().update_params(p);
  /// Create an inter-analysis instance
  registerDomain(); // register the domain before creating an InterGlobalClam instance  
  InterGlobalClam ga(*module, man);
  /// Run the Crab analysis
  ClamGlobalAnalysis::abs_dom_map_t assumptions;
  ga.analyze(aparams, assumptions);
  
  /// Print results about assertion checks
  llvm::errs() << "===Assertion checks ===\n";
  ga.getChecksDB().write(crab::outs());
  
  return 0;
}
