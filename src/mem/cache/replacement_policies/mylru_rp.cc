/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Daniel Carvalho
 */

#include "mem/cache/replacement_policies/mylru_rp.hh"

#include <cassert>
#include <memory>

#include "params/MYLRURP.hh"

MYLRURP::MYLRURP(const Params *p) : BaseReplacementPolicy(p), recentRow(0) {}

Addr
MYLRURP::extractRow(Addr addr) const {
    // Assuming row granularity is 4KB (typical)
    return addr >> 12;
}

void
MYLRURP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    /*
    // Reset last touch timestamp
    std::static_pointer_cast<MYLRUReplData>(
        replacement_data)->lastTouchTick = Tick(0);
    */
    //OPTIMIZE
    auto d = std::static_pointer_cast<MYLRUReplData>(replacement_data);
    d->lastTouchTick = Tick(0);
    d->accessCount = 0;
    d->isDirty = false;
    d->blockAddr = 0;
}

void
MYLRURP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    /*
    // Update last touch timestamp
    std::static_pointer_cast<MYLRUReplData>(
        replacement_data)->lastTouchTick = curTick();
    */

    // OPTIMIZE
    auto d = std::static_pointer_cast<MYLRUReplData>(replacement_data);
    d->accessCount++;
    // 每 4 次 access 才更新 touch tick（lazy promote）
    if (d->accessCount % 4 == 0){
        recentRow = extractRow(d->blockAddr);
        d->lastTouchTick = curTick();
    }
}

void
MYLRURP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    /*
    // Set last touch timestamp
    std::static_pointer_cast<MYLRUReplData>(
        replacement_data)->lastTouchTick = curTick();
    */
    //OPTIMIZE
    auto d = std::static_pointer_cast<MYLRUReplData>(replacement_data);
    d->accessCount = 0;
    d->lastTouchTick = curTick();
    d->isDirty = false; // Reset dirty flag (could add external hook)
    d->blockAddr = 0;
}

ReplaceableEntry*
MYLRURP::getVictim(const ReplacementCandidates& candidates) const
{
    /*
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<MYLRUReplData>(
                    candidate->replacementData)->lastTouchTick <
                std::static_pointer_cast<MYLRUReplData>(
                    victim->replacementData)->lastTouchTick) {
            victim = candidate;
        }
    }

    return victim;
    */
    //OPTIMIZE
    assert(!candidates.empty());

    ReplaceableEntry* victim = nullptr;
    Tick minScore = Tick(-1);  // large initial value

    for (auto& c : candidates) {
        auto d = std::static_pointer_cast<MYLRUReplData>(c->replacementData);

        // Dirty-aware victim scoring
        // Add penalty if the block is dirty to discourage replacing it
        Tick rowPenalty = extractRow(d->blockAddr) != recentRow ? 500000 : 0;
        Tick dirtyPenalty = d->isDirty ? 1000000 : 0;
        Tick score = d->lastTouchTick + rowPenalty + dirtyPenalty;

        if (!victim || score < minScore) {
            victim = c;
            minScore = score;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
MYLRURP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new MYLRUReplData());
}

MYLRURP*
MYLRURPParams::create()
{
    return new MYLRURP(this);
}
