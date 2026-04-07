//=- AArch64SchedHIP09WriteRes.td - Huawei HIP09 WriteRes ---*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Contains all of the HIP09-specific SchedWriteRes types. The approach below
// is to define a generic SchedWriteRes for every combination of latency and
// micro-ops. The naming conventions is to use a prefix, one field for latency,
// and one or more microOp count/type designators.
//
//   Prefix: HIP09Write
//   Latency: #cyc
//   Micro-op Count/Types: #(BRU|ALUS01|ALUS23|ALUS|ALUM1|ALUM2|ALUM|LD|ST|FSU0|FSU2|FSU02|FSU|STD)
//
// e.g. HIP09Write_6cyc_1ALUS_6LD_4FSU means the total latency is 6 cycles,
// and 11 micro-ops are issued down 1 ALUS pipe, 6 LD pipes, and 4 FSU
// pipes
//
//===----------------------------------------------------------------------===//


def HIP09Write_0cyc                       : SchedWriteRes<[]>                 { let Latency = 0; }

def HIP09Write_1cyc_1BRU                  : SchedWriteRes<[HIP09UnitBRU]>     { let Latency = 1; }

def HIP09Write_1cyc_1ALUS                 : SchedWriteRes<[HIP09UnitALUS]>    { let Latency = 1; }
def HIP09Write_1cyc_1ALUS1                : SchedWriteRes<[HIP09UnitALUS1]>   { let Latency = 1; }
def HIP09Write_1cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 1; }
def HIP09Write_2cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 2; }
def HIP09Write_3cyc_1ALUS01               : SchedWriteRes<[HIP09UnitALUS01]>  { let Latency = 3; }
def HIP09Write_1cyc_1ALUS23               : SchedWriteRes<[HIP09UnitALUS23]>  { let Latency = 1; }
def HIP09Write_2cyc_1ALUS23               : SchedWriteRes<[HIP09UnitALUS23]>  { let Latency = 2; }

def HIP09Write_2cyc_1ALUM                 : SchedWriteRes<[HIP09UnitALUM]>    { let Latency = 2; }
def HIP09Write_3cyc_1ALUM                 : SchedWriteRes<[HIP09UnitALUM]>    { let Latency = 3; }
def HIP09Write_5cyc_1ALUM1                : SchedWriteRes<[HIP09UnitALUM1]>   { let Latency = 5; }
def HIP09Write_12cyc_1ALUM0_12RC          : SchedWriteRes<[HIP09UnitALUM0]>   { let Latency = 12; let ReleaseAtCycles = [12]; }
def HIP09Write_20cyc_1ALUM0_20RC          : SchedWriteRes<[HIP09UnitALUM0]>   { let Latency = 20; let ReleaseAtCycles = [20]; }

def HIP09Write_1cyc_1ST                   : SchedWriteRes<[HIP09UnitST]>      { let Latency = 1; }

def HIP09Write_1cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 1; }
def HIP09Write_2cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 2; }
def HIP09Write_3cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 3; }
def HIP09Write_4cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 4; }
def HIP09Write_5cyc_1FSU                  : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 5; }
def HIP09Write_5cyc_1FSU_3RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 5;  let ReleaseAtCycles = [3]; }
def HIP09Write_7cyc_1FSU_3RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 7;  let ReleaseAtCycles = [3]; }
def HIP09Write_9cyc_1FSU_5RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 9;  let ReleaseAtCycles = [5]; }
def HIP09Write_9cyc_1FSU_8RC              : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 9;  let ReleaseAtCycles = [8]; }
def HIP09Write_10cyc_1FSU_6RC             : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 10; let ReleaseAtCycles = [6]; }
def HIP09Write_13cyc_1FSU_9RC             : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 13; let ReleaseAtCycles = [9]; }
def HIP09Write_15cyc_1FSU_11RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 15; let ReleaseAtCycles = [11]; }
def HIP09Write_21cyc_1FSU_17RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 21; let ReleaseAtCycles = [17]; }
def HIP09Write_25cyc_1FSU_21RC            : SchedWriteRes<[HIP09UnitFSU]>     { let Latency = 25; let ReleaseAtCycles = [21]; }
def HIP09Write_1cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 1; }
def HIP09Write_2cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 2; }
def HIP09Write_3cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 3; }
def HIP09Write_4cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 4; }
def HIP09Write_4cyc_1FSU02_4RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 4;  let ReleaseAtCycles = [4]; }
def HIP09Write_5cyc_1FSU02                : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 5; }
def HIP09Write_7cyc_1FSU02_3RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 7;  let ReleaseAtCycles = [3]; }
def HIP09Write_9cyc_1FSU02_3RC            : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 9;  let ReleaseAtCycles = [3]; }
def HIP09Write_12cyc_1FSU02_4RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 12; let ReleaseAtCycles = [4]; }
def HIP09Write_12cyc_1FSU02_8RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 12; let ReleaseAtCycles = [8]; }
def HIP09Write_13cyc_1FSU02_9RC           : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 13; let ReleaseAtCycles = [9]; }
def HIP09Write_15cyc_1FSU02_11RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 15; let ReleaseAtCycles = [11]; }
def HIP09Write_17cyc_1FSU02_13RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 17; let ReleaseAtCycles = [13]; }
def HIP09Write_20cyc_1FSU02_16RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 20; let ReleaseAtCycles = [16]; }
def HIP09Write_21cyc_1FSU02_17RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 21; let ReleaseAtCycles = [17]; }
def HIP09Write_25cyc_1FSU02_21RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 25; let ReleaseAtCycles = [21]; }
def HIP09Write_36cyc_1FSU02_32RC          : SchedWriteRes<[HIP09UnitFSU02]>   { let Latency = 36; let ReleaseAtCycles = [32]; }
def HIP09Write_1cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 1; }
def HIP09Write_2cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 2; }
def HIP09Write_4cyc_1FSU2                 : SchedWriteRes<[HIP09UnitFSU2]>    { let Latency = 4; }
def HIP09Write_4cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 4; }
def HIP09Write_5cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 5; }
def HIP09Write_6cyc_1LD                   : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6; }
def HIP09Write_6cyc_1LD_3RC               : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6;  let ReleaseAtCycles = [3]; }
def HIP09Write_6cyc_1LD_4RC               : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 6;  let ReleaseAtCycles = [4]; }
def HIP09Write_16cyc_1LD_4RC              : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 16; let ReleaseAtCycles = [4]; }
def HIP09Write_18cyc_1LD_4RC              : SchedWriteRes<[HIP09UnitLD]>      { let Latency = 18; let ReleaseAtCycles = [4]; }

