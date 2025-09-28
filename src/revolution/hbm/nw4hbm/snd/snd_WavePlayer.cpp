#include "revolution/hbm/nw4hbm/snd/WaveFile.h"
#include "revolution/hbm/nw4hbm/snd/WavePlayer.h"

#include "revolution/hbm/nw4hbm/snd/AxManager.h"

#include "revolution/wpad.h"

#include "decomp.h"

namespace nw4hbm {
namespace snd {

namespace {
WavePlayerList sPlayerList;
}

// DECOMP_FORCE(__FILE__);
// DECOMP_FORCE(NW4HBMAssertHeaderClampedLRValue_String(setupParam.channelCount));

WavePlayer::WavePlayer()
    : mStartFlag(false), mVoiceStartFlag(false), mChannelCount(0), mVoice(nullptr), mLoopSetFlag(false),
      mCallback(nullptr), mCallbackArg(nullptr), mPauseFlag(false), mPlaySampleCount(0) {
    InitParam();
}

WavePlayer::~WavePlayer() { Shutdown(); }

void WavePlayer::InitParam() {
    mVolume = 1.0f;
    mPitch = 1.0f;
    mPan = 0.0f;
    mSurroundPan = 0.0f;
    mLpfFreq = 0.0f;
    mOutputLineFlag = OUTPUT_LINE_MAIN;
    mMainOutVolume = 1.0f;
    mMainSend = 0.0f;

    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mFxSend[i] = 0.0f;
    }

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mRemoteOutVolume[i] = 1.0f;
        mRemoteSend[i] = 0.0f;
        mRemoteFxSend[i] = 0.0f;
    }
}

bool WavePlayer::Setup(const SetupParam& setupParam, WavePacketCallback callback, void* callbackArg) {
    NW4HBMAssertHeaderClampedLRValue(setupParam.channelCount, 0, CHANNEL_MAX);
    NW4HBMAssertHeaderClampedLRValue(setupParam.voiceCount, 0, VOICE_MAX);
    int channels = ut::Clamp(setupParam.channelCount, 1, CHANNEL_MAX);
    int voices = ut::Clamp(setupParam.voiceCount, 1, VOICE_MAX);

    Shutdown();

    detail::AxVoice* axVoice =
        detail::AxManager::GetInstance().AllocVoice(channels, voices, PRIORITY_MAX, VoiceCallbackFunc, this);
    if (axVoice == nullptr) {
        return false;
    }
    mVoice = axVoice;

    mChannelCount = channels;
    mSampleFormat = setupParam.sampleFormat;
    mSampleRate = setupParam.sampleRate;
    mPitchMax = setupParam.pitchMax;

    mCallback = callback;
    mCallbackArg = callbackArg;

    return true;
}

void WavePlayer::Shutdown() {
    ut::AutoInterruptLock lock;

    Stop();

    if (mVoice != nullptr) {
        mVoice->Free();
    }
    mVoice = nullptr;

    mChannelCount = 0;
}

bool WavePlayer::AppendWavePacket(WavePacket* packet) {
    if (packet->mAppendFlag) {
        return false;
    }

    if (packet->mWaveBuffer.channelCount < mChannelCount) {
        return false;
    }

    u32 bufSize = packet->mWaveBuffer.bufferSize;
    switch (mSampleFormat) {
        case SAMPLE_FORMAT_PCM_S16: {
            bufSize >>= 1;
            break;
        }
        case SAMPLE_FORMAT_PCM_S8: {
            break;
        }
        default: {
            NW4HBMPanic();
            return false;
        }
    }

    f32 val = ((bufSize * 1000) / mSampleRate);
    if (val < (6.0f * mPitchMax)) {
        return false;
    }

    ut::AutoInterruptLock lock;

    mWavePacketList.PushBack(packet);
    packet->mAppendFlag = true;

    return true;
}

DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(wavePacket));
DECOMP_FORCE("WavePacket is busy.");
DECOMP_FORCE("not enough buffer count in WavePacket.");
DECOMP_FORCE("Too small wave buffer.");

void WavePlayer::Start() {
    ut::AutoInterruptLock lock;

    if (mPauseFlag) {
        Pause(false);
    }

    if (!mStartFlag) {
        sPlayerList.PushBack(this);
        mStartFlag = true;
    }

    mPlaySampleCount = 0;

    if (!mWavePacketList.IsEmpty()) {
        StartVoice();
    }
}

