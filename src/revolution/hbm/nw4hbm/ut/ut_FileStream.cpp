#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace ut {

const ut::detail::RuntimeTypeInfo FileStream::typeInfo(&IOStream::typeInfo);

void FileStream::Seek(s32 offset, u32 origin) {
    NW4HBMAssertMessage_Line(CanSeek(), 44, "Stream don't support SEEK function\n");
}

void FileStream::Cancel() { NW4HBMAssertMessage_Line(CanCancel(), 60, "Stream don't support CANCEL function\n"); }

bool FileStream::CancelAsync(StreamCallback pCallback, void* pCallbackArg) {
#pragma unused(pCallback)
#pragma unused(pCallbackArg)
    NW4HBMAssertMessage_Line(CanCancel(), 78, "Stream don't support CANCEL function\n");
    NW4HBMAssertMessage_Line(CanAsync(), 79, "Stream don't support ASYNC function\n");
    return true;
}

u32 FileStream::FilePosition::Skip(s32 offset) {
    if (offset != 0) {
        s64 newOffset = mPosition + offset;
        mPosition = Clamp<s64>(newOffset, 0, mFileSize);
    }

    return mPosition;
}

u32 FileStream::FilePosition::Append(s32 offset) {
    s64 newOffset = mPosition + offset;

    if (newOffset < 0) {
        mPosition = 0;
    } else {
        mPosition = newOffset;
        mFileSize = Max(mPosition, mFileSize);
    }

    return mPosition;
}

void FileStream::FilePosition::Seek(s32 offset, u32 origin) {
    switch (origin) {
        case SEEK_BEG: {
            mPosition = 0;
            break;
        }

        case SEEK_END: {
            mPosition = mFileSize;
            break;
        }

        case SEEK_CUR:
        default: {
            break;
        }
    }

    Skip(offset);
}

} // namespace ut
} // namespace nw4hbm
