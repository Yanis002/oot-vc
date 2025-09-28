#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/lyt/common.h"
#include "revolution/hbm/nw4hbm/lyt/layout.h"

#include "decomp.h"

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

void ReverseYAxis(math::MTX34* pMtx);
} // unnamed namespace

namespace nw4hbm {
namespace lyt {
NW4HBM_UT_GET_RUNTIME_TYPEINFO(Pane);
}
} // namespace nw4hbm

namespace {

void ReverseYAxis(math::MTX34* pMtx) {
    pMtx->m[0][1] = -pMtx->m[0][1];
    pMtx->m[1][1] = -pMtx->m[1][1];
    pMtx->m[2][1] = -pMtx->m[2][1];
}

} // unnamed namespace

namespace nw4hbm {
namespace lyt {
DECOMP_FORCE_CLASS_METHOD(nw4hbm::lyt::PaneLinkList, GetNodeFromPointer(nullptr));
DECOMP_FORCE_CLASS_METHOD(nw4hbm::lyt::AnimTransformList, GetNodeFromPointer(nullptr));
extern void __deadstrip1();
extern void __deadstrip1() {
    (void)0.0f;
    (void)1.0f;
}

Pane::Pane(const res::Pane* pBlock) {
    Init();

    mBasePosition = pBlock->basePosition;
    SetName(pBlock->name);
    SetUserData(pBlock->userData);

    mTranslate = pBlock->translate;
    mRotate = pBlock->rotate;
    mScale = pBlock->scale;
    mSize = pBlock->size;
    mAlpha = pBlock->alpha;
    mGlbAlpha = mAlpha;
    mFlag = pBlock->flag;
}

void Pane::Init() {
    mpParent = nullptr;
    mpMaterial = nullptr;
    mbUserAllocated = false;
}

Pane::~Pane() {
    for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter();) {
        PaneList::Iterator currIt = it++;
        mChildList.Erase(currIt);

        if (!currIt->IsUserAllocated()) {
            currIt->~Pane();
            Layout::FreeMemory(&*currIt);
        }
    }

    UnbindAnimationSelf(nullptr);

    if (mpMaterial && !mpMaterial->IsUserAllocated()) {
        mpMaterial->~Material();
        Layout::FreeMemory(mpMaterial);
    }
}

void Pane::SetName(const char* name) { std::strncpy(mName, name, sizeof mName); }

void Pane::SetUserData(const char* userData) { std::strncpy(mUserData, userData, sizeof mUserData); }

void Pane::AppendChild(Pane* pChild) { InsertChild(mChildList.GetEndIter(), pChild); }

// extern void _deadstrip2();
// extern void _deadstrip2() { PaneList::GetNodeFromPointer(nullptr); }




void Pane::InsertChild(PaneList::Iterator next, Pane* pChild) {
    NW4HBMAssertPointerNonnull_Line(pChild, 253);
    NW4HBMAssert_Line(pChild->mpParent == 0, 254);
    mChildList.Insert(next, pChild);
    pChild->mpParent = this;
}

DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(pNext));
DECOMP_FORCE(NW4HBMAssert_String(pNext->mpParent == this));
DECOMP_FORCE(NW4HBMAssert_String(pChild->mpParent == this));

#pragma ppc_iro_level 0

const ut::Rect Pane::GetPaneRect(const DrawInfo& drawInfo) const {
    ut::Rect ret;
    math::VEC2 basePt = GetVtxPos();

    ret.left = basePt.x;
    ret.top = basePt.y;
    ret.right = ret.left + mSize.width;
    ret.bottom = ret.top + mSize.height;

    if (drawInfo.IsYAxisUp()) {
        ret.top = -ret.top;
        ret.bottom = -ret.bottom;
    }

    return ret;
}

#pragma ppc_iro_level reset

ut::Color Pane::GetVtxColor(u32) const { return ut::Color(0xFFFFFFFF); }

void Pane::SetVtxColor(u32, ut::Color) { /* ... */ }

u8 Pane::GetColorElement(u32 idx) const {
    NW4HBMAssert_Line(idx < ANIMTARGET_PANE_COLOR_MAX, 319);
    switch (idx) {
        case 16:
            return mAlpha;

        default:
            return GetVtxColorElement(idx);
    }
}

DECOMP_FORCE_CLASS_METHOD(nw4hbm::lyt::GroupList, GetNodeFromPointer(nullptr));

void Pane::SetColorElement(u32 idx, u8 value) {
    NW4HBMAssert_Line(idx < ANIMTARGET_PANE_COLOR_MAX, 334);
    switch (idx) {
        case 16:
            mAlpha = value;
            break;

        default:
            SetVtxColorElement(idx, value);
            break;
    }
}

