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

#pragma once

#include "Containers/HashMap.h"
#include "ComLogic.h"
#include "Script/LuaVM.h"

BE_NAMESPACE_BEGIN

class Collision;
class ScriptAsset;

class ComScript : public ComLogic {
public:
    OBJECT_PROTOTYPE(ComScript);

    ComScript();
    virtual ~ComScript();

    void                    InitScriptPropertyInfo(const Guid &scriptGuid);

    virtual void            GetPropertyInfoList(Array<PropertyInfo> &propertyInfoList) const override;

    virtual void            Deserialize(const Json::Value &in) override;

    virtual bool            AllowSameComponent() const override { return true; }

    virtual void            SetEnable(bool enable) override;

    virtual void            Purge(bool chainPurge = true) override;

                            /// Initializes this component. Called after deserialization.
    virtual void            Init() override;

                            /// Called once when game started before Start()
                            /// When game already started, called immediately after spawned
    virtual void            Awake() override;

                            /// Called once when game started.
                            /// When game already started, called immediately after spawned
    virtual void            Start() override;

                            /// Called on game world update, variable timestep.
    virtual void            Update() override;
                            /// Called on game world late-update, variable timestep.
    virtual void            LateUpdate() override;
                            /// Called on physics update, fixed timestep.
    virtual void            FixedUpdate(float timeStep) override;
                            /// Called on physics late-update, fixed timestep.
    virtual void            FixedLateUpdate(float timeStep) override;

    virtual void            OnPointerEnter();
    virtual void            OnPointerExit();
    virtual void            OnPointerOver();
    virtual void            OnPointerDown();
    virtual void            OnPointerUp();
    virtual void            OnPointerDrag();

    virtual void            OnCollisionEnter(const Collision &collision);
    virtual void            OnCollisionExit(const Collision &collision);
    virtual void            OnCollisionStay(const Collision &collision);

    virtual void            OnSensorEnter(const Entity *entity);
    virtual void            OnSensorExit(const Entity *entity);
    virtual void            OnSensorStay(const Entity *entity);

    virtual void            OnParticleCollision(const Entity *entity);
    
    void                    OnApplicationTerminate();
    void                    OnApplicationPause(bool pause);

    Guid                    GetScriptGuid() const;
    void                    SetScriptGuid(const Guid &guid);

    const char *            GetSandboxName() const { return sandboxName.c_str(); }

    template <typename... Args>
    void                    CallFunc(const char *funcName, Args&&... args);

protected:
    bool                    LoadScriptWithSandbox(const char *filename, const char *sandboxName);
    void                    SetScriptProperties();
    void                    CacheFunction(const char *funcname);
    void                    UpdateFunctionMap();

    void                    ChangeScript(const Guid &scriptGuid);
    void                    ScriptReloaded();

    ScriptAsset *           scriptAsset;
    Str                     sandboxName;
    LuaCpp::Selector        sandbox;
    Array<PropertyInfo>     fieldInfos;
    HashMap<Str, Variant>   fieldValues;
    HashMap<Str, LuaCpp::Selector> functions;
};

template <typename... Args>
BE_INLINE void ComScript::CallFunc(const char *funcName, Args&&... args) {
    auto func = sandbox[funcName];
    if (func.IsFunction()) {
        func(std::forward<Args>(args)...);
    }
}

BE_NAMESPACE_END
