#ifndef LLVM_TRANSFORMS_IPO_SEARCHSTRATEGY_H
#define LLVM_TRANSFORMS_IPO_SEARCHSTRATEGY_H

#include <algorithm>
#include <functional>
#include <random>
#include <unordered_set>

class SearchStrategy {
private:
  // Default values
  const size_t NHashes{200};
  const size_t Rows{2};
  const size_t Bands{100};
  std::vector<uint32_t> RandomHashFuncs;

public:
  SearchStrategy() = default;

  SearchStrategy(size_t Rows, size_t Bands)
      : NHashes(Rows * Bands), Rows(Rows), Bands(Bands) {
    updateRandomHashFunctions(NHashes - 1);
  };

  uint32_t fnv1a(const std::vector<uint32_t> &Seq) {
    uint32_t Hash = 2166136261;
    const int Len = Seq.size();

    for (int i = 0; i < Len; i++) {
      Hash ^= Seq[i];
      Hash *= 1099511628211;
    }

    return Hash;
  }

  uint32_t fnv1a(const std::vector<uint32_t> &Seq, uint32_t NewHash) {
    uint32_t Hash = NewHash;
    const int Len = Seq.size();

    for (int i = 0; i < Len; i++) {
      Hash ^= Seq[i];
      Hash *= 1099511628211;
    }

    return Hash;
  }

  // Generate shingles using a single hash -- unused as not effective for
  // function merging
  template <uint32_t K>
  std::vector<uint32_t> &
  generateShinglesSingleHashPipelineTurbo(const std::vector<uint32_t> &Seq,
                                          std::vector<uint32_t> &Ret) {
    uint32_t Pipeline[K] = {0};
    const int Len = Seq.size();

    Ret.resize(NHashes);

    std::unordered_set<uint32_t> Set;
    // set.reserve(NHashes);
    uint32_t Last = 0;

    for (int i = 0; i < Len; i++) {

      for (int k = 0; k < K; k++) {
        Pipeline[k] ^= Seq[i];
        Pipeline[k] *= 1099511628211;
      }

      // Collect head of pipeline
      if (Last <= NHashes - 1) {
        Ret[Last++] = Pipeline[0];

        if (Last > NHashes - 1) {
          std::make_heap(Ret.begin(), Ret.end());
          std::sort_heap(Ret.begin(), Ret.end());
        }
      }

      if (Pipeline[0] < Ret.front() && Last > NHashes - 1) {
        if (Set.find(Pipeline[0]) == Set.end()) {
          Set.insert(Pipeline[0]);

          Ret[Last] = Pipeline[0];

          std::sort_heap(Ret.begin(), Ret.end());
        }
      }

      // Shift pipeline
      for (int k = 0; k < K - 1; k++) {
        Pipeline[k] = Pipeline[k + 1];
      }
      Pipeline[K - 1] = 2166136261;
    }

    return Ret;
  }

  // Generate MinHash fingerprint with multiple hash functions
  template <uint32_t K>
  std::vector<uint32_t> &
  generateShinglesMultipleHashPipelineTurbo(const std::vector<uint32_t> &Seq,
                                            std::vector<uint32_t> &Ret) {
    uint32_t Pipeline[K] = {0};
    const uint32_t Len = Seq.size();

    uint32_t Smallest = std::numeric_limits<uint32_t>::max();

    std::vector<uint32_t> ShingleHashes(Len);

    Ret.resize(NHashes);

    // Pipeline to hash all shingles using fnv1a
    // Store all hashes
    // While storing smallest
    // Then for each shingle hash, rehash with an XOR of 32 bit random number
    // and store smallest Do this NHashes-1 times to obtain NHashes minHashes
    // quickly Sort the hashes at the end

    for (uint32_t i = 0; i < Len; i++) {
      for (uint32_t k = 0; k < K; k++) {
        Pipeline[k] ^= Seq[i];
        Pipeline[k] *= 1099511628211;
      }

      // Collect head of pipeline
      if (Pipeline[0] < Smallest)
        Smallest = Pipeline[0];
      ShingleHashes[i] = Pipeline[0];

      // Shift pipeline
      for (uint32_t k = 0; k < K - 1; k++)
        Pipeline[k] = Pipeline[k + 1];
      Pipeline[K - 1] = 2166136261;
    }

    Ret[0] = Smallest;

    // Now for each hash function, rehash each shingle and store the smallest
    // each time
    for (uint32_t i = 0; i < RandomHashFuncs.size(); i++) {
      Smallest = std::numeric_limits<uint32_t>::max();

      for (uint32_t j = 0; j < ShingleHashes.size(); j++) {
        const uint32_t Temp = ShingleHashes[j] ^ RandomHashFuncs[i];

        if (Temp < Smallest)
          Smallest = Temp;
      }

      Ret[i + 1] = Smallest;
    }

    std::sort(Ret.begin(), Ret.end());

    return Ret;
  }

  void updateRandomHashFunctions(size_t Num) {
    const size_t OldNum = RandomHashFuncs.size();
    RandomHashFuncs.resize(Num);

    // if we shrunk the vector, there is nothing more to do
    if (Num <= OldNum)
      return;

    // If we enlarged it, we need to generate new random numbers
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::mt19937 Gen(0);
    std::uniform_real_distribution<> Distribution(
        0, std::numeric_limits<uint32_t>::max());

    // generating a random integer:
    for (size_t i = OldNum; i < Num; i++)
      RandomHashFuncs[i] = Distribution(Gen);
  }

  std::vector<uint32_t> &generateBands(const std::vector<uint32_t> &MinHashes,
                                       std::vector<uint32_t> &LSHBands) {
    LSHBands.resize(Bands);

    // Generate a hash for each band
    for (size_t i = 0; i < Bands; i++) {
      // Perform fnv1a on the rows
      auto First = MinHashes.begin() + (i * Rows);
      auto Last = MinHashes.begin() + (i * Rows) + Rows;
      LSHBands[i] = fnv1a(std::vector<uint32_t>{First, Last});
    }

    // Remove duplicate bands -- no need to place twice in the same bucket
    std::sort(LSHBands.begin(), LSHBands.end());
    auto Last = std::unique(LSHBands.begin(), LSHBands.end());
    LSHBands.erase(Last, LSHBands.end());

    return LSHBands;
  }

  uint32_t itemFootprint() { return sizeof(uint32_t) * Bands * (Rows + 1); }
};

#endif // LLVM_TRANSFORMS_IPO_SEARCHSTRATEGY_H
