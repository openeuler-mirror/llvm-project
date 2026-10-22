// RUN: llvm-mc -triple=aarch64 -show-encoding -mattr=+tlbid,+tlb-rmi < %s \
// RUN:        | FileCheck %s --check-prefixes=CHECK-ENCODING,CHECK-INST
// RUN: not llvm-mc -triple=aarch64 -show-encoding -mattr=+tlb-rmi < %s 2>&1 \
// RUN:        | FileCheck %s --check-prefix=CHECK-ERROR
// RUN: llvm-mc -triple=aarch64 -filetype=obj -mattr=+tlbid,+tlb-rmi < %s \
// RUN:        | llvm-objdump -d --mattr=+tlbid,+tlb-rmi --no-print-imm-hex - | FileCheck %s --check-prefix=CHECK-INST
// RUN: llvm-mc -triple=aarch64 -filetype=obj -mattr=+tlbid,+tlb-rmi < %s \
// RUN:        | llvm-objdump -d --mattr=-tlbid --no-print-imm-hex - | FileCheck %s --check-prefix=CHECK-UNKNOWN
// Disassemble encoding and check the re-encoding (-show-encoding) matches.
// RUN: llvm-mc -triple=aarch64 -show-encoding -mattr=+tlbid,+tlb-rmi < %s \
// RUN:        | sed '/.text/d' | sed 's/.*encoding: //g' \
// RUN:        | llvm-mc -triple=aarch64 -mattr=+tlbid,+tlb-rmi -disassemble -show-encoding \
// RUN:        | FileCheck %s --check-prefixes=CHECK-ENCODING,CHECK-INST

// Armv9.7-A TLBI Domains (FEAT_TLBID)

tlbi vmalle1is
// CHECK-INST: tlbi vmalle1is
// CHECK-ENCODING: encoding: [0x1f,0x83,0x08,0xd5]
// CHECK-UNKNOWN: d508831f tlbi vmalle1is

tlbi vmalle1is, xzr
// CHECK-INST: tlbi vmalle1is
// CHECK-ENCODING: encoding: [0x1f,0x83,0x08,0xd5]
// CHECK-UNKNOWN: d508831f tlbi vmalle1is

tlbi vmalle1is, x31
// CHECK-INST: tlbi vmalle1is
// CHECK-ENCODING: encoding: [0x1f,0x83,0x08,0xd5]
// CHECK-UNKNOWN: d508831f tlbi vmalle1is

tlbi vmalle1is, x5
// CHECK-INST: tlbi vmalle1is, x5
// CHECK-ENCODING: encoding: [0x05,0x83,0x08,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d5088305 sys	#0, c8, c3, #0, x5

tlbi vmalle1os, x5
// CHECK-INST: tlbi vmalle1os, x5
// CHECK-ENCODING: encoding: [0x05,0x81,0x08,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d5088105 sys	#0, c8, c1, #0, x5

tlbi alle1is, x5
// CHECK-INST: tlbi alle1is, x5
// CHECK-ENCODING: encoding: [0x85,0x83,0x0c,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d50c8385 sys	#4, c8, c3, #4, x5

tlbi alle2is, x5
// CHECK-INST: tlbi alle2is, x5
// CHECK-ENCODING: encoding: [0x05,0x83,0x0c,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d50c8305 sys	#4, c8, c3, #0, x5

tlbi alle3is, x5
// CHECK-INST: tlbi alle3is, x5
// CHECK-ENCODING: encoding: [0x05,0x83,0x0e,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d50e8305 sys	#6, c8, c3, #0, x5

tlbi vmalls12e1is, x1
// CHECK-INST: tlbi vmalls12e1is, x1
// CHECK-ENCODING: encoding: [0xc1,0x83,0x0c,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d50c83c1 sys	#4, c8, c3, #6, x1

tlbi vmalls12e1os, x5
// CHECK-INST: tlbi vmalls12e1os, x5
// CHECK-ENCODING: encoding: [0xc5,0x81,0x0c,0xd5]
// CHECK-ERROR: error: specified tlbi op does not use a register
// CHECK-UNKNOWN: d50c81c5 sys	#4, c8, c1, #6, x5

mrs x3, VTLBID0_EL2
// CHECK-INST: mrs x3, VTLBID0_EL2
// CHECK-ENCODING: encoding: [0x03,0x28,0x3c,0xd5]
mrs x3, VTLBID1_EL2
// CHECK-INST: mrs x3, VTLBID1_EL2
// CHECK-ENCODING: encoding: [0x23,0x28,0x3c,0xd5]
mrs x3, VTLBID2_EL2
// CHECK-INST: mrs x3, VTLBID2_EL2
// CHECK-ENCODING: encoding: [0x43,0x28,0x3c,0xd5]
mrs x3, VTLBID3_EL2
// CHECK-INST: mrs x3, VTLBID3_EL2
// CHECK-ENCODING: encoding: [0x63,0x28,0x3c,0xd5]

mrs x3, VTLBIDOS0_EL2
// CHECK-INST: mrs x3, VTLBIDOS0_EL2
// CHECK-ENCODING: encoding: [0x03,0x29,0x3c,0xd5]
mrs x3, VTLBIDOS1_EL2
// CHECK-INST: mrs x3, VTLBIDOS1_EL2
// CHECK-ENCODING: encoding: [0x23,0x29,0x3c,0xd5]
mrs x3, VTLBIDOS2_EL2
// CHECK-INST: mrs x3, VTLBIDOS2_EL2
// CHECK-ENCODING: encoding: [0x43,0x29,0x3c,0xd5]
mrs x3, VTLBIDOS3_EL2
// CHECK-INST: mrs x3, VTLBIDOS3_EL2
// CHECK-ENCODING: encoding: [0x63,0x29,0x3c,0xd5]

mrs x3, TLBIDIDR_EL1
// CHECK-INST: mrs x3, TLBIDIDR_EL1
// CHECK-ENCODING: encoding: [0xc3,0xa4,0x38,0xd5]

msr VTLBID0_EL2, x3
// CHECK-INST: msr VTLBID0_EL2, x3
// CHECK-ENCODING: encoding: [0x03,0x28,0x1c,0xd5]
msr VTLBID1_EL2, x3
// CHECK-INST: msr VTLBID1_EL2, x3
// CHECK-ENCODING: encoding: [0x23,0x28,0x1c,0xd5]
msr VTLBID2_EL2, x3
// CHECK-INST: msr VTLBID2_EL2, x3
// CHECK-ENCODING: encoding: [0x43,0x28,0x1c,0xd5]
msr VTLBID3_EL2, x3
// CHECK-INST: msr VTLBID3_EL2, x3
// CHECK-ENCODING: encoding: [0x63,0x28,0x1c,0xd5]

msr VTLBIDOS0_EL2, x3
// CHECK-INST: msr VTLBIDOS0_EL2, x3
// CHECK-ENCODING: encoding: [0x03,0x29,0x1c,0xd5]
msr VTLBIDOS1_EL2, x3
// CHECK-INST: msr VTLBIDOS1_EL2, x3
// CHECK-ENCODING: encoding: [0x23,0x29,0x1c,0xd5]
msr VTLBIDOS2_EL2, x3
// CHECK-INST: msr VTLBIDOS2_EL2, x3
// CHECK-ENCODING: encoding: [0x43,0x29,0x1c,0xd5]
msr VTLBIDOS3_EL2, x3
// CHECK-INST: msr VTLBIDOS3_EL2, x3
// CHECK-ENCODING: encoding: [0x63,0x29,0x1c,0xd5]