def HIP09Write_1cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}
def HIP09Write_2cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}
def HIP09Write_2cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_3cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 3;
  let NumMicroOps = 2;
}

def HIP09Write_3cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 3;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 4;
}

def HIP09Write_5cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_2FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_2FSU02                : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 6;
  let NumMicroOps = 4;
}

def HIP09Write_7cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 7;
  let NumMicroOps = 4;
}

def HIP09Write_9cyc_4FSU                  : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 9;
  let NumMicroOps = 4;
}

def HIP09Write_6cyc_1BRU_1ALUM1           : SchedWriteRes<[HIP09UnitBRU, HIP09UnitALUM1]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 1;
  let NumMicroOps = 4;
}

def HIP09Write_2cyc_1ST_1STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_2cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 4;
}

def HIP09Write_2cyc_4ST_4STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 2;
  let NumMicroOps = 8;
}

def HIP09Write_3cyc_2ST_2STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 3;
  let NumMicroOps = 4;
}

def HIP09Write_4cyc_3ST_3STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 6;
}

def HIP09Write_4cyc_8ST_8STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 16;
}


def HIP09Write_5cyc_4ST_4STD              : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 5;
  let NumMicroOps = 8;
}

def HIP09Write_1cyc_1ST_1STD_1ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD, HIP09UnitALUS]>
{
  let Latency = 1;
  let NumMicroOps = 3;
}

def HIP09Write_2cyc_1ST_1STD_1ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitSTD, HIP09UnitALUS]>
{
  let Latency = 2;
  let NumMicroOps = 3;
}

def HIP09Write_2cyc_2ST_2STD_2ALUS        : SchedWriteRes<[HIP09UnitST, HIP09UnitST, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitALUS, HIP09UnitALUS]>
{
  let Latency = 2;
  let NumMicroOps = 6;
}

def HIP09Write_2cyc_1BRU_1ALUS23          : SchedWriteRes<[HIP09UnitBRU, HIP09UnitALUS23]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_1LD_1ALUS             : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUS]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_1LD_1ALUS             : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUS]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_9cyc_1LD_1ALUM1            : SchedWriteRes<[HIP09UnitLD, HIP09UnitALUM1]>
{
  let Latency = 9;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1ALUM             : SchedWriteRes<[HIP09UnitST, HIP09UnitALUM]>
{
  let Latency = 1;
  let NumMicroOps = 2;
}

def HIP09Write_2cyc_1ST_1ALUM             : SchedWriteRes<[HIP09UnitST, HIP09UnitALUM]>
{
  let Latency = 2;
  let NumMicroOps = 2;
}

def HIP09Write_4cyc_1ALUS01_1FSU          : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_1cyc_1ST_1ALUS_1ALUM       : SchedWriteRes<[HIP09UnitST, HIP09UnitALUS, HIP09UnitALUM]>
{
  let Latency = 1;
  let NumMicroOps = 3;
}

def HIP09Write_3cyc_1ST_1ALUS_1ALUM       : SchedWriteRes<[HIP09UnitST, HIP09UnitALUS, HIP09UnitALUM]>
{
  let Latency = 3;
  let NumMicroOps = 3;
}

def HIP09Write_4cyc_1FSU_1ALUS23          : SchedWriteRes<[HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 4;
  let NumMicroOps = 2;
}

def HIP09Write_7cyc_1ALUS01_1FSU_1ALUS23  : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 3;
}

