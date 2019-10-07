// Copyright(c) 2017 POLYGONTEK
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Precompiled.h"
#include "Components/ComRectTransform.h"
#include "Game/Entity.h"
#include "Game/GameWorld.h"
#include "Render/Render.h"

BE_NAMESPACE_BEGIN

OBJECT_DECLARATION("Rect Transform", ComRectTransform, ComTransform)
BEGIN_EVENTS(ComRectTransform)
END_EVENTS

void ComRectTransform::RegisterProperties() {
    REGISTER_MIXED_ACCESSOR_PROPERTY("anchorMin", "Anchor Min", Vec2, GetAnchorMin, SetAnchorMin, Vec2(0.5, 0.5),
        "", PropertyInfo::Flag::Editor);
    REGISTER_MIXED_ACCESSOR_PROPERTY("anchorMax", "Anchor Max", Vec2, GetAnchorMax, SetAnchorMax, Vec2(0.5, 0.5),
        "", PropertyInfo::Flag::Editor);
    REGISTER_MIXED_ACCESSOR_PROPERTY("anchoredPosition", "Anchored Position", Vec2, GetAnchoredPosition, SetAnchoredPosition, Vec2(0, 0),
        "", PropertyInfo::Flag::Editor);
    REGISTER_MIXED_ACCESSOR_PROPERTY("sizeDelta", "Size Delta", Vec2, GetSizeDelta, SetSizeDelta, Vec2(100, 100),
        "", PropertyInfo::Flag::Editor);
    REGISTER_MIXED_ACCESSOR_PROPERTY("pivot", "Pivot", Vec2, GetPivot, SetPivot, Vec2(0.5, 0.5),
        "", PropertyInfo::Flag::Editor);
}

ComRectTransform::ComRectTransform() {
    cachedRect = RectF::empty;
    cachedRectInvalidated = false;
}

ComRectTransform::~ComRectTransform() {
}

void ComRectTransform::Init() {
    Component::Init();

    UpdateWorldMatrix();

    // Mark as initialized
    SetInitialized(true);
}

Vec2 ComRectTransform::GetAnchorMin() const {
    return anchorMin;
}

void ComRectTransform::SetAnchorMin(const Vec2 &anchorMin) {
    this->anchorMin = anchorMin;

    if (IsInitialized()) {
        InvalidateCachedRect();
    }
}

Vec2 ComRectTransform::GetAnchorMax() const {
    return anchorMax;
}

void ComRectTransform::SetAnchorMax(const Vec2 &anchorMax) {
    this->anchorMax = anchorMax;

    if (IsInitialized()) {
        InvalidateCachedRect();
    }
}

Vec2 ComRectTransform::GetAnchoredPosition() const {
    return anchoredPosition;
}

void ComRectTransform::SetAnchoredPosition(const Vec2 &anchoredPosition) {
    this->anchoredPosition = anchoredPosition;

    if (IsInitialized()) {
        InvalidateCachedRect();
    }
}

Vec2 ComRectTransform::GetSizeDelta() const {
    return sizeDelta;
}

void ComRectTransform::SetSizeDelta(const Vec2 &sizeDelta) {
    this->sizeDelta = sizeDelta;

    if (IsInitialized()) {
        InvalidateCachedRect();
    }
}

Vec2 ComRectTransform::GetPivot() const {
    return pivot;
}

void ComRectTransform::SetPivot(const Vec2 &pivot) {
    this->pivot = pivot;

    if (IsInitialized()) {
        InvalidateCachedRect();
    }
}

RectF ComRectTransform::GetRectInLocalSpace() {
    if (cachedRectInvalidated) {
        UpdateOriginAndRect();
    }
    return cachedRect;
}

void ComRectTransform::GetLocalCorners(Vec3(&localCorners)[4]) {
    RectF rect = GetRectInLocalSpace();
    
    float x1 = rect.x;
    float y1 = rect.y;
    float x2 = rect.X2();
    float y2 = rect.Y2();

    localCorners[0] = Vec3(x1, y1, 0);
    localCorners[1] = Vec3(x2, y1, 0);
    localCorners[2] = Vec3(x2, y2, 0);
    localCorners[3] = Vec3(x1, y2, 0);
}

