// tree_plru_rp.hh
#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_TREE_PLRU_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_TREE_PLRU_RP_HH__

#include <memory>
#include <vector>

#include "mem/cache/replacement_policies/base.hh"
#include "params/TreePLRURP.hh"

namespace gem5
{
class TreePLRURPParams;
namespace replacement_policy
{

class TreePLRURP : public BaseReplacementPolicy
{
  protected:
    using PLRUTree = std::vector<bool>; // Inlined instead of tree_plru_tree.hh

    class TreePLRUReplData : public ReplacementData
    {
      public:
        const uint64_t index;
        std::shared_ptr<PLRUTree> tree;
        TreePLRUReplData(const uint64_t index, std::shared_ptr<PLRUTree> tree);
    };

  private:
    const uint64_t numLeaves;
    mutable uint64_t count;
    mutable PLRUTree *treeInstance;

  public:
    using Params = TreePLRURPParams;
    TreePLRURP(const Params *p);
    void invalidate
    (const std::shared_ptr<ReplacementData>& replacement_data) const override;
    void touch
    (const std::shared_ptr<ReplacementData>& replacement_data) const override;
    void reset
    (const std::shared_ptr<ReplacementData>& replacement_data) const override;
    ReplaceableEntry* getVictim
    (const ReplacementCandidates& candidates) const override;
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace replacement_policy

TreePLRURP* TreePLRURPParams::create();

} // namespace gem5

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_TREE_PLRU_RP_HH__

