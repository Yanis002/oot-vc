#include "revolution/hbm/nw4hbm/ut/ut_CharWriter.hpp"

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "macros.h"
#include "revolution/gx.h"
#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Font.hpp"

//! TODO: remove once matched
extern "C" void fn_8010CBAC(char*, int, ...);

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm {
static void SetupGXCommon();
} // namespace nw4hbm

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace ut {
// .bss
/* CharWriter::LoadingTexture CharWriter::mLoadingTexture; */ // pooling
} // namespace ut
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {

void SetupGXCommon() {
    static ut::Color fog(0x00000000);

    GXSetFog(GX_FOG_NONE, fog, 0.0f, 0.0f, 0.0f, 0.0f);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60);
    GXSetNumIndStages(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}

namespace ut {

// pooling
CharWriter::LoadingTexture CharWriter::mLoadingTexture;

CharWriter::CharWriter() : mAlpha(0xff), mIsWidthFixed(false), mFixedWidth(0.0f), mFont(nullptr) {
    mLoadingTexture.Reset();
    ResetColorMapping();
    SetGradationMode(GRADMODE_NONE);
    SetTextColor(0xffffffff);
    SetScale(1.0f, 1.0f);
    SetCursor(0.0f, 0.0f, 0.0f);
    EnableLinearFilter(true, true);
}

CharWriter::~CharWriter() {}

void CharWriter::SetFont(const Font& font) {
    // clang-format off
    NW4HBM_ASSERT_PTR(this, 133);
    NW4HBM_ASSERT_PTR(& font, 134);
    mFont = &font;
    // clang-format on
}

const Font* CharWriter::GetFont() const {
    NW4HBM_ASSERT_PTR(this, 151);
    return mFont;
}

void CharWriter::SetupGX() {
    NW4HBM_ASSERT_PTR(this, 173);
    ResetTextureCache();

    if (mColorMapping.min != 0x00000000 || mColorMapping.max != 0xffffffff) {
        SetupGXWithColorMapping(mColorMapping.min, mColorMapping.max);
    } else if (mFont) {
        GXTexFmt format = mFont->GetTextureFormat();
        switch (format) {
            case GX_TF_I4:
            case GX_TF_I8:
                SetupGXForI();
                break;

            case GX_TF_IA4:
            case GX_TF_IA8:
                SetupGXDefault();
                break;

            case GX_TF_RGB565:
            case GX_TF_RGB5A3:
            case GX_TF_RGBA8:
                SetupGXForRGBA();
                break;

            default:
                fn_8010CBAC(__FILE__, 207, "CharWriter::SetupGX: Unknown font sheet format(=%d)", format);
                SetupGXDefault();
                break;
        }
    } else {
        SetupGXDefault();
    }
}

void CharWriter::SetColorMapping(Color min, Color max) {
    NW4HBM_ASSERT_PTR(this, 235);
    mColorMapping.min = min;
    mColorMapping.max = max;
}

void CharWriter::ResetColorMapping() {
    NW4HBM_ASSERT_PTR(this, 284);
    SetColorMapping(0x00000000, 0xffffffff);
}

void CharWriter::ResetTextureCache() {
    NW4HBM_ASSERT_PTR(this, 300);
    mLoadingTexture.Reset();
}

void CharWriter::SetGradationMode(GradationMode mode) {
    NW4HBM_ASSERT_PTR(this, 355);
    NW4HBM_PANIC(mode >= 0 && mode <= 2 ? false : true, 356,
                 "mode is out of bounds(%d)\n%d <= mode <= %d not satisfied.", mode, 0, 2);
    mTextColor.gradationMode = mode;
    UpdateVertexColor();
}

void CharWriter::SetTextColor(Color color) {
    NW4HBM_ASSERT_PTR(this, 389);
    mTextColor.start = color;
    UpdateVertexColor();
}

void CharWriter::SetTextColor(Color start, Color end) {
    NW4HBM_ASSERT_PTR(this, 410);
    mTextColor.start = start;
    mTextColor.end = end;
    UpdateVertexColor();
}

void CharWriter::SetScale(f32 hScale, f32 vScale) {
    NW4HBM_ASSERT_PTR(this, 487);
    mScale.x = hScale;
    mScale.y = vScale;
}

f32 CharWriter::GetScaleH() const {
    NW4HBM_ASSERT_PTR(this, 522);
    return mScale.x;
}

f32 CharWriter::GetScaleV() const {
    NW4HBM_ASSERT_PTR(this, 538);
    return mScale.y;
}

void CharWriter::SetFontSize(f32 width, f32 height) {
    NW4HBM_ASSERT_PTR(this, 559);
    NW4HBM_ASSERT_PTR(mFont, 560);
    NW4HBM_PANIC(mFont->GetWidth() < 1, 561,
                 "mFont->GetWidth() is out of bounds(%d)\n%d <= mFont->GetWidth() not satisfied.", mFont->GetWidth(),
                 1);
    NW4HBM_PANIC(mFont->GetHeight() < 1, 562,
                 "mFont->GetHeight() is out of bounds(%d)\n%d <= mFont->GetHeight() not satisfied.", mFont->GetHeight(),
                 1);
    SetScale(width / mFont->GetWidth(), height / mFont->GetHeight());
}

f32 CharWriter::GetFontWidth() const {
    NW4HBM_ASSERT_PTR(this, 601);
    NW4HBM_ASSERT_PTR(mFont, 602);
    return mFont->GetWidth() * mScale.x;
}

f32 CharWriter::GetFontHeight() const {
    NW4HBM_ASSERT_PTR(this, 618);
    NW4HBM_ASSERT_PTR(mFont, 619);
    return mFont->GetHeight() * mScale.y;
}

f32 CharWriter::GetFontAscent() const {
    NW4HBM_ASSERT_PTR(this, 635);
    NW4HBM_ASSERT_PTR(mFont, 636);
    return mFont->GetAscent() * mScale.y;
}

void CharWriter::EnableLinearFilter(bool atSmall, bool atLarge) {
    NW4HBM_ASSERT_PTR(this, 681);
    mFilter.atSmall = atSmall ? GX_LINEAR : GX_NEAR;
    mFilter.atLarge = atLarge ? GX_LINEAR : GX_NEAR;
}

bool CharWriter::IsWidthFixed() const {
    NW4HBM_ASSERT_PTR(this, 738);
    return mIsWidthFixed;
}

f32 CharWriter::GetFixedWidth() const {
    NW4HBM_ASSERT_PTR(this, 769);
    return mFixedWidth;
}

f32 CharWriter::Print(char16_t code) {
    NW4HBM_ASSERT_PTR(this, 808);
    NW4HBM_ASSERT_PTR(mFont, 809);
    NW4HBM_ASSERT2(code != Font::INVALID_CHARACTER_CODE, 810);
    Glyph glyph;
    f32 width;
    f32 left;

    mFont->GetGlyph(&glyph, code);

    CharWidths& widths = glyph.widths;

    if (mIsWidthFixed) {
        f32 margin = (mFixedWidth - widths.charWidth * mScale.x) / 2.0f;

        width = mFixedWidth;
        left = margin + widths.left * mScale.x;
    } else {
        width = widths.charWidth * mScale.x;
        left = widths.left * mScale.x;
    }

    PrintGlyph(mCursorPos.x + left, mCursorPos.y, mCursorPos.z, glyph);

    mCursorPos.x += width;
    return width;
}

void CharWriter::SetCursor(f32 x, f32 y) {
    NW4HBM_ASSERT_PTR(this, 879);
    mCursorPos.x = x;
    mCursorPos.y = y;
}

void CharWriter::SetCursor(f32 x, f32 y, f32 z) {
    NW4HBM_ASSERT_PTR(this, 902);
    mCursorPos.x = x;
    mCursorPos.y = y;
    mCursorPos.z = z;
}

void CharWriter::SetCursorX(f32 x) {
    NW4HBM_ASSERT_PTR(this, 965);
    mCursorPos.x = x;
}

void CharWriter::SetCursorY(f32 y) {
    NW4HBM_ASSERT_PTR(this, 981);
    mCursorPos.y = y;
}

void CharWriter::MoveCursorX(f32 dx) {
    NW4HBM_ASSERT_PTR(this, 1013);
    mCursorPos.x += dx;
}

void CharWriter::MoveCursorY(f32 dy) {
    NW4HBM_ASSERT_PTR(this, 1029);
    mCursorPos.y += dy;
}

f32 CharWriter::GetCursorX() const {
    NW4HBM_ASSERT_PTR(this, 1061);
    return mCursorPos.x;
}

f32 CharWriter::GetCursorY() const {
    NW4HBM_ASSERT_PTR(this, 1077);
    return mCursorPos.y;
}

void CharWriter::PrintGlyph(f32 x, f32 y, f32 z, const Glyph& glyph) {
    // clang-format off
    NW4HBM_ASSERT_PTR(this, 1127);
    NW4HBM_ASSERT_PTR(& glyph, 1128);
    NW4HBM_PANIC(glyph.texWidth < 1, 1129,
                 "glyph.texWidth is out of bounds(%d)\n%d <= glyph.texWidth not satisfied.", glyph.texWidth, 1);
    NW4HBM_PANIC(glyph.texHeight < 1, 1130,
                 "glyph.texHeight is out of bounds(%d)\n%d <= glyph.texHeight not satisfied.", glyph.texHeight, 1);
    // clang-format on

    f32 posLeft = x;
    f32 posTop = y;
    f32 posRight = posLeft + glyph.widths.glyphWidth * mScale.x;
    f32 posBottom = posTop + glyph.height * mScale.y;
    f32 posZ = z;
    u16 texLeft = static_cast<u32>(glyph.cellX << 15) / glyph.texWidth;
    u16 texTop = static_cast<u32>(glyph.cellY << 15) / glyph.texHeight;
    u16 texRight = static_cast<u32>((glyph.cellX + glyph.widths.glyphWidth) << 15) / glyph.texWidth;
    u16 texBottom = static_cast<u32>((glyph.cellY + glyph.height) << 15) / glyph.texHeight;

    LoadTexture(glyph, GX_TEXMAP0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(posLeft, posTop, posZ);
    GXColor1u32(mVertexColor.lu);
    GXTexCoord2u16(texLeft, texTop);

    GXPosition3f32(posRight, posTop, posZ);
    GXColor1u32(mVertexColor.ru);
    GXTexCoord2u16(texRight, texTop);

    GXPosition3f32(posRight, posBottom, posZ);
    GXColor1u32(mVertexColor.rd);
    GXTexCoord2u16(texRight, texBottom);

    GXPosition3f32(posLeft, posBottom, posZ);
    GXColor1u32(mVertexColor.ld);
    GXTexCoord2u16(texLeft, texBottom);
    GXEnd();
}

void CharWriter::LoadTexture(const Glyph& glyph, GXTexMapID slot) {
    // clang-format off
    NW4HBM_ASSERT_PTR(this, 1192);
    NW4HBM_ASSERT_PTR(& glyph, 1193);
    // clang-format on

    LoadingTexture loadInfo;
    loadInfo.slot = slot;
    loadInfo.texture = glyph.pTexture;
    loadInfo.filter = mFilter;

    if (loadInfo != mLoadingTexture) {
        GXTexObj tobj;

        GXInitTexObj(&tobj, glyph.pTexture, glyph.texWidth, glyph.texHeight, glyph.texFormat, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(&tobj, mFilter.atSmall, mFilter.atLarge, 0.0f, 0.0f, 0.0f, false, false, GX_ANISO_1);
        GXLoadTexObj(&tobj, slot);

        mLoadingTexture = loadInfo;
    }
}

void CharWriter::UpdateVertexColor() {
    NW4HBM_ASSERT_PTR(this, 1242);

    mVertexColor.lu = mTextColor.start;
    mVertexColor.ru = mTextColor.gradationMode != GRADMODE_H ? mTextColor.start : mTextColor.end;
    mVertexColor.ld = mTextColor.gradationMode != GRADMODE_V ? mTextColor.start : mTextColor.end;
    mVertexColor.rd = mTextColor.gradationMode == GRADMODE_NONE ? mTextColor.start : mTextColor.end;

    mVertexColor.lu.a = mVertexColor.lu.a * mAlpha / 255;
    mVertexColor.ru.a = mVertexColor.ru.a * mAlpha / 255;
    mVertexColor.ld.a = mVertexColor.ld.a * mAlpha / 255;
    mVertexColor.rd.a = mVertexColor.rd.a * mAlpha / 255;
}

void CharWriter::SetupVertexFormat() {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);

    GXClearVtxDesc();

    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
}

void CharWriter::SetupGXDefault() {
    SetupGXCommon();

    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    SetupVertexFormat();
}

void CharWriter::SetupGXWithColorMapping(Color min, Color max) {
    SetupGXCommon();

    GXSetNumTevStages(2);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_TEVREG0, min);
    GXSetTevColor(GX_TEVREG1, max);

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    SetupVertexFormat();
}

void CharWriter::SetupGXForI() {
    SetupGXCommon();

    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    SetupVertexFormat();
}

void CharWriter::SetupGXForRGBA() { SetupGXDefault(); }

} // namespace ut

} // namespace nw4hbm
