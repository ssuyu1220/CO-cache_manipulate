#include "mem/cache/replacement_policies/tree_plru_rp.hh"

#include <cassert>
#include <cmath>
#include <memory>

#include "base/logging.hh"
#include "base/types.hh"
#include "params/TreePLRURP.hh"

TreePLRURP::TreePLRUReplData::TreePLRUReplData(
    const uint64_t index, std::shared_ptr<PLRUTree> tree)
  : index(index), tree(tree)
{
}

TreePLRURP::TreePLRURP(const TreePLRURPParams *p)
  : BaseReplacementPolicy(p),
    numLeaves(p->num_leaves), count(0), treeInstance(nullptr)
{
    fatal_if(numLeaves < 1, "numLeaves should never be 0");
}

void
TreePLRURP::invalidate
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    auto data = std::static_pointer_cast<TreePLRUReplData>(replacement_data);
    PLRUTree *tree = data->tree.get();
    uint64_t index = data->index;

    while (index != 0) {
        bool right = (index % 2 == 0);
        index = (index - 1) / 2;
        (*tree)[index] = right;
    }
}

void
TreePLRURP::touch
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    auto data = std::static_pointer_cast<TreePLRUReplData>(replacement_data);
    PLRUTree *tree = data->tree.get();
    uint64_t index = data->index;

    while (index != 0) {
        bool right = (index % 2 == 0);
        index = (index - 1) / 2;
        (*tree)[index] = !right;
    }
}

void
TreePLRURP::reset
(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    touch(replacement_data);
}

ReplaceableEntry*
TreePLRURP::getVictim
(const ReplacementCandidates& candidates) const
{
    assert(!candidates.empty());

    const PLRUTree *tree = std::static_pointer_cast<TreePLRUReplData>(
        candidates[0]->replacementData)->tree.get();

    uint64_t index = 0;
    while (index < tree->size()) {
        if ((*tree)[index]) {
            index = 2 * index + 2; // right
        } else {
            index = 2 * index + 1; // left
        }
    }

    return candidates.at(index - (numLeaves - 1));
}

std::shared_ptr<ReplacementData>
TreePLRURP::instantiateEntry()
{
    if (count % numLeaves == 0) {
        treeInstance = new PLRUTree(numLeaves - 1, false);
    }

    auto data = new TreePLRUReplData(
        (count % numLeaves) + numLeaves - 1,
        std::shared_ptr<PLRUTree>(treeInstance));

    count++;
    return std::shared_ptr<ReplacementData>(data);
}

TreePLRURP*
TreePLRURPParams::create()
{
    return new TreePLRURP(this);
}

