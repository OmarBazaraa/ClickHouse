#pragma once

#include <Storages/MergeTree/MergeTreeThreadSelectProcessor.h>
#include <Storages/MergeTree/MergeTreeData.h>
#include <Storages/MergeTree/MarkRange.h>
#include <Storages/MergeTree/MergeTreeBlockReadUtils.h>
#include <Storages/SelectQueryInfo.h>


namespace DB
{


/// Used to read data from single part with select query
/// Cares about PREWHERE, virtual columns, indexes etc.
/// To read data from multiple parts, Storage (MergeTree) creates multiple such objects.
class MergeTreeReverseSelectProcessor : public MergeTreeBaseSelectProcessor
{
public:
    MergeTreeReverseSelectProcessor(
        const MergeTreeData & storage,
        const StorageMetadataPtr & metadata_snapshot,
        const MergeTreeData::DataPartPtr & owned_data_part,
        UInt64 max_block_size_rows,
        size_t preferred_block_size_bytes,
        size_t preferred_max_column_in_block_size_bytes,
        Names required_columns_,
        MarkRanges mark_ranges,
        bool use_uncompressed_cache,
        const PrewhereInfoPtr & prewhere_info,
        ExpressionActionsSettings actions_settings,
        bool check_columns,
        const MergeTreeReaderSettings & reader_settings,
        const Names & virt_column_names = {},
        size_t part_index_in_query = 0,
        bool quiet = false);

    ~MergeTreeReverseSelectProcessor() override;

    String getName() const override { return "MergeTreeReverse"; }

    /// Closes readers and unlock part locks
    void finish();

protected:

    bool getNewTask() override;
    Chunk readFromPart() override;

private:
    Block header;

    /// Used by Task
    Names required_columns;
    /// Names from header. Used in order to order columns in read blocks.
    Names ordered_names;
    NameSet column_name_set;

    MergeTreeReadTaskColumns task_columns;

    /// Data part will not be removed if the pointer owns it
    MergeTreeData::DataPartPtr data_part;

    /// Mark ranges we should read (in ascending order)
    MarkRanges all_mark_ranges;
    /// Total number of marks we should read
    size_t total_marks_count = 0;
    /// Value of _part_index virtual column (used only in SelectExecutor)
    size_t part_index_in_query = 0;

    String path;

    Chunks chunks;

    Poco::Logger * log = &Poco::Logger::get("MergeTreeReverseSelectProcessor");
};

}