def HIP09Write_5cyc_1FSU02_1ALUS          : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_1FSU02_1ALUS          : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1ALUS01_1FSU          : SchedWriteRes<[HIP09UnitALUS01, HIP09UnitFSU]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1FSU_1ALUS23          : SchedWriteRes<[HIP09UnitFSU, HIP09UnitALUS23]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_5cyc_1FSU02_1ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitALUS23]>
{
  let Latency = 5;
  let NumMicroOps = 2;
}

def HIP09Write_6cyc_4FSU02_4ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 6;
  let NumMicroOps = 8;
}

def HIP09Write_7cyc_4FSU02_4ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 8;
}

def HIP09Write_6cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 6;
  let NumMicroOps = 12;
}

def HIP09Write_7cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 7;
  let NumMicroOps = 12;
}

def HIP09Write_8cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 8;
  let NumMicroOps = 12;
}

def HIP09Write_9cyc_6FSU02_6ALUS23        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23,
                                                              HIP09UnitALUS23, HIP09UnitALUS23, HIP09UnitALUS23]>
{
  let Latency = 9;
  let NumMicroOps = 12;
}

def HIP09Write_9cyc_18FSU02_9ALUM1        : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitALUM1, HIP09UnitALUM1,
                                                              HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1,
                                                              HIP09UnitALUM1, HIP09UnitALUM1, HIP09UnitALUM1]>
{
  let Latency = 9;
  let NumMicroOps = 27;
}

def HIP09Write_6cyc_2LD                   : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD]>
{
  let Latency = 6;
  let NumMicroOps = 2;
}

def HIP09Write_7cyc_1LD_1FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitFSU]>
{
  let Latency = 7;
  let NumMicroOps = 2;
}

def HIP09Write_8cyc_1LD_1FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitFSU]>
{
  let Latency = 8;
  let NumMicroOps = 2;
}

def HIP09Write_8cyc_2LD_2FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 8;
  let NumMicroOps = 4;
}

def HIP09Write_8cyc_2LD_2FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 8;
  let NumMicroOps = 4;
}

def HIP09Write_9cyc_3LD_3FSU              : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 9;
  let NumMicroOps = 6;
}

def HIP09Write_9cyc_4LD_4FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 9;
  let NumMicroOps = 8;
}

def HIP09Write_11cyc_6LD_6FSU02            : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 11;
  let NumMicroOps = 12;
}

def HIP09Write_16cyc_16LD_16FSU02         : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02]>
{
  let Latency = 16;
  let NumMicroOps = 32;
}

def HIP09Write_12cyc_8LD_8FSU             : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 12;
  let NumMicroOps = 16;
}

def HIP09Write_13cyc_8LD_8FSU             : SchedWriteRes<[HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitLD, HIP09UnitLD, HIP09UnitLD, HIP09UnitLD,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU]>
{
  let Latency = 13;
  let NumMicroOps = 16;
}

def HIP09Write_3cyc_1FSU02_1ST_1STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 3;
  let NumMicroOps = 3;
}

def HIP09Write_4cyc_1FSU_1ST_1STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitST, HIP09UnitSTD]>
{
  let Latency = 4;
  let NumMicroOps = 3;
}

def HIP09Write_6cyc_2FSU_2ST_2STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 6;
}

def HIP09Write_6cyc_3FSU02_3ST_3STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 9;
}

def HIP09Write_6cyc_4FSU02_4ST_4STD       : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 6;
  let NumMicroOps = 12;
}

def HIP09Write_7cyc_3FSU_3ST_3STD         : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 7;
  let NumMicroOps = 9;
}

def HIP09Write_8cyc_16FSU02_16ST_16STD    : SchedWriteRes<[HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02, HIP09UnitFSU02,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 8;
  let NumMicroOps = 48;
}

def HIP09Write_10cyc_6FSU_6ST_6STD        : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 10;
  let NumMicroOps = 18;
}

def HIP09Write_10cyc_8FSU_8ST_8STD        : SchedWriteRes<[HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU, HIP09UnitFSU,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitST, HIP09UnitST, HIP09UnitST, HIP09UnitST,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD,
                                                              HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD, HIP09UnitSTD]>
{
  let Latency = 10;
  let NumMicroOps = 24;
}

