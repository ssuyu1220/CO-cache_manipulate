// tree_plru_rp.cc
#include "mem/cache/replacement_policies/tree_plru_rp.hh"

#include <cmath>

#include "base/logging.hh"
#include "base/types.hh"
#include "params/TreePLRURP.hh"

namespace gem5
{

namespace replacement_policy
{

static uint64_t parentIndex(const uint64_t index)
{
    return (index - 1) / 2;
}

static uint64_t leftSubtreeIndex(const uint64_t index)
{
    return 2 * index + 1;
}

static uint64_t rightSubtreeIndex(const uint64_t index)
{
    return 2 * index + 2;
}

static bool isRightSubtree(const uint64_t index)
{
    return index % 2 == 0;
}

TreePLRURP::TreePLRUReplData::TreePLRUReplData(
    const uint64_t index, std::shared_ptr<PLRUTree> tree)
  : index(index), tree(tree)
{
}

TreePLRURP::TreePLRURP(const Params *p)
  : BaseReplacementPolicy(p),
    numLeaves(p->num_leaves), count(0), treeInstance(nullptr)
{
    fatal_if(numLeaves < 1, "numLeaves should never be 0");
}

void TreePLRURP::invalidate
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<TreePLRUReplData> data =
        std::static_pointer_cast<TreePLRUReplData>(replacement_data);
    PLRUTree *tree = data->tree.get();
    uint64_t index = data->index;

    while (index != 0) {
        bool right = isRightSubtree(index);
        index = parentIndex(index);
        (*tree)[index] = right;
    }
}

void TreePLRURP::touch
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<TreePLRUReplData> data =
        std::static_pointer_cast<TreePLRUReplData>(replacement_data);
    PLRUTree *tree = data->tree.get();
    uint64_t index = data->index;

    while (index != 0) {
        bool right = isRightSubtree(index);
        index = parentIndex(index);
        (*tree)[index] = !right;
    }
}

void TreePLRURP::reset
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    touch(replacement_data);
}

ReplaceableEntry* TreePLRURP::getVictim
(const ReplacementCandidates& candidates) const
{
    assert(!candidates.empty());

    const PLRUTree *tree = std::static_pointer_cast<TreePLRUReplData>(
        candidates[0]->replacementData)->tree.get();

    uint64_t index = 0;
    while (index < tree->size()) {
        if ((*tree)[index])
            index = rightSubtreeIndex(index);
        else
            index = leftSubtreeIndex(index);
    }

    return candidates.at(index - (numLeaves - 1));
}

std::shared_ptr<ReplacementData> TreePLRURP::instantiateEntry()
{
    if (count % numLeaves == 0) {
        treeInstance = new PLRUTree(numLeaves - 1, false);
    }

    TreePLRUReplData *data = new TreePLRUReplData(
        (count % numLeaves) + numLeaves - 1,
        std::shared_ptr<PLRUTree>(treeInstance));

    count++;
    return std::shared_ptr<ReplacementData>(data);
}


} // namespace replacement_policy

TreePLRURP* gem5::TreePLRURPParams::create()
{
    return new replacement_policy::TreePLRURP(this);
}

} // namespace gem5

