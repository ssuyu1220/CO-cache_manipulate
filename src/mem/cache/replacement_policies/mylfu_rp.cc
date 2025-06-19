#include "mem/cache/replacement_policies/mylfu_rp.hh"

#include <cassert>
#include <memory>

#include "params/MYLFURP.hh"

MYLFURP::MYLFURP(const Params *p)
    : BaseReplacementPolicy(p)
{
}

void
MYLFURP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    // Reset reference count
    std::static_pointer_cast<MYLFUReplData>(replacement_data)->refCount = 0;
}

void
MYLFURP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update reference count
    std::static_pointer_cast<MYLFUReplData>(replacement_data)->refCount++;
}

void
MYLFURP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Reset reference count
    std::static_pointer_cast<MYLFUReplData>(replacement_data)->refCount = 1;
}

ReplaceableEntry*
MYLFURP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<MYLFUReplData>(
                    candidate->replacementData)->refCount <
                std::static_pointer_cast<MYLFUReplData>(
                    victim->replacementData)->refCount) {
            victim = candidate;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
MYLFURP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new MYLFUReplData());
}

MYLFURP*
MYLFURPParams::create()
{
    return new MYLFURP(this);
}