void ComRectTransform::GetWorldCorners(Vec3(&worldCorners)[4]) {
    Vec3 localCorners[4];
    GetLocalCorners(localCorners);

    Mat3x4 worldMatrix = GetMatrix();

    for (int i = 0; i < 4; i++) {
        worldCorners[i] = worldMatrix.Transform(localCorners[i]);
    }
}

void ComRectTransform::GetWorldAnchorCorners(Vec3(&worldAnchorCorners)[4]) {
    for (int i = 0; i < COUNT_OF(worldAnchorCorners); i++) {
        worldAnchorCorners[i] = Vec3::zero;
    }

    ComTransform *parentTransform = GetParent();
    if (parentTransform) {
        ComRectTransform *parentRectTransform = parentTransform->Cast<ComRectTransform>();
        if (parentRectTransform) {
            Vec3 parentWorldCorners[4];
            parentRectTransform->GetWorldCorners(parentWorldCorners);

            Vec2 worldAnchorMin, worldAnchorMax;
            worldAnchorMin.x = parentWorldCorners[0].x + (parentWorldCorners[2].x - parentWorldCorners[0].x) * anchorMin.x;
            worldAnchorMin.y = parentWorldCorners[0].y + (parentWorldCorners[2].y - parentWorldCorners[0].y) * anchorMin.y;
            worldAnchorMax.x = parentWorldCorners[0].x + (parentWorldCorners[2].x - parentWorldCorners[0].x) * anchorMax.x;
            worldAnchorMax.y = parentWorldCorners[0].y + (parentWorldCorners[2].y - parentWorldCorners[0].y) * anchorMax.y;

            worldAnchorCorners[0] = Vec3(worldAnchorMin.x, worldAnchorMin.y, 0);
            worldAnchorCorners[1] = Vec3(worldAnchorMax.x, worldAnchorMin.y, 0);
            worldAnchorCorners[2] = Vec3(worldAnchorMax.x, worldAnchorMax.y, 0);
            worldAnchorCorners[3] = Vec3(worldAnchorMin.x, worldAnchorMax.y, 0);
        }
    }
}

void ComRectTransform::UpdateOriginAndRect() {
    Vec3 oldLocalOrigin = GetLocalOrigin();
    Vec3 newLocalOrigin = ComputeLocalOrigin3D();

    if (oldLocalOrigin != newLocalOrigin) {
        SetLocalOrigin(newLocalOrigin);
    }

    cachedRect = ComputeRectInLocalSpace();

    cachedRectInvalidated = false;
}

void ComRectTransform::InvalidateCachedRect() {
    if (cachedRectInvalidated) {
        return;
    }
    cachedRectInvalidated = true;

    for (Entity *childEntity = GetEntity()->GetNode().GetFirstChild(); childEntity; childEntity = childEntity->GetNode().GetNextSibling()) {
        ComRectTransform *rectTransform = childEntity->GetComponent(0)->Cast<ComRectTransform>();
        if (rectTransform) {
            rectTransform->InvalidateCachedRect();
        }
    }
}

RectF ComRectTransform::ComputeRectInParentSpace() const {
    RectF parentRect = RectF(0, 0, 0, 0);

    ComTransform *parentTransform = GetParent();
    if (parentTransform) {
        ComRectTransform *parentRectTransform = parentTransform->Cast<ComRectTransform>();
        if (parentRectTransform) {
            parentRect = parentRectTransform->GetRectInLocalSpace();
        }
    }

    PointF anchoredMin = PointF(
        parentRect.x + (parentRect.w * anchorMin.x), 
        parentRect.y + (parentRect.h * anchorMin.y));

    PointF anchoredMax = PointF(
        parentRect.x + (parentRect.w * anchorMax.x), 
        parentRect.y + (parentRect.h * anchorMax.y));

    RectF rect;
    rect.x = anchoredMin.x + anchoredPosition.x - (sizeDelta.x * pivot.x);
    rect.y = anchoredMin.y + anchoredPosition.y - (sizeDelta.y * pivot.y);
    rect.w = anchoredMax.x - anchoredMin.x + sizeDelta.x;
    rect.h = anchoredMax.y - anchoredMin.y + sizeDelta.y;

    return rect;
}

RectF ComRectTransform::ComputeRectInLocalSpace() const {
    RectF rect = ComputeRectInParentSpace();
    
    // Translate rect in local pivot.
    rect.x = -rect.w * pivot.x;
    rect.y = -rect.h * pivot.y;

    return rect;
}