u8 Pane::GetVtxColorElement(u32) const { return 0xff; }

void Pane::SetVtxColorElement(u32, u8) { /* ... */ }

Pane* Pane::FindPaneByName(const char* findName, bool bRecursive) {
    if (detail::EqualsPaneName(mName, findName)) {
        return this;
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            if (Pane* pane = it->FindPaneByName(findName, true)) {
                return pane;
            }
        }
    }

    return nullptr;
}

Material* Pane::FindMaterialByName(const char* findName, bool bRecursive) {
    if (mpMaterial && detail::EqualsMaterialName(mpMaterial->GetName(), findName)) {
        return mpMaterial;
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            if (Material* pMaterial = it->FindMaterialByName(findName, true)) {
                return pMaterial;
            }
        }
    }

    return nullptr;
}

void Pane::CalculateMtx(const DrawInfo& drawInfo) {
    // this doesnt even get referenced, an entirely new constant is generated lol
    const f32 invAlpha = 1.0f / 255.0f;

    if (!detail::TestBit(mFlag, 0) && !drawInfo.IsInvisiblePaneCalculateMtx()) {
        return;
    }

    {
        math::MTX34 mtx1;
        math::MTX34 mtx2;
        math::MTX34 rotateMtx;

        {
            math::VEC2 scale(mScale);

            if (drawInfo.IsLocationAdjust() && detail::TestBit(mFlag, 2)) {
                scale.x *= drawInfo.GetLocationAdjustScale().x;
                scale.y *= drawInfo.GetLocationAdjustScale().y;
            }

            PSMTXScale(mtx2, scale.x, scale.y, 1.0f);
        }

        PSMTXRotRad(rotateMtx, 'x', DEG_TO_RAD(mRotate.x));
        PSMTXConcat(rotateMtx, mtx2, mtx1);

        PSMTXRotRad(rotateMtx, 'y', DEG_TO_RAD(mRotate.y));
        PSMTXConcat(rotateMtx, mtx1, mtx2);

        PSMTXRotRad(rotateMtx, 'z', DEG_TO_RAD(mRotate.z));
        PSMTXConcat(rotateMtx, mtx2, mtx1);

        PSMTXTransApply(mtx1, mMtx, mTranslate.x, mTranslate.y, mTranslate.z);
    }

    if (mpParent) {
        math::MTX34Mult(&mGlbMtx, &mpParent->mGlbMtx, &mMtx);
    } else if (drawInfo.IsMultipleViewMtxOnDraw()) {
        mGlbMtx = mMtx;
    } else {
        math::MTX34Mult(&mGlbMtx, &drawInfo.GetViewMtx(), &mMtx);
    }

    if (drawInfo.IsInfluencedAlpha() && mpParent) {
        mGlbAlpha = mAlpha * drawInfo.GetGlobalAlpha();
    } else {
        mGlbAlpha = mAlpha;
    }

    // cr = const ref?
    f32 crGlobalAlpha = drawInfo.GetGlobalAlpha();
    bool bCrInfluenced = drawInfo.IsInfluencedAlpha();

    bool bModDrawInfo = detail::TestBit(mFlag, 1) && mAlpha != 0xff;

    if (bModDrawInfo) {
        // mt = mutable, probably
        DrawInfo& mtDrawInfo = const_cast<DrawInfo&>(drawInfo);

        mtDrawInfo.SetGlobalAlpha(crGlobalAlpha * mAlpha * invAlpha);
        mtDrawInfo.SetInfluencedAlpha(true);
    }

    CalculateMtxChild(drawInfo);

    // restore changed values if applicable
    if (bModDrawInfo) {
        DrawInfo& mtDrawInfo = const_cast<DrawInfo&>(drawInfo);

        mtDrawInfo.SetGlobalAlpha(crGlobalAlpha);
        mtDrawInfo.SetInfluencedAlpha(bCrInfluenced);
    }
}

void Pane::CalculateMtxChild(const DrawInfo& drawInfo) {
    for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
        it->CalculateMtx(drawInfo);
    }
}

void Pane::Draw(const DrawInfo& drawInfo) {
    if (detail::TestBit(mFlag, 0)) {
        DrawSelf(drawInfo);

        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            it->Draw(drawInfo);
        }
    }
}

void Pane::DrawSelf(const DrawInfo& drawInfo) {
    if (mpParent && drawInfo.IsDebugDrawMode()) {
        LoadMtx(drawInfo);
        detail::DrawLine(GetVtxPos(), mSize, 0x00ff00ff); // green
    }
}

