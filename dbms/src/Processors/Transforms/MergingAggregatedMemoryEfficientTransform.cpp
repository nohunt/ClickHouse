#include <Processors/Transforms/MergingAggregatedMemoryEfficientTransform.h>

#include <Processors/ISimpleTransform.h>
#include <Interpreters/Aggregator.h>

namespace DB
{

struct ChunksToMerge : public ChunkInfo
{
    std::unique_ptr<Chunks> chunks;
};

class MergingAggregatedSimpleTransform : public ISimpleTransform
{
public:
    /// Aggregator must exists until all data will be processed.
    MergingAggregatedSimpleTransform(Block header, Aggregator & aggregator, bool final)
        : ISimpleTransform(std::move(header), aggregator.getHeader(final), false), aggregator(aggregator), final(final)
    {
        setInputNotNeededAfterRead(true);
    }

protected:

    void transform(Chunk & chunk) override
    {
        auto & info = chunk.getChunkInfo();
        auto * chunks_to_merge = typeid_cast<const ChunksToMerge *>(info.get());

        if (!chunks_to_merge)
            throw Exception("MergingAggregatedSimpleTransform chunk must have ChunkInfo with type ChunksToMerge.",
                    ErrorCodes::LOGICAL_ERROR);

        BlocksList blocks_list;
        for (auto & cur_chunk : *chunks_to_merge->chunks)
            blocks_list.emplace_back(getInputPort().getHeader().cloneWithColumns(cur_chunk.detachColumns()));

        chunk.setChunkInfo(nullptr);

        auto block = aggregator.mergeBlocks(blocks_list, final);
        size_t num_rows = block.rows();
        chunk.setColumns(block.getColumns(), num_rows);
    }

private:
    Aggregator & aggregator;
    bool final;
};

}