// Returns local origin in parent space in 2D.
Vec2 ComRectTransform::ComputeLocalOrigin2D() const {
    RectF rect = ComputeRectInParentSpace();

    return Vec2(
        rect.x + rect.w * pivot.x,
        rect.y + rect.h * pivot.y);
}

// Returns local origin in parent space in 3D.
Vec3 ComRectTransform::ComputeLocalOrigin3D() const {
    RectF rect = ComputeRectInParentSpace();
    Vec3 localOrigin = GetLocalOrigin();

    return Vec3(
        rect.x + rect.w * pivot.x,
        rect.y + rect.h * pivot.y,
        localOrigin.z);
}

#if WITH_EDITOR
void ComRectTransform::DrawGizmos(const RenderCamera *camera, bool selected, bool selectedByParent) {
    if (!selected) {
        return;
    }

    RenderWorld *renderWorld = GetGameWorld()->GetRenderWorld();

    // Draw rectangle
    Vec3 worldCorners[4];
    GetWorldCorners(worldCorners);

    renderWorld->SetDebugColor(Color4(1.0f, 1.0f, 1.0f, 0.4f), Color4::zero);

    renderWorld->DebugLine(worldCorners[0], worldCorners[1]);
    renderWorld->DebugLine(worldCorners[1], worldCorners[2]);
    renderWorld->DebugLine(worldCorners[2], worldCorners[3]);
    renderWorld->DebugLine(worldCorners[3], worldCorners[0]);

    ComTransform *parentTransform = GetParent();
    if (parentTransform) {
        ComRectTransform *parentRectTransform = parentTransform->Cast<ComRectTransform>();
        if (parentRectTransform) {
            // Draw parent rectangle
            Vec3 parentWorldCorners[4];
            parentRectTransform->GetWorldCorners(parentWorldCorners);

            renderWorld->SetDebugColor(Color4(1.0f, 1.0f, 1.0f, 1.0f), Color4::zero);

            renderWorld->DebugLine(parentWorldCorners[0], parentWorldCorners[1]);
            renderWorld->DebugLine(parentWorldCorners[1], parentWorldCorners[2]);
            renderWorld->DebugLine(parentWorldCorners[2], parentWorldCorners[3]);
            renderWorld->DebugLine(parentWorldCorners[3], parentWorldCorners[0]);

            // Draw anchors
            Vec3 worldAnchorCorners[4];
            GetWorldAnchorCorners(worldAnchorCorners);

            float viewScale = camera->CalcViewScale(worldAnchorCorners[0]);
            renderWorld->DebugTriangle(worldAnchorCorners[0], worldAnchorCorners[0] + Vec3(-10, -5, 0) * viewScale, worldAnchorCorners[0] + Vec3(-5, -10, 0) * viewScale);

            viewScale = camera->CalcViewScale(worldAnchorCorners[1]);
            renderWorld->DebugTriangle(worldAnchorCorners[1], worldAnchorCorners[1] + Vec3(+10, -5, 0) * viewScale, worldAnchorCorners[1] + Vec3(+5, -10, 0) * viewScale);

            viewScale = camera->CalcViewScale(worldAnchorCorners[2]);
            renderWorld->DebugTriangle(worldAnchorCorners[2], worldAnchorCorners[2] + Vec3(+10, +5, 0) * viewScale, worldAnchorCorners[2] + Vec3(+5, +10, 0) * viewScale);

            viewScale = camera->CalcViewScale(worldAnchorCorners[3]);
            renderWorld->DebugTriangle(worldAnchorCorners[3], worldAnchorCorners[3] + Vec3(-10, +5, 0) * viewScale, worldAnchorCorners[3] + Vec3(-5, +10, 0) * viewScale);
        }
    }
    }
#endif

const AABB ComRectTransform::GetAABB() {
    Vec3 localCorners[4];
    GetLocalCorners(localCorners);

    AABB aabb;
    aabb.Clear();
    aabb.AddPoint(localCorners[0]);
    aabb.AddPoint(localCorners[1]);
    aabb.AddPoint(localCorners[2]);
    aabb.AddPoint(localCorners[3]);

    return aabb;
}

BE_NAMESPACE_END