void WavePlayer::Stop() {
    ut::AutoInterruptLock lock;

    if (mStartFlag) {
        sPlayerList.Erase(this);
        mStartFlag = false;
    }

    StopVoice();

    for (WavePacketList::Iterator it = mWavePacketList.GetBeginIter(); it != mWavePacketList.GetEndIter();) {
        WavePacketList::Iterator currIt = it++;

        currIt->mAppendFlag = false;
        mWavePacketList.Erase(currIt);

        if (mCallback != nullptr) {
            mCallback(WAVE_PACKET_CALLBACK_STATUS_CANCEL, this, &*currIt, mCallbackArg);
        }
    }

    NW4HBMAssert_Line(mWavePacketList.IsEmpty(), 354);
    mPauseFlag = false;
}

DECOMP_FORCE("Too large pitch.");
DECOMP_FORCE(NW4HBMAssertHeaderClampedLValue_String(bus));
DECOMP_FORCE(NW4HBMAssertHeaderClampedLValue_String(remoteIndex));

void WavePlayer::Pause(bool flag) {
    ut::AutoInterruptLock lock;

    if (mStartFlag == false) {
        return;
    }

    if (mPauseFlag == flag) {
        return;
    }

    mPauseFlag = flag;
    mVoice->Pause(mPauseFlag);
}

void WavePlayer::StartVoice() {
    // clang-format off
    NW4HBMAssert_Line(! mWavePacketList.IsEmpty(), 503);
    NW4HBMAssertPointerNonnull_Line(mVoice, 504);
    // clang-format on

    ut::AutoInterruptLock lock;

    const WavePacket& wavePacket = mWavePacketList.GetFront();

    detail::WaveData waveData;
    detail::AxVoice::Format axVoiceFormat;

    switch (mSampleFormat) {
        case SAMPLE_FORMAT_PCM_S16: {
            waveData.format = detail::WaveFile::FORMAT_PCM16;
            axVoiceFormat = detail::AxVoice::FORMAT_PCM16;
            break;
        }
        case SAMPLE_FORMAT_PCM_S8: {
            waveData.format = detail::WaveFile::FORMAT_PCM8;
            axVoiceFormat = detail::AxVoice::FORMAT_PCM8;
            break;
        }
        default: {
            NW4HBMPanic_Line(523);
            return;
        }
    }
    waveData.loopFlag = false;
    waveData.numChannels = static_cast<u8>(mChannelCount);
    waveData.sampleRate = static_cast<u16>(mSampleRate);
    waveData.loopStart = 0;
    waveData.loopEnd = detail::AxVoice::GetSampleByByte(wavePacket.mWaveBuffer.bufferSize, axVoiceFormat);

    for (int i = 0; i < mChannelCount; i++) {
        detail::ChannelParam& channelParam = waveData.channelParam[i];
        channelParam.dataAddr = wavePacket.mWaveBuffer.bufferAddress[i];
    }

    mVoice->Setup(waveData);
    mVoice->Start();
    mVoice->Pause(mPauseFlag);

    mVoiceStartFlag = true;
    mLoopSetFlag = false;
}

DECOMP_FORCE("Unknown AxVoice callback status %d");

void WavePlayer::StopVoice() {
    ut::AutoInterruptLock lock;

    if (mVoice != nullptr) {
        mVoice->Stop();
    }
    mVoiceStartFlag = false;
}

void WavePlayer::VoiceCallbackFunc(detail::AxVoice* voice, detail::AxVoice::CallbackStatus status, void* arg) {
    WavePlayer* wavePlayer = reinterpret_cast<WavePlayer*>(arg);
    switch (status) {
        case detail::AxVoice::CALLBACK_STATUS_FINISH_WAVE:
        case detail::AxVoice::CALLBACK_STATUS_INVALIDATE_WAVE: {
            wavePlayer->StopVoice();

            if (!wavePlayer->mWavePacketList.IsEmpty()) {
                WavePacket& wavePacket = wavePlayer->mWavePacketList.GetFront();
                wavePacket.mAppendFlag = false;
                wavePlayer->mWavePacketList.PopFront();

                if (wavePlayer->mCallback != nullptr) {
                    wavePlayer->mCallback(WAVE_PACKET_CALLBACK_STATUS_FINISH, wavePlayer, &wavePacket,
                                          wavePlayer->mCallbackArg);
                }
            }
            break;
        }
        case detail::AxVoice::CALLBACK_STATUS_DROP_VOICE:
        case detail::AxVoice::CALLBACK_STATUS_DROP_DSP: {
            wavePlayer->mVoice = nullptr;
            wavePlayer->Shutdown();
            break;
        }
        default: {
            break;
        }
    }
}

void WavePlayer::SetNextWavePacket() {
    if (mLoopSetFlag) {
        return;
    }

    if (mWavePacketList.GetSize() < 2) {
        return;
    }

    WavePacketList::Iterator it = mWavePacketList.GetBeginIter();
    it++;

    for (int i = 0; i < mChannelCount; i++) {
        mVoice->SetLoopStart(i, it->mWaveBuffer.bufferAddress[i], 0);
    }
    mVoice->SetLoopFlag(true);
    mLoopSetFlag = true;
}

