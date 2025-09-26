#include "revolution/hbm/nw4hbm/snd/SeqFile.h"

#include <cstring.hpp>

#include "macros.h" // NW4HBM_VERSION
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/Util.h"

#include "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::AddOffsetToPtr

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

bool SeqFileReader::IsValidFileHeader(const void* seqData) {
    const ut::BinaryFileHeader* fileHeader = static_cast<const ut::BinaryFileHeader*>(seqData);

    NW4HBMAssert_Line(fileHeader->signature == SeqFile::SIGNATURE_FILE, 38);

    if (fileHeader->signature != SeqFile::SIGNATURE_FILE) {
        return false;
    }

    NW4HBMAssertMessage_Line(Util::ReadBigEndian(fileHeader->version) >= NW4HBM_VERSION(1, 0), 46,
                             "seq file is not supported version.\n  please reconvert file using new version tools.\n");

    u16 version = Util::ReadBigEndian(fileHeader->version);

    if (version < NW4R_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(version <= NW4HBM_VERSION(1, 0), 52,
                             "seq file is not supported version.\n  please reconvert file using new version tools.\n");

    if (Util::ReadBigEndian(fileHeader->version) > SeqFile::FILE_VERSION) {
        return false;
    }

    return true;
}

SeqFileReader::SeqFileReader(const void* seqData) : mHeader(nullptr), mDataBlock(nullptr) {
    NW4HBMAssertPointerNonnull_Line(seqData, 71);

    if (!IsValidFileHeader(seqData)) {
        return;
    }

    mHeader = static_cast<const SeqFile::Header*>(seqData);
    mDataBlock = static_cast<const SeqFile::DataBlock*>(
        ut::AddOffsetToPtr(mHeader, Util::ReadBigEndian(mHeader->dataBlockOffset)));

    NW4HBMAssert_Line(mDataBlock->blockHeader.kind == SeqFile::SIGNATURE_DATA_BLOCK, 80);
}

const void* SeqFileReader::GetBaseAddress() const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 94);
    return ut::AddOffsetToPtr(mDataBlock, Util::ReadBigEndian(mDataBlock->baseOffset));
}

// bool SeqFileReader::ReadOffsetByLabel(char const* labelName, u32* offsetPtr) const {
//     NW4HBMAssertPointerNonnull_Line(offsetPtr, 117);

//     // NOTE: reinterpret_cast necessary instead of static_cast for regalloc(???)
//     SeqFile::LabelBlock const* labelBlock = reinterpret_cast<SeqFile::LabelBlock const*>(
//         ut::AddOffsetToPtr(mHeader, Util::ReadBigEndian(mHeader->labelBlockOffset)));

//     if (!labelBlock) {
//         return false;
//     }

//     u32 labelNameLen = std::strlen(labelName);

//     for (int index = 0; index < Util::ReadBigEndian(labelBlock->labelInfoTable.count); ++index) {
//         u32 ofs = labelBlock->labelInfoTable.items[index];

//         // NOTE: reinterpret_cast necessary here too
//         SeqFile::LabelInfo const* labelInfo =
//             reinterpret_cast<SeqFile::LabelInfo const*>(ut::AddOffsetToPtr(labelBlock, Util::ReadBigEndian(ofs) +
//             8));

//         NW4HBMAssertPointerNonnull_Line(labelInfo, 133);

//         if (labelNameLen == Util::ReadBigEndian(labelInfo->nameLen) &&
//             std::strncmp(labelName, labelInfo->name, labelNameLen) == 0) {
//             *offsetPtr = Util::ReadBigEndian(labelInfo->offset);
//             return true;
//         }
//     }

//     return false;
// }

} // namespace detail
} // namespace snd
} // namespace nw4hbm
