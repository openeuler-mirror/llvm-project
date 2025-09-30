// Testing signal counter reset
// RUN: %clangxx_profgen -fcoverage-mapping -Wno-comment -o %t %s
// RUN: env LLVM_PROFILE_RESET_SIGNUM=40 LLVM_PROFILE_FILE=%t.profraw %run %t
// RUN: llvm-profdata merge -o %t.profdata %t.profraw
// RUN: llvm-cov show %t -instr-profile %t.profdata 2>&1 | FileCheck %s --check-prefix=RESET
// RUN: (trap '' 40;env LLVM_PROFILE_RESET_SIGNUM=0 LLVM_PROFILE_FILE=%t.profraw %run %t)
// RUN: llvm-profdata merge -o %t.profdata %t.profraw
// RUN: llvm-cov show %t -instr-profile %t.profdata 2>&1 | FileCheck %s --check-prefix=NOT-RESET

#include <signal.h>                                            
#include <unistd.h>                                            
                                                               
int main() {                                                   
  int j = 1;                                                   
  for (int i = 0; i < 100; ++i) {                              
    if (i == 20) {                                             
      kill(getpid(), 40);                                  
    }                                                          
    if (i < 20) {                                              
      ++j;                                                     
    }                                                          
    else {                                                     
      j += 2;                                                  
    }                                                          
  }                                                            
  return 0;                                                    
}
 //RESET:   6|       |
 //RESET:   7|       |
 //RESET:   8|       |
 //RESET:   9|      0|
 //RESET:   10|      0|
 //RESET:   11|     79|
 //RESET:   12|     79|

 //NOT-RESET:   6|       |
 //NOT-RESET:   7|       |
 //NOT-RESET:   8|       |
 //NOT-RESET:   9|    100|
 //NOT-RESET:   10|    100|
 //NOT-RESET:   11|    100|
 //NOT-RESET:   12|    100|
