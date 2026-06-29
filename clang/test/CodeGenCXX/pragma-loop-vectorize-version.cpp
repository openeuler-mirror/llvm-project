// RUN: %clang_cc1 -triple aarch64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s --check-prefix=DEFAULT
// RUN: %clang_cc1 -triple aarch64-unknown-linux-gnu -emit-llvm -o - %s -fvectorize-version=sve | FileCheck %s --check-prefix=SVE-OPT
// RUN: %clang_cc1 -triple aarch64-unknown-linux-gnu -emit-llvm -o - %s -fvectorize-version=neon | FileCheck %s --check-prefix=NEON-OPT

void pragma_sve(int *List, int Length) {
#pragma clang loop vectorize_version(sve)
  for (int i = 0; i < Length; i++) {
    // DEFAULT: br label {{.*}}, !llvm.loop ![[SVE_LOOP:.*]]
    List[i] = i;
  }
}

void pragma_neon(int *List, int Length) {
#pragma clang loop vectorize_width(4) vectorize_version(neon)
  for (int i = 0; i < Length; i++) {
    // DEFAULT: br label {{.*}}, !llvm.loop ![[NEON_LOOP:.*]]
    List[i] = i;
  }
}

void no_version(int *List, int Length) {
  for (int i = 0; i < Length; i++) {
    // DEFAULT: br label {{.*}}, !llvm.loop ![[NO_VERSION_LOOP:.*]]
    // SVE-OPT: br label {{.*}}, !llvm.loop ![[SVE_OPT_LOOP:.*]]
    // NEON-OPT: br label {{.*}}, !llvm.loop ![[NEON_OPT_LOOP:.*]]
    List[i] = i;
  }
}

// DEFAULT-DAG: !{!"llvm.loop.vectorize.version", i32 1}
// DEFAULT-DAG: ![[VECTORIZE_ENABLE:[0-9]+]] = !{!"llvm.loop.vectorize.enable", i1 true}

// DEFAULT-DAG: !{!"llvm.loop.vectorize.width", i32 4}
// DEFAULT-DAG: !{!"llvm.loop.vectorize.scalable.enable", i1 false}
// DEFAULT-DAG: !{!"llvm.loop.vectorize.version", i32 0}
// DEFAULT: ![[NO_VERSION_LOOP]] = distinct !{![[NO_VERSION_LOOP]], {{.*}}}
// DEFAULT-NOT: !"llvm.loop.vectorize.version"

// SVE-OPT-DAG: !{!"llvm.loop.vectorize.version", i32 1}

// NEON-OPT-DAG: !{!"llvm.loop.vectorize.version", i32 0}
