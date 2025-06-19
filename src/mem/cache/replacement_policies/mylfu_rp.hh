#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_MYLFU_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_MYLFU_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

struct MYLFURPParams;

class MYLFURP : public BaseReplacementPolicy
{
  protected:
    /** MYLFU-specific implementation of replacement data. */
    struct MYLFUReplData : ReplacementData
    {
        /** Number of references to this entry since it was reset. */
        unsigned refCount;

        /**
         * Default constructor. Invalidate data.
         */
        MYLFUReplData() : refCount(0) {}
    };

  public:
    /** Convenience typedef. */
    typedef MYLFURPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    MYLFURP(const Params *p);

    /**
     * Destructor.
     */
    ~MYLFURP() {}

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Clear the number of references.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

    /**
     * Touch an entry to update its replacement data.
     * Increase number of references.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Reset number of references.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using reference frequency.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_MYLFU_RP_HH__
