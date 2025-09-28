#ifndef NW4HBM_LYT_MATERIAL_H
#define NW4HBM_LYT_MATERIAL_H

#include "revolution/types.h"

#include "revolution/tpl.h"

#include "revolution/gx/GXTypes.h"

#include "revolution/hbm/nw4hbm/lyt/animation.h"
#include "revolution/hbm/nw4hbm/lyt/resources.h"
#include "revolution/hbm/nw4hbm/lyt/types.h"

#include "revolution/hbm/nw4hbm/ut/Color.h"

namespace nw4hbm {
namespace lyt {
class Material;

namespace detail {
typedef struct BitGXNums {
    u32 texMap : 4; // 11110000000000000000000000000000
    u32 texSRT : 4; // 00001111000000000000000000000000
    u32 texCoordGen : 4; // 00000000111100000000000000000000
    u32 indSRT : 2; // 00000000000011000000000000000000
    u32 indStage : 3; // 00000000000000111000000000000000
    u32 tevSwap : 1; // 00000000000000000100000000000000
    u32 tevStage : 5; // 00000000000000000011111000000000
    u32 chanCtrl : 1; // 00000000000000000000000100000000
    u32 matCol : 1; // 00000000000000000000000010000000
    u32 alpComp : 1; // 00000000000000000000000001000000
    u32 blendMode : 1; // 00000000000000000000000000100000
} BitGXNums;
Size GetTextureSize(Material* pMaterial, u8 texMapIdx);
} // namespace detail

class Material {
  public:
    Material();
    Material(const res::Material* pRes, const ResBlockSet& resBlockSet);
    virtual ~Material();

    virtual bool SetupGX(bool bModVtxCol, u8 alpha);

    virtual void BindAnimation(AnimTransform* animTrans);
    virtual void UnbindAnimation(AnimTransform* animTrans);
    virtual void UnbindAllAnimation();

    virtual void Animate();

    virtual AnimationLink* FindAnimationLink(AnimTransform* animTrans);

    virtual void SetAnimationEnable(AnimTransform* animTrans, bool bEnable);

    const char* GetName() const { return mName; }
    GXColorS10 GetTevColor(u32 idx) const { return mTevCols[idx]; }

    u8 GetTextureCap() const { return mGXMemCap.texMap; }

    u8 GetTexSRTCap() const { return mGXMemCap.texSRT; }

    u8 GetTexCoordGenCap() const { return mGXMemCap.texCoordGen; }

    u8 GetIndTexSRTCap() const { return mGXMemCap.indSRT; }

    bool IsTevSwapCap() const { return static_cast<bool>(mGXMemCap.tevSwap); }
    bool IsBlendModeCap() const { return static_cast<bool>(mGXMemCap.blendMode); }
    bool IsAlphaCompareCap() const { return static_cast<bool>(mGXMemCap.alpComp); }

    bool IsMatColorCap() const { return static_cast<bool>(mGXMemCap.matCol); }
    bool IsChanCtrlCap() const { return static_cast<bool>(mGXMemCap.chanCtrl); }

    u8 GetTextureNum() const { return mGXMemNum.texMap; }

    bool IsUserAllocated() const { return mbUserAllocated; }

    void SetName(const char* name);

    const GXTexObj* GetTexMapAry() const;
    const TexSRT* GetTexSRTAry() const;
    const TexCoordGen* GetTexCoordGenAry() const;
    const ChanCtrl* GetChanCtrlAry() const;
    const ut::Color* GetMatColAry() const;
    const TevSwapMode* GetTevSwapAry() const;
    const AlphaCompare* GetAlphaComparePtr() const;
    const BlendMode* GetBlendModePtr() const;
    const IndirectStage* GetIndirectStageAry() const;
    const TexSRT* GetIndTexSRTAry() const;
    const TevStage* GetTevStageAry() const;

    GXTexObj* GetTexMapAry();
    TexSRT* GetTexSRTAry();
    TexCoordGen* GetTexCoordGenAry();
    ChanCtrl* GetChanCtrlAry();
    ut::Color* GetMatColAry();
    TevSwapMode* GetTevSwapAry();
    AlphaCompare* GetAlphaComparePtr();
    BlendMode* GetBlendModePtr();
    IndirectStage* GetIndirectStageAry();
    TexSRT* GetIndTexSRTAry();
    TevStage* GetTevStageAry();

    void GetTexture(GXTexObj* pTexObj, u8 texMapIdx) const;

    void SetTextureNum(u8 num);
    void SetTexCoordGenNum(u8 num);
    void SetIndStageNum(u8 num);
    void SetTevStageNum(u8 num);

    void SetTexture(u8 texMapIdx, const GXTexObj& texObj);
    void SetTexture(u8 texMapIdx, TPLPalette* pTplRes);
    void SetTextureNoWrap(u8 texMapIdx, TPLPalette* pTplRes);

    void SetColorElement(u32 colorType, s16 value);

    void SetTexCoordGen(u32 idx, TexCoordGen value) {
        NW4HBMAssert_Line(idx < mGXMemNum.texCoordGen, 180);
        GetTexCoordGenAry()[idx] = value;
    }

    void SetTexSRTElement(u32 texSRTIdx, u32 eleIdx, f32 value) {
        NW4HBMAssert_Line(texSRTIdx < mGXMemNum.texSRT, 293);
        f32* srtAry = reinterpret_cast<f32*>(&GetTexSRTAry()[texSRTIdx]);
        srtAry[eleIdx] = value;
    }

    void SetIndTexSRTElement(u32 texSRTIdx, u32 eleIdx, f32 value) {
        NW4HBMAssert_Line(texSRTIdx < mGXMemNum.indSRT, 309);
        f32* srtAry = reinterpret_cast<f32*>(&GetIndTexSRTAry()[texSRTIdx]);
        srtAry[eleIdx] = value;
    }

    void Init();
    void InitBitGXNums(detail::BitGXNums* ptr);

    void ReserveGXMem(u8 texMapNum, u8 texSRTNum, u8 texCoordGenNum, u8 tevStageNum, bool allocTevSwap, u8 indStageNum,
                      u8 indSRTNum, bool allocChanCtrl, bool allocMatCol, bool allocAlpComp, bool allocBlendMode);

    void AddAnimationLink(AnimationLink* animationLink);

  private:
    static const int MAX_TEX_SRT = (GX_TEXMTX9 - GX_TEXMTX0) / 3 + 1;
    static const int MAX_IND_SRT = (GX_ITM_2 - GX_ITM_0) + 1;

    char mName[20]; // 0x04

    AnimationLinkList mAnimList; // 0x18

    GXColorS10 mTevCols[TEVCOLOR_MAX]; // 0x24
    ut::Color mTevKCols[GX_MAX_KCOLOR]; // 0x3C

    detail::BitGXNums mGXMemCap; // 0x4C
    detail::BitGXNums mGXMemNum; // 0x50

    bool mbUserAllocated; // 0x54

    void* mpGXMem; // 0x58
};
} // namespace lyt
} // namespace nw4hbm

#endif // NW4HBM_LYT_MATERIAL_H
