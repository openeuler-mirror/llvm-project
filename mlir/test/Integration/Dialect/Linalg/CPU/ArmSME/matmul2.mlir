module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg0: !transform.any_op {transform.consumed}) {
    %0 = transform.structured.match ops{["linalg.matmul"]} in %arg0 : (!transform.any_op) -> !transform.any_op

    %tiled_linalg_op, %loops:3 = transform.structured.tile_using_for %0 tile_sizes [[4], [4], 1] : (!transform.any_op) -> (!transform.any_op, !transform.any_op, !transform.any_op, !transform.any_op)

    transform.structured.vectorize %tiled_linalg_op vector_sizes [[4], [4], 1] : !transform.any_op

    %1 = transform.bufferization.one_shot_bufferize %arg0 {bufferize_function_boundaries = true} : (!transform.any_op) -> !transform.any_op

    %2 = transform.structured.match ops{["func.func"]} in %1 : (!transform.any_op) -> !transform.any_op

    %3 = transform.apply_registered_pass "convert-linalg-to-loops" to %2 : (!transform.any_op) -> !transform.op<"func.func">

    transform.apply_patterns to %3 {
      transform.apply_patterns.vector.lower_masked_transfers
      transform.apply_patterns.vector.transfer_permutation_patterns
      transform.apply_patterns.vector.reduction_to_contract
    } : !transform.op<"func.func">

    transform.apply_patterns to %3 {
      transform.apply_patterns.vector.cast_away_vector_leading_one_dim
      transform.apply_patterns.tensor.fold_tensor_subset_ops_into_vector_transfers
      transform.apply_patterns.vector.lower_contraction lowering_strategy = "outerproduct"
      transform.apply_patterns.vector.lower_masks
      transform.apply_patterns.canonicalization
    } : !transform.op<"func.func">

    %5 = transform.structured.match interface{LoopLikeInterface} in %1 : (!transform.any_op) -> !transform.any_op

    transform.apply_licm to %5 : !transform.any_op

    transform.loop.hoist_loop_invariant_subsets %5 : !transform.any_op

    transform.yield
  }
  transform.named_sequence @arm_sme_lowering_schedule(%arg0: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.lower_to_arm_sme %arg0 : !transform.any_op
    transform.yield %arg0 : !transform.any_op
  }
  transform.named_sequence @lower_to_llvm_schedule(%arg0: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.lower_to_llvm_new %arg0 {enable_arm_sve = true, enable_index_optimizations = true, vscale_range = 0 : i64} : !transform.any_op
    transform.yield %arg0 : !transform.any_op
  }
  transform.named_sequence @__transform_main_next(%arg0: !transform.any_op) {
    %0 = transform.structured.match ops{["func.func"]} in %arg0 : (!transform.any_op) -> !transform.any_op
    %1 = transform.get_parent_op %0 {deduplicate} : (!transform.any_op) -> !transform.any_op
    %2 = transform.include @arm_sme_lowering_schedule failures(propagate) (%1) : (!transform.any_op) -> !transform.any_op
    %3 = transform.apply_registered_pass "cse" to %2 : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %3 {
      transform.apply_patterns.linalg.tiling_canonicalization
      transform.apply_patterns.scf.for_loop_canonicalization
    } : !transform.any_op
    %4 = transform.structured.match interface{LoopLikeInterface} in %3 : (!transform.any_op) -> !transform.any_op
    transform.apply_licm to %4 : !transform.any_op
    %5 = transform.structured.match ops{["func.func"]} in %3 : (!transform.any_op) -> !transform.any_op
    %6 = transform.structured.hoist_redundant_vector_transfers %5 : (!transform.any_op) -> !transform.any_op
    %7 = transform.structured.hoist_redundant_vector_broadcasts %6 : (!transform.any_op) -> !transform.any_op
    %8 = transform.apply_registered_pass "canonicalize" to %7 : (!transform.any_op) -> !transform.any_op
    transform.yield
  }
}