void Pane::Animate(u32 option) {
    AnimateSelf(option);

    if (detail::TestBit(mFlag, 0) || !(option & 1)) {
        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            it->Animate(option);
        }
    }
}

void Pane::AnimateSelf(u32 option) {
    for (AnimationLinkList::Iterator it = mAnimList.GetBeginIter(); it != mAnimList.GetEndIter(); it++) {
        if (it->IsEnable()) {
            AnimTransform* animTrans = it->GetAnimTransform();

            animTrans->Animate(it->GetIndex(), this);
        }
    }

    if ((detail::TestBit(mFlag, 0) || !(option & 1)) && mpMaterial) {
        mpMaterial->Animate();
    }
}

void Pane::BindAnimation(AnimTransform* pAnimTrans, bool bRecursive) {
    NW4HBMAssertPointerNonnull_Line(pAnimTrans, 596);
    pAnimTrans->Bind(this, bRecursive);
}

void Pane::UnbindAnimation(AnimTransform* pAnimTrans, bool bRecursive) {
    UnbindAnimationSelf(pAnimTrans);

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            it->UnbindAnimation(pAnimTrans, bRecursive);
        }
    }
}

void Pane::UnbindAllAnimation(bool bRecursive) { UnbindAnimation(nullptr, bRecursive); }

void Pane::UnbindAnimationSelf(AnimTransform* pAnimTrans) {
    if (mpMaterial) {
        mpMaterial->UnbindAnimation(pAnimTrans);
    }

    for (AnimationLinkList::Iterator it = mAnimList.GetBeginIter(); it != mAnimList.GetEndIter();) {
        AnimationLinkList::Iterator currIt = it++;

        if (pAnimTrans == NULL || currIt->GetAnimTransform() == pAnimTrans) {
            mAnimList.Erase(currIt);
            currIt->Reset();
        }
    }
}

void Pane::AddAnimationLink(AnimationLink* pAnimationLink) {
    NW4HBMAssertPointerNonnull_Line(pAnimationLink, 649);
    mAnimList.PushBack(pAnimationLink);
}

AnimationLink* Pane::FindAnimationLink(AnimTransform* pAnimTrans) {
    if (AnimationLink* ret = detail::FindAnimationLink(&mAnimList, pAnimTrans)) {
        return ret;
    }

    if (mpMaterial) {
        if (AnimationLink* ret = mpMaterial->FindAnimationLink(pAnimTrans)) {
            return ret;
        }
    }

    return nullptr;
}

void Pane::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable, bool bRecursive) {
    if (AnimationLink* pAnimLink = detail::FindAnimationLink(&mAnimList, pAnimTrans)) {
        pAnimLink->SetEnable(bEnable);
    }

    if (mpMaterial) {
        mpMaterial->SetAnimationEnable(pAnimTrans, bEnable);
    }

    if (bRecursive) {
        for (PaneList::Iterator it = mChildList.GetBeginIter(); it != mChildList.GetEndIter(); it++) {
            it->SetAnimationEnable(pAnimTrans, bEnable, bRecursive);
        }
    }
}

void Pane::LoadMtx(const DrawInfo& drawInfo) {
    math::MTX34 mtx;
    MtxPtr mtxPtr = nullptr;

    if (drawInfo.IsMultipleViewMtxOnDraw()) {
        math::MTX34Mult(&mtx, &drawInfo.GetViewMtx(), &mGlbMtx);

        if (drawInfo.IsYAxisUp()) {
            ReverseYAxis(&mtx);
        }

        mtxPtr = mtx;
    } else if (drawInfo.IsYAxisUp()) {
        math::MTX34Copy(&mtx, &mGlbMtx);
        ReverseYAxis(&mtx);
        mtxPtr = mtx;
    } else {
        mtxPtr = mGlbMtx;
    }

    GXLoadPosMtxImm(mtxPtr, 0);
    GXSetCurrentMtx(0);
}

math::VEC2 Pane::GetVtxPos() const {
    math::VEC2 basePt(0.0f, 0.0f);

    switch (mBasePosition % 3) {
        default:
        case 0:
            basePt.x = 0.0f;
            break;

        case 1:
            basePt.x = -mSize.width / 2.0f;
            break;

        case 2:
            basePt.x = -mSize.width;
            break;
    }

    switch (mBasePosition / 3) {
        default:
        case 0:
            basePt.y = 0.0f;
            break;

        case 1:
            basePt.y = -mSize.height / 2.0f;
            break;

        case 2:
            basePt.y = -mSize.height;
            break;
    }

    return basePt;
}

Material* Pane::GetMaterial() const { return mpMaterial; }

} // namespace lyt
} // namespace nw4hbm