bool WavePlayer::IsNextWavePacket() const {
    if (mVoice == nullptr) {
        return false;
    }
    if (mVoice->IsPlayFinished()) {
        return false;
    }
    if (mWavePacketList.IsEmpty()) {
        return false;
    }

    const WavePacket& wavePacket = mWavePacketList.GetFront();
    u32 addr = mVoice->GetCurrentPlayingDspAddress();

    detail::AxVoice::Format format = mVoice->GetFormat();
    u32 samples = detail::AxVoice::GetSampleByByte(wavePacket.mWaveBuffer.bufferSize, format);

    u32 addrBegin = detail::AxVoice::GetDspAddressBySample(wavePacket.mWaveBuffer.bufferAddress[0], 0, format);
    u32 addrEnd = detail::AxVoice::GetDspAddressBySample(wavePacket.mWaveBuffer.bufferAddress[0], samples, format);

    if (addr < addrBegin || addrEnd <= addr) {
        return true;
    }

    return false;
}

void WavePlayer::UpdateWavePacket() {
    WavePacket& wavePacket = mWavePacketList.GetFront();
    wavePacket.mAppendFlag = false;

    mWavePacketList.PopFront();

    u32 samples = wavePacket.mWaveBuffer.bufferSize;
    if (mSampleFormat == SAMPLE_FORMAT_PCM_S16) {
        samples >>= 1;
    }

    mPlaySampleCount += samples;

    if (mCallback != nullptr) {
        mCallback(WAVE_PACKET_CALLBACK_STATUS_FINISH, this, &wavePacket, mCallbackArg);
    }

    if (!mWavePacketList.IsEmpty()) {
        WavePacket& wavePacket = mWavePacketList.GetFront();

        detail::AxVoice::Format axFormat = mVoice->GetFormat();
        u32 samples = detail::AxVoice::GetSampleByByte(wavePacket.mWaveBuffer.bufferSize, axFormat);

        for (int i = 0; i < mChannelCount; i++) {
            mVoice->SetBaseAddress(i, wavePacket.mWaveBuffer.bufferAddress[i]);
            mVoice->StopAtPoint(i, wavePacket.mWaveBuffer.bufferAddress[i], samples);
        }
    }

    mLoopSetFlag = false;
}

void WavePlayer::detail_UpdateBufferAllPlayers() {
    for (WavePlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        WavePlayerList::Iterator currIt = it++;
        currIt->detail_UpdateBuffer();
    }
}

void WavePlayer::detail_UpdateBuffer() {
    if (mVoiceStartFlag) {
        if (IsNextWavePacket()) {
            UpdateWavePacket();
        }

        SetNextWavePacket();
    } else {
        if (!mWavePacketList.IsEmpty()) {
            StartVoice();
        }
    }
}

void WavePlayer::detail_Update() {
    if (mVoice == nullptr) {
        return;
    }

    mVoice->SetVolume(mVolume);
    mVoice->SetPitch(mPitch);
    mVoice->SetPan(mPan);
    mVoice->SetSurroundPan(mSurroundPan);
    mVoice->SetLpfFreq(mLpfFreq + 1.0f);
    mVoice->SetOutputLine(mOutputLineFlag);
    mVoice->SetMainOutVolume(mMainOutVolume);
    mVoice->SetMainSend(mMainSend);

    for (int i = 0; i < AUX_BUS_NUM; i++) {
        mVoice->SetFxSend(static_cast<AuxBus>(i), mFxSend[i]);
    }

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mVoice->SetRemoteOutVolume(i, mRemoteOutVolume[i]);
        mVoice->SetRemoteSend(i, mRemoteSend[i]);
        mVoice->SetRemoteFxSend(i, mRemoteSend[i]);
    }
}

void WavePlayer::detail_UpdateAllPlayers() {
    for (WavePlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        WavePlayerList::Iterator currIt = it++;
        currIt->detail_Update();
    }
}

void WavePlayer::detail_StopAllPlayers() {
    for (WavePlayerList::Iterator it = sPlayerList.GetBeginIter(); it != sPlayerList.GetEndIter();) {
        WavePlayerList::Iterator currIt = it++;
        currIt->Stop();
    }

    NW4HBMAssert_Line(sPlayerList.IsEmpty(), 793);
}

WavePlayer::WavePacket::WavePacket() : mWaveBuffer(), mAppendFlag(false) {
    for (int i = 0; i < CHANNEL_MAX; i++) {
        mWaveBuffer.bufferAddress[i] = nullptr;
    }
}

} // namespace snd
} // namespace nw4hbm
