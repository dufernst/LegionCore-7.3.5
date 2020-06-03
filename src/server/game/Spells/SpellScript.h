/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SPELL_SCRIPT_H
#define __SPELL_SCRIPT_H

#include "Util.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "Spell.h"
#include <stack>
#include <safe_ptr.h>

class Unit;
class SpellInfo;
class SpellScript;
class Spell;
class Aura;
class AuraEffect;
struct SpellModifier;
class Creature;
class GameObject;
class DynamicObject;
class Player;
class Item;
class WorldLocation;
class WorldObject;

#define SPELL_EFFECT_ANY (uint16)-1
#define SPELL_AURA_ANY (uint16)-1

enum SpellScriptState
{
    SPELL_SCRIPT_STATE_NONE = 0,
    SPELL_SCRIPT_STATE_REGISTRATION,
    SPELL_SCRIPT_STATE_LOADING,
    SPELL_SCRIPT_STATE_UNLOADING,
};
constexpr uint8 SPELL_SCRIPT_STATE_END = SPELL_SCRIPT_STATE_UNLOADING + 1;

// helper class from which SpellScript and SpellAura derive, use these classes instead
class _SpellScript
{
    // internal use classes & functions
    // DO NOT OVERRIDE THESE IN SCRIPTS
    protected:
        virtual bool _Validate(SpellInfo const* entry);

    public:
        _SpellScript();
        virtual ~_SpellScript() {}
        virtual void _Register();
        virtual void _Unload();
        virtual void _Init(std::string const* scriptname, uint32 spellId);
        std::string const* _GetScriptName() const;

    protected:
        class EffectHook
        {
            public:
                EffectHook(uint8 _effIndex);
                uint32 GetAffectedEffectsMask(SpellInfo const* spellEntry);
                bool IsEffectAffected(SpellInfo const* spellEntry, uint8 effIndex);
                virtual bool CheckEffect(SpellInfo const* spellEntry, uint8 effIndex) = 0;
                std::string EffIndexToString();
            protected:
                uint8 effIndex;
        };

        class EffectNameCheck
        {
            public:
                EffectNameCheck(uint16 _effName) {effName = _effName;};
                bool Check(SpellInfo const* spellEntry, uint8 effIndex);
                std::string ToString();
            private:
                uint16 effName;
        };

        class EffectAuraNameCheck
        {
            public:
                EffectAuraNameCheck(uint16 _effAurName) { effAurName = _effAurName; }
                bool Check(SpellInfo const* spellEntry, uint8 effIndex);
                std::string ToString();
            private:
                uint16 effAurName;
        };

        uint8 m_currentScriptState;
        std::string const* m_scriptName;
        uint32 m_scriptSpellId;
    public:
        //
        // SpellScript/AuraScript interface base
        // these functions are safe to override, see notes below for usage instructions
        //
        // Function in which handler functions are registered, must be implemented in script
        virtual void Register() = 0;
        // Function called on server startup, if returns false script won't be used in core
        // use for: dbc/template data presence/correctness checks
        virtual bool Validate(SpellInfo const* /*spellEntry*/) { return true; }
        // Function called when script is created, if returns false script will be unloaded afterwards
        // use for: initializing local script variables (DO NOT USE CONSTRUCTOR FOR THIS PURPOSE!)
        virtual bool Load() { return true; }
        // Function called when script is destroyed
        // use for: deallocating memory allocated by script
        virtual void Unload() {}

        static bool ValidateSpellInfo(std::vector<uint32> spellIds);
};

// SpellScript interface - enum used for runtime checks of script function calls
enum SpellScriptHookType
{
    SPELL_SCRIPT_HOOK_EFFECT_LAUNCH = SPELL_SCRIPT_STATE_END,
    SPELL_SCRIPT_HOOK_EFFECT_LAUNCH_TARGET,
    SPELL_SCRIPT_HOOK_EFFECT_HIT,
    SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET,
    SPELL_SCRIPT_HOOK_BEFORE_HIT,
    SPELL_SCRIPT_HOOK_EFFECT_SUCCESSFUL_DISPEL,
    SPELL_SCRIPT_HOOK_HIT,
    SPELL_SCRIPT_HOOK_AFTER_HIT,
    SPELL_SCRIPT_HOOK_OBJECT_AREA_TARGET_SELECT,
    SPELL_SCRIPT_HOOK_OBJECT_JUMP_TARGET,
    SPELL_SCRIPT_HOOK_OBJECT_TARGET_SELECT,
    SPELL_SCRIPT_HOOK_CHECK_CAST,
    SPELL_SCRIPT_HOOK_TAKE_POWER,
    SPELL_SCRIPT_HOOK_BEFORE_CAST,
    SPELL_SCRIPT_HOOK_ON_CAST,
    SPELL_SCRIPT_HOOK_AFTER_CAST,
    SPELL_SCRIPT_HOOK_BEFORE_START_CAST,
    SPELL_SCRIPT_HOOK_ON_FINISH_CAST,
    SPELL_SCRIPT_HOOK_CALC_EFFECT_MASK
};

const uint8 HOOK_SPELL_HIT_START = SPELL_SCRIPT_HOOK_EFFECT_HIT;
constexpr uint8 HOOK_SPELL_HIT_END = SPELL_SCRIPT_HOOK_AFTER_HIT + 1;

class SpellScript : public _SpellScript
{
    // internal use classes & functions
    // DO NOT OVERRIDE THESE IN SCRIPTS
    public:
        #define SPELLSCRIPT_FUNCTION_TYPE_DEFINES(CLASSNAME) \
            typedef SpellCastResult(CLASSNAME::*SpellCheckCastFnType)(); \
            typedef void(CLASSNAME::*TakePowerFnType)(Powers , int32 &); \
            typedef void(CLASSNAME::*SpellEffectFnType)(SpellEffIndex); \
            typedef void(CLASSNAME::*SpellHitFnType)(); \
            typedef void(CLASSNAME::*SpellCastFnType)(); \
            typedef void(CLASSNAME::*SpellObjectAreaTargetSelectFnType)(std::list<WorldObject*>&); \
            typedef void(CLASSNAME::*SpellObjectTargetSelectFnType)(WorldObject*&); \
            typedef void(CLASSNAME::*SpellObjectJumpTargetFnType)(int32 &); \
            typedef void(CLASSNAME::*CalcEffectMaskFnType)(uint32 &);

        SPELLSCRIPT_FUNCTION_TYPE_DEFINES(SpellScript)

        class CastHandler
        {
            public:
                CastHandler(SpellCastFnType _pCastHandlerScript);
                void Call(SpellScript* spellScript);
            private:
                SpellCastFnType pCastHandlerScript;
        };

        class CheckCastHandler
        {
            public:
                CheckCastHandler(SpellCheckCastFnType checkCastHandlerScript);
                SpellCastResult Call(SpellScript* spellScript);
            private:
                SpellCheckCastFnType _checkCastHandlerScript;
        };

        class TakePowerHandler
        {
            public:
                TakePowerHandler(TakePowerFnType TakePowerHandlerScript);
                void Call(SpellScript* spellScript, Powers power, int32 &amount);
            private:
                TakePowerFnType _TakePowerHandlerScript;
        };

        class CalcEffectMaskHandler
        {
            public:
                CalcEffectMaskHandler(CalcEffectMaskFnType CalcEffectMaskHandlerScript);
                void Call(SpellScript* spellScript, uint32 &effMask);
            private:
                CalcEffectMaskFnType _CalcEffectMaskHandlerScript;
        };

        class EffectHandler : public  _SpellScript::EffectNameCheck, public _SpellScript::EffectHook
        {
            public:
                EffectHandler(SpellEffectFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                std::string ToString();
                bool CheckEffect(SpellInfo const* spellEntry, uint8 effIndex) override;
                void Call(SpellScript* spellScript, SpellEffIndex effIndex);
            private:
                SpellEffectFnType pEffectHandlerScript;
        };

        class HitHandler
        {
            public:
                HitHandler(SpellHitFnType _pHitHandlerScript);
                void Call(SpellScript* spellScript);
            private:
                SpellHitFnType pHitHandlerScript;
        };

        class TargetHook : public _SpellScript::EffectHook
        {
            public:
                TargetHook(uint8 _effectIndex, uint16 _targetType, bool _area);
                bool CheckEffect(SpellInfo const* spellEntry, uint8 effIndex) override;
                std::string ToString();
            protected:
                uint16 targetType;
                bool area;
        };

        class ObjectAreaTargetSelectHandler : public TargetHook
        {
            public:
                ObjectAreaTargetSelectHandler(SpellObjectAreaTargetSelectFnType _pObjectAreaTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType);
                void Call(SpellScript* spellScript, std::list<WorldObject*>& targets);
            private:
                SpellObjectAreaTargetSelectFnType pObjectAreaTargetSelectHandlerScript;
        };

        class ObjectTargetSelectHandler : public TargetHook
        {
            public:
                ObjectTargetSelectHandler(SpellObjectTargetSelectFnType _pObjectTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType);
                void Call(SpellScript* spellScript, WorldObject*& targets);
            private:
                SpellObjectTargetSelectFnType pObjectTargetSelectHandlerScript;
        };

        class ObjectJumpTargetHandler : public TargetHook
        {
            public:
            ObjectJumpTargetHandler(SpellObjectJumpTargetFnType _pObjectJumpTargetHandlerScript, uint8 _effIndex, uint16 _targetType);
            void Call(SpellScript* spellScript, int32 & AddJumpTargets);
            private:
            SpellObjectJumpTargetFnType pObjectJumpTargetHandlerScript;
        };

        #define SPELLSCRIPT_FUNCTION_CAST_DEFINES(CLASSNAME) \
        class CastHandlerFunction : public SpellScript::CastHandler { public: CastHandlerFunction(SpellCastFnType _pCastHandlerScript) : SpellScript::CastHandler((SpellScript::SpellCastFnType)_pCastHandlerScript) {} }; \
        class CheckCastHandlerFunction : public SpellScript::CheckCastHandler { public: CheckCastHandlerFunction(SpellCheckCastFnType _checkCastHandlerScript) : SpellScript::CheckCastHandler((SpellScript::SpellCheckCastFnType)_checkCastHandlerScript) {} }; \
        class TakePowerHandlerFunction : public SpellScript::TakePowerHandler { public: TakePowerHandlerFunction(TakePowerFnType _TakePowerHandlerScript) : SpellScript::TakePowerHandler((SpellScript::TakePowerFnType)_TakePowerHandlerScript) {} }; \
        class EffectHandlerFunction : public SpellScript::EffectHandler { public: EffectHandlerFunction(SpellEffectFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : SpellScript::EffectHandler((SpellScript::SpellEffectFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class HitHandlerFunction : public SpellScript::HitHandler { public: HitHandlerFunction(SpellHitFnType _pHitHandlerScript) : SpellScript::HitHandler((SpellScript::SpellHitFnType)_pHitHandlerScript) {} }; \
        class ObjectAreaTargetSelectHandlerFunction : public SpellScript::ObjectAreaTargetSelectHandler { public: ObjectAreaTargetSelectHandlerFunction(SpellObjectAreaTargetSelectFnType _pObjectAreaTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType) : SpellScript::ObjectAreaTargetSelectHandler((SpellScript::SpellObjectAreaTargetSelectFnType)_pObjectAreaTargetSelectHandlerScript, _effIndex, _targetType) {} }; \
        class ObjectTargetSelectHandlerFunction : public SpellScript::ObjectTargetSelectHandler { public: ObjectTargetSelectHandlerFunction(SpellObjectTargetSelectFnType _pObjectTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType) : SpellScript::ObjectTargetSelectHandler((SpellScript::SpellObjectTargetSelectFnType)_pObjectTargetSelectHandlerScript, _effIndex, _targetType) {} }; \
        class ObjectJumpTargetHandlerFunction : public SpellScript::ObjectJumpTargetHandler { public: ObjectJumpTargetHandlerFunction(SpellObjectJumpTargetFnType _pObjectJumpTargetHandlerScript, uint8 _effIndex, uint16 _targetType) : SpellScript::ObjectJumpTargetHandler((SpellScript::SpellObjectJumpTargetFnType)_pObjectJumpTargetHandlerScript, _effIndex, _targetType) {} }; \
        class CalcEffectMaskHandlerFunction : public SpellScript::CalcEffectMaskHandler { public: CalcEffectMaskHandlerFunction(CalcEffectMaskFnType _CalcEffectMaskHandlerScript) : SpellScript::CalcEffectMaskHandler((SpellScript::CalcEffectMaskFnType)_CalcEffectMaskHandlerScript) {} };

        #define PrepareSpellScript(CLASSNAME) SPELLSCRIPT_FUNCTION_TYPE_DEFINES(CLASSNAME) SPELLSCRIPT_FUNCTION_CAST_DEFINES(CLASSNAME)
    public:
        bool _Validate(SpellInfo const* entry) override;
        bool _Load(Spell* spell);
        void _InitHit();
        bool _IsEffectPrevented(SpellEffIndex effIndex) { return (m_hitPreventEffectMask & (1 << effIndex)) != 0; }
        bool _IsDefaultEffectPrevented(SpellEffIndex effIndex) { return (m_hitPreventDefaultEffectMask & (1 << effIndex)) != 0; }
        void _PrepareScriptCall(SpellScriptHookType hookType);
        void _FinishScriptCall();
        bool IsInCheckCastHook() const;
        bool IsInTargetHook() const;
        bool IsInHitPhase() const;
        bool IsInEffectHook() const;
    private:
        Spell* m_spell = nullptr;
        uint8 m_hitPreventEffectMask = 0;
        uint8 m_hitPreventDefaultEffectMask = 0;
    public:
        //
        // SpellScript interface
        // hooks to which you can attach your functions
        //
        // example: BeforeCast += SpellCastFn(class::function);
        HookList<CastHandler> BeforeCast;
        // example: OnCast += SpellCastFn(class::function);
        HookList<CastHandler> OnCast;
        // example: AfterCast += SpellCastFn(class::function);
        HookList<CastHandler> AfterCast;
        // example: BeforeStartCast += SpellCastFn(class::function);
        HookList<CastHandler> BeforeStartCast;
        // example: OnFinishCast += SpellCastFn(class::function);
        HookList<CastHandler> OnFinishCast;
        #define SpellCastFn(F) CastHandlerFunction(&F)

        // example: OnCheckCast += SpellCheckCastFn();
        // where function is SpellCastResult function()
        HookList<CheckCastHandler> OnCheckCast;
        #define SpellCheckCastFn(F) CheckCastHandlerFunction(&F)

        // example: OnTakePower += TakePowertFn();
        // where function is TakePower function()
        HookList<TakePowerHandler> OnTakePower;
        #define TakePowertFn(F) TakePowerHandlerFunction(&F)

        // example: DoCalcEffMask += CalcEffectMaskFn();
        HookList<CalcEffectMaskHandler> DoCalcEffMask;
        #define CalcEffectMaskFn(F) CalcEffectMaskHandlerFunction(&F)

        // example: OnEffect**** += SpellEffectFn(class::function, EffectIndexSpecifier, EffectNameSpecifier);
        // where function is void function(SpellEffIndex effIndex)
        HookList<EffectHandler> OnEffectLaunch;
        HookList<EffectHandler> OnEffectLaunchTarget;
        HookList<EffectHandler> OnEffectHit;
        HookList<EffectHandler> OnEffectHitTarget;
        HookList<EffectHandler> OnEffectSuccessfulDispel;
        #define SpellEffectFn(F, I, N) EffectHandlerFunction(&F, I, N)

        // example: BeforeHit += SpellHitFn(class::function);
        HookList<HitHandler> BeforeHit;
        // example: OnHit += SpellHitFn(class::function);
        HookList<HitHandler> OnHit;
        // example: AfterHit += SpellHitFn(class::function);
        HookList<HitHandler> AfterHit;
        // where function is: void function()
        #define SpellHitFn(F) HitHandlerFunction(&F)

        // example: OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(class::function, EffectIndexSpecifier, TargetsNameSpecifier);
        // where function is void function(std::list<WorldObject*>& targets)
        HookList<ObjectAreaTargetSelectHandler> OnObjectAreaTargetSelect;
        #define SpellObjectAreaTargetSelectFn(F, I, N) ObjectAreaTargetSelectHandlerFunction(&F, I, N)

        // example: OnObjectJumpTarget += SpellObjectJumpTargetFn(class::function, EffectIndexSpecifier, TargetsNameSpecifier);
        // where function is void function(int32 AddJumpTarget)
        HookList<ObjectJumpTargetHandler> OnObjectJumpTarget;
        #define SpellObjectJumpTargetFn(F, I, N) ObjectJumpTargetHandlerFunction(&F, I, N)

        // example: OnObjectTargetSelect += SpellObjectTargetSelectFn(class::function, EffectIndexSpecifier, TargetsNameSpecifier);
        // where function is void function(WorldObject*& target)
        HookList<ObjectTargetSelectHandler> OnObjectTargetSelect;
        #define SpellObjectTargetSelectFn(F, I, N) ObjectTargetSelectHandlerFunction(&F, I, N)

        // hooks are executed in following order, at specified event of spell:
        // 1. BeforeCast - executed when spell preparation is finished (when cast bar becomes full) before cast is handled
        // 2. OnCheckCast - allows to override result of CheckCast function
        // 3a. OnObjectAreaTargetSelect - executed just before adding selected targets to final target list (for area targets)
        // 3b. OnObjectTargetSelect - executed just before adding selected target to final target list (for single unit targets)
        // 4. OnCast - executed just before spell is launched (creates missile) or executed
        // 5. AfterCast - executed after spell missile is launched and immediate spell actions are done
        // 6. OnEffectLaunch - executed just before specified effect handler call - when spell missile is launched
        // 7. OnEffectLaunchTarget - executed just before specified effect handler call - when spell missile is launched - called for each target from spell target map
        // 8. OnEffectHit - executed just before specified effect handler call - when spell missile hits dest
        // 9. BeforeHit - executed just before spell hits a target - called for each target from spell target map
        // 10. OnEffectHitTarget - executed just before specified effect handler call - called for each target from spell target map
        // 11. OnHit - executed just before spell deals damage and procs auras - when spell hits target - called for each target from spell target map
        // 12. AfterHit - executed just after spell finishes all it's jobs for target - called for each target from spell target map
        // 13. BeforeStartCast - executed when spell preparation is start (when cast bar becomes start) before cast is handled

        //
        // methods allowing interaction with Spell object
        //
        // methods useable during all spell handling phases
        Unit* GetCaster();
        Unit* GetOriginalCaster();
        SpellInfo const* GetSpellInfo();
        SpellValue const* GetSpellValue();
        uint32 GetId() const;

        // methods useable after spell is prepared
        // accessors to the explicit targets of the spell
        // explicit target - target selected by caster (player, game client, or script - DoCast(explicitTarget, ...), required for spell to be cast
        // examples:
        // -shadowstep - explicit target is the unit you want to go behind of
        // -chain heal - explicit target is the unit to be healed first
        // -holy nova/arcane explosion - explicit target = NULL because target you are selecting doesn't affect how spell targets are selected
        // you can determine if spell requires explicit targets by dbc columns:
        // - Targets - mask of explicit target types
        // - ImplicitTargetXX set to TARGET_XXX_TARGET_YYY, _TARGET_ here means that explicit target is used by the effect, so spell needs one too

        // returns: WorldLocation which was selected as a spell destination or NULL
        WorldLocation const* GetExplTargetDest();

        void SetExplTargetDest(WorldLocation& loc);

        // returns: WorldObject which was selected as an explicit spell target or NULL if there's no target
        WorldObject* GetExplTargetWorldObject();

        // returns: Unit which was selected as an explicit spell target or NULL if there's no target
        Unit* GetExplTargetUnit();

        // returns: GameObject which was selected as an explicit spell target or NULL if there's no target
        GameObject* GetExplTargetGObj();

        // returns: Item which was selected as an explicit spell target or NULL if there's no target
        Item* GetExplTargetItem();

        // methods useable only during spell hit on target, or during spell launch on target:
        // returns: target of current effect if it was Unit otherwise NULL
        Unit* GetHitUnit();
        Unit* GetOriginalTarget();
        // returns: target of current effect if it was Creature otherwise NULL
        Creature* GetHitCreature();
        // returns: target of current effect if it was Player otherwise NULL
        Player* GetHitPlayer();
        // returns: target of current effect if it was Item otherwise NULL
        Item* GetHitItem();
        // returns: target of current effect if it was GameObject otherwise NULL
        GameObject* GetHitGObj();
        // returns: destination of current effect
        WorldLocation* GetHitDest();
        // setter/getter for for damage done by spell to target of spell hit
        // returns damage calculated before hit, and real dmg done after hit
        int32 GetHitAbsorb();
        int32 GetHitResist();
        int32 GetHitBlocked();
        int32 GetHitDamage();
        int32 GetFinalHitDamage();
        int32 GetAbsorbDamage();
        void SetHitDamage(int32 damage);
        void PreventHitDamage() { SetHitDamage(0); }
        // setter/getter for for heal done by spell to target of spell hit
        // returns healing calculated before hit, and real dmg done after hit
        int32 GetHitHeal();
        void SetHitHeal(int32 heal);
        void PreventHitHeal() { SetHitHeal(0); }
        Spell* GetSpell() { return m_spell; }
        // returns current spell hit target aura
        Aura* GetHitAura();
        // prevents applying aura on current spell hit target
        void PreventHitAura();

        // prevents effect execution on current spell hit target
        // including other effect/hit scripts
        // will not work on aura/damage/heal
        // will not work if effects were already handled
        void PreventHitEffect(SpellEffIndex effIndex);

        // prevents default effect execution on current spell hit target
        // will not work on aura/damage/heal effects
        // will not work if effects were already handled
        void PreventHitDefaultEffect(SpellEffIndex effIndex);

        // method avalible only in EffectHandler method
        int32 GetEffectValue();
        void SetEffectValue(int32 val);

        // returns: cast item if present.
        Item* GetCastItem();

        // Creates item. Calls Spell::DoCreateItem method.
        void CreateItem(uint32 effIndex, uint32 itemId);

        // Returns SpellInfo from the spell that triggered the current one
        SpellInfo const* GetTriggeringSpell();
        AuraEffect const* GetTriggeredAuraEff();

        // finishes spellcast prematurely with selected error message
        void FinishCast(SpellCastResult result);

        void SetCustomCastResultMessage(SpellCustomErrors result);

        // Call special function from scripts
        virtual uint32 CallSpecialFunction(uint32 Num = 0) { return 0; }
};

// AuraScript interface - enum used for runtime checks of script function calls
enum AuraScriptHookType
{
    AURA_SCRIPT_HOOK_EFFECT_APPLY = SPELL_SCRIPT_STATE_END,
    AURA_SCRIPT_HOOK_EFFECT_AFTER_APPLY,
    AURA_SCRIPT_HOOK_EFFECT_REMOVE,
    AURA_SCRIPT_HOOK_EFFECT_AFTER_REMOVE,
    AURA_SCRIPT_HOOK_EFFECT_PERIODIC,
    AURA_SCRIPT_HOOK_ON_UPDATE,
    AURA_SCRIPT_HOOK_EFFECT_UPDATE,
    AURA_SCRIPT_HOOK_EFFECT_UPDATE_PERIODIC,
    AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT,
    AURA_SCRIPT_HOOK_EFFECT_CALC_PERIODIC,
    AURA_SCRIPT_HOOK_EFFECT_CALC_SPELLMOD,
    AURA_SCRIPT_HOOK_EFFECT_ABSORB,
    AURA_SCRIPT_HOOK_EFFECT_AFTER_ABSORB,
    AURA_SCRIPT_HOOK_EFFECT_MANASHIELD,
    AURA_SCRIPT_HOOK_EFFECT_AFTER_MANASHIELD,
    AURA_SCRIPT_HOOK_EFFECT_SPLIT_DAMAGE,
    AURA_SCRIPT_HOOK_CALC_MAX_DURATION,
    AURA_SCRIPT_HOOK_CHECK_AREA_TARGET,
    AURA_SCRIPT_HOOK_DISPEL,
    AURA_SCRIPT_HOOK_AFTER_DISPEL,
    AURA_SCRIPT_HOOK_CHECK_TARGETS_LIST,
    // Spell Proc Hooks
    AURA_SCRIPT_HOOK_CHECK_PROC,
    AURA_SCRIPT_HOOK_PREPARE_PROC,
    AURA_SCRIPT_HOOK_PROC,
    AURA_SCRIPT_HOOK_EFFECT_PROC,
    AURA_SCRIPT_HOOK_EFFECT_AFTER_PROC,
    AURA_SCRIPT_HOOK_AFTER_PROC
    /*AURA_SCRIPT_HOOK_APPLY,
    AURA_SCRIPT_HOOK_REMOVE, */
};
/*
#define HOOK_AURA_EFFECT_START HOOK_AURA_EFFECT_APPLY
#define HOOK_AURA_EFFECT_END HOOK_AURA_EFFECT_CALC_SPELLMOD + 1
#define HOOK_AURA_EFFECT_COUNT HOOK_AURA_EFFECT_END - HOOK_AURA_EFFECT_START
*/
class AuraScript : public _SpellScript
{
    // internal use classes & functions
    // DO NOT OVERRIDE THESE IN SCRIPTS
    public:

    #define AURASCRIPT_FUNCTION_TYPE_DEFINES(CLASSNAME) \
        typedef void(CLASSNAME::*AuraCheckTargetsListFnType)(std::list<Unit*> &); \
        typedef bool(CLASSNAME::*AuraCheckAreaTargetFnType)(Unit* target); \
        typedef void(CLASSNAME::*AuraDispelFnType)(DispelInfo* dispelInfo); \
        typedef void(CLASSNAME::*AuraEffectApplicationModeFnType)(AuraEffect const*, AuraEffectHandleModes); \
        typedef void(CLASSNAME::*AuraEffectPeriodicFnType)(AuraEffect const*); \
        typedef void(CLASSNAME::*AuraUpdateFnType)(uint32); \
        typedef void(CLASSNAME::*AuraEffectUpdateFnType)(uint32, AuraEffect*); \
        typedef void(CLASSNAME::*AuraEffectUpdatePeriodicFnType)(AuraEffect*); \
        typedef void(CLASSNAME::*AuraEffectCalcAmountFnType)(AuraEffect const*, float &, bool &); \
        typedef void(CLASSNAME::*AuraEffectChangeTickDamageFnType)(AuraEffect const*, float &, Unit*); \
        typedef void(CLASSNAME::*AuraEffectCalcPeriodicFnType)(AuraEffect const*, bool &, int32 &); \
        typedef void(CLASSNAME::*AuraEffectCalcSpellModFnType)(AuraEffect const*, SpellModifier* &); \
        typedef void(CLASSNAME::*AuraEffectAbsorbFnType)(AuraEffect*, DamageInfo &, float &); \
        typedef bool(CLASSNAME::*AuraCheckProcFnType)(ProcEventInfo&); \
        typedef void(CLASSNAME::*AuraProcFnType)(ProcEventInfo&); \
        typedef void(CLASSNAME::*AuraEffectProcFnType)(AuraEffect const*, ProcEventInfo&); \
        typedef void(CLASSNAME::*AuraCalcMaxDurationFnType)(int32 &); \

        AURASCRIPT_FUNCTION_TYPE_DEFINES(AuraScript)

        class CheckTargetsListHandler
        {
            public:
                CheckTargetsListHandler(AuraCheckTargetsListFnType pHandlerScript);
                void Call(AuraScript* auraScript, std::list<Unit*>& unitTargets);
            private:
                AuraCheckTargetsListFnType pHandlerScript;
        };
        
        class CheckAreaTargetHandler
        {
            public:
                CheckAreaTargetHandler(AuraCheckAreaTargetFnType pHandlerScript);
                bool Call(AuraScript* auraScript, Unit* target);
            private:
                AuraCheckAreaTargetFnType pHandlerScript;
        };
        class AuraDispelHandler
        {
            public:
                AuraDispelHandler(AuraDispelFnType pHandlerScript);
                void Call(AuraScript* auraScript, DispelInfo* dispelInfo);
            private:
                AuraDispelFnType pHandlerScript;
        };
        class EffectBase : public  _SpellScript::EffectAuraNameCheck, public _SpellScript::EffectHook
        {
            public:
                EffectBase(uint8 _effIndex, uint16 _effName);
                std::string ToString();
                bool CheckEffect(SpellInfo const* spellEntry, uint8 effIndex) override;
        };
        class EffectPeriodicHandler : public EffectBase
        {
            public:
                EffectPeriodicHandler(AuraEffectPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect const* _aurEff);
            private:
                AuraEffectPeriodicFnType pEffectHandlerScript;
        };
        class AuraUpdateHandler
        {
            public:
                AuraUpdateHandler(AuraUpdateFnType _pEffectHandlerScript);
                void Call(AuraScript* auraScript, uint32 diff);
            private:
                AuraUpdateFnType pEffectHandlerScript;
        };
        class EffectUpdateHandler : public EffectBase
        {
            public:
                EffectUpdateHandler(AuraEffectUpdateFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, uint32 diff, AuraEffect* aurEff);
            private:
                AuraEffectUpdateFnType pEffectHandlerScript;
        };
        class EffectUpdatePeriodicHandler : public EffectBase
        {
            public:
                EffectUpdatePeriodicHandler(AuraEffectUpdatePeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect* aurEff);
            private:
                AuraEffectUpdatePeriodicFnType pEffectHandlerScript;
        };
        class EffectCalcAmountHandler : public EffectBase
        {
            public:
                EffectCalcAmountHandler(AuraEffectCalcAmountFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect const* aurEff, float & amount, bool & canBeRecalculated);
            private:
                AuraEffectCalcAmountFnType pEffectHandlerScript;
        };
        class CalcMaxDurationHandler
        {
            public:
                CalcMaxDurationHandler(AuraCalcMaxDurationFnType pHandlerScript);
                void Call(AuraScript* auraScript, int32 & maxDuration);
            private:
                AuraCalcMaxDurationFnType pHandlerScript;
        };
        class EffectChangeTickDamageHandler : public EffectBase
        {
            public:
                EffectChangeTickDamageHandler(AuraEffectChangeTickDamageFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect const* aurEff, float & amount, Unit* target);
            private:
                AuraEffectChangeTickDamageFnType pEffectHandlerScript;
        };
        class EffectCalcPeriodicHandler : public EffectBase
        {
            public:
                EffectCalcPeriodicHandler(AuraEffectCalcPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect const* aurEff, bool & isPeriodic, int32 & periodicTimer);
            private:
                AuraEffectCalcPeriodicFnType pEffectHandlerScript;
        };
        class EffectCalcSpellModHandler : public EffectBase
        {
            public:
                EffectCalcSpellModHandler(AuraEffectCalcSpellModFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect const* aurEff, SpellModifier* & spellMod);
            private:
                AuraEffectCalcSpellModFnType pEffectHandlerScript;
        };
        class EffectApplyHandler : public EffectBase
        {
            public:
                EffectApplyHandler(AuraEffectApplicationModeFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName, AuraEffectHandleModes _mode);
                void Call(AuraScript* auraScript, AuraEffect const* _aurEff, AuraEffectHandleModes _mode);
            private:
                AuraEffectApplicationModeFnType pEffectHandlerScript;
                AuraEffectHandleModes mode;
        };
        class EffectAbsorbHandler : public EffectBase
        {
            public:
                EffectAbsorbHandler(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName);
                void Call(AuraScript* auraScript, AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount);
            private:
                AuraEffectAbsorbFnType pEffectHandlerScript;
        };
        class EffectManaShieldHandler : public EffectBase
        {
            public:
                EffectManaShieldHandler(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex);
                void Call(AuraScript* auraScript, AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount);
            private:
                AuraEffectAbsorbFnType pEffectHandlerScript;
        };
        class EffectProcHandler : public EffectBase
        {
            public:
                EffectProcHandler(AuraEffectProcFnType effectHandlerScript, uint8 effIndex, uint16 effName);
                void Call(AuraScript* auraScript, AuraEffect const* aurEff, ProcEventInfo& eventInfo);
            private:
                AuraEffectProcFnType _EffectHandlerScript;
        };

        #define AURASCRIPT_FUNCTION_CAST_DEFINES(CLASSNAME) \
        class CheckTargetsListFunction : public AuraScript::CheckTargetsListHandler { public: CheckTargetsListFunction(AuraCheckTargetsListFnType _pHandlerScript) : AuraScript::CheckTargetsListHandler((AuraScript::AuraCheckTargetsListFnType)_pHandlerScript) {} }; \
        class CheckAreaTargetFunction : public AuraScript::CheckAreaTargetHandler { public: CheckAreaTargetFunction(AuraCheckAreaTargetFnType _pHandlerScript) : AuraScript::CheckAreaTargetHandler((AuraScript::AuraCheckAreaTargetFnType)_pHandlerScript) {} }; \
        class AuraDispelFunction : public AuraScript::AuraDispelHandler { public: AuraDispelFunction(AuraDispelFnType _pHandlerScript) : AuraScript::AuraDispelHandler((AuraScript::AuraDispelFnType)_pHandlerScript) {} }; \
        class EffectPeriodicHandlerFunction : public AuraScript::EffectPeriodicHandler { public: EffectPeriodicHandlerFunction(AuraEffectPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectPeriodicHandler((AuraScript::AuraEffectPeriodicFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class AuraUpdateHandlerFunction : public AuraScript::AuraUpdateHandler { public: AuraUpdateHandlerFunction(AuraUpdateFnType _pEffectHandlerScript) : AuraScript::AuraUpdateHandler((AuraScript::AuraUpdateFnType)_pEffectHandlerScript) {} }; \
        class EffectUpdateHandlerFunction : public AuraScript::EffectUpdateHandler { public: EffectUpdateHandlerFunction(AuraEffectUpdateFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectUpdateHandler((AuraScript::AuraEffectUpdateFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectUpdatePeriodicHandlerFunction : public AuraScript::EffectUpdatePeriodicHandler { public: EffectUpdatePeriodicHandlerFunction(AuraEffectUpdatePeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectUpdatePeriodicHandler((AuraScript::AuraEffectUpdatePeriodicFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectCalcAmountHandlerFunction : public AuraScript::EffectCalcAmountHandler { public: EffectCalcAmountHandlerFunction(AuraEffectCalcAmountFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectCalcAmountHandler((AuraScript::AuraEffectCalcAmountFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class CalcMaxDurationHandlerFunction : public AuraScript::CalcMaxDurationHandler { public: CalcMaxDurationHandlerFunction(AuraCalcMaxDurationFnType _pHandlerScript) : AuraScript::CalcMaxDurationHandler((AuraScript::AuraCalcMaxDurationFnType)_pHandlerScript) {} }; \
        class EffectChangeTickDamageHandlerFunction : public AuraScript::EffectChangeTickDamageHandler { public: EffectChangeTickDamageHandlerFunction(AuraEffectChangeTickDamageFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectChangeTickDamageHandler((AuraScript::AuraEffectChangeTickDamageFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectCalcPeriodicHandlerFunction : public AuraScript::EffectCalcPeriodicHandler { public: EffectCalcPeriodicHandlerFunction(AuraEffectCalcPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectCalcPeriodicHandler((AuraScript::AuraEffectCalcPeriodicFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectCalcSpellModHandlerFunction : public AuraScript::EffectCalcSpellModHandler { public: EffectCalcSpellModHandlerFunction(AuraEffectCalcSpellModFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectCalcSpellModHandler((AuraScript::AuraEffectCalcSpellModFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectApplyHandlerFunction : public AuraScript::EffectApplyHandler { public: EffectApplyHandlerFunction(AuraEffectApplicationModeFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName, AuraEffectHandleModes _mode) : AuraScript::EffectApplyHandler((AuraScript::AuraEffectApplicationModeFnType)_pEffectHandlerScript, _effIndex, _effName, _mode) {} }; \
        class EffectAbsorbFunction : public AuraScript::EffectAbsorbHandler { public: EffectAbsorbFunction(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : AuraScript::EffectAbsorbHandler((AuraScript::AuraEffectAbsorbFnType)_pEffectHandlerScript, _effIndex, _effName) {} }; \
        class EffectManaShieldFunction : public AuraScript::EffectManaShieldHandler { public: EffectManaShieldFunction(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex) : AuraScript::EffectManaShieldHandler((AuraScript::AuraEffectAbsorbFnType)_pEffectHandlerScript, _effIndex) {} }; \
        class EffectProcHandlerFunction : public AuraScript::EffectProcHandler { public: EffectProcHandlerFunction(AuraEffectProcFnType effectHandlerScript, uint8 effIndex, uint16 effName) : AuraScript::EffectProcHandler((AuraScript::AuraEffectProcFnType)effectHandlerScript, effIndex, effName) {} }; \

        #define PrepareAuraScript(CLASSNAME) AURASCRIPT_FUNCTION_TYPE_DEFINES(CLASSNAME) AURASCRIPT_FUNCTION_CAST_DEFINES(CLASSNAME)

    public:
        AuraScript() : _SpellScript(), m_aura(nullptr), m_auraApplication(nullptr), m_defaultActionPrevented(false)
        {}
        bool _Validate(SpellInfo const* entry) override;
        bool _Load(Aura* aura);
        void _PrepareScriptCall(AuraScriptHookType hookType, AuraApplication const* aurApp = nullptr);
        void _FinishScriptCall();
        bool _IsDefaultActionPrevented();
    private:
        Aura* m_aura;
        AuraApplication const* m_auraApplication;
        bool m_defaultActionPrevented;

        class ScriptStateStore
        {
        public:
            uint8 _currentScriptState;
            AuraApplication const* _auraApplication;
            bool _defaultActionPrevented;
            ScriptStateStore(uint8 currentScriptState, AuraApplication const* auraApplication, bool defaultActionPrevented)
                : _currentScriptState(currentScriptState), _auraApplication(auraApplication), _defaultActionPrevented(defaultActionPrevented)
            {}
        };
        typedef std::stack<ScriptStateStore> ScriptStateStack;
        ScriptStateStack m_scriptStates;
        sf::contention_free_shared_mutex< > i_scriptStatesLock;

    public:
        //
        // AuraScript interface
        // hooks to which you can attach your functions
        //
        // executed when area aura checks if it can be applied on target
        // example: OnEffectApply += AuraEffectApplyFn(class::function);
        // where function is: bool function (Unit* target);
        HookList<CheckAreaTargetHandler> DoCheckAreaTarget;
        #define AuraCheckAreaTargetFn(F) CheckAreaTargetFunction(&F)
        
        HookList<CheckTargetsListHandler> DoCheckTargetsList;
        #define AuraCheckTargetsListFn(F) CheckTargetsListFunction(&F)

        // executed when aura is dispelled by a unit
        // example: OnDispel += AuraDispelFn(class::function);
        // where function is: void function (DispelInfo* dispelInfo);
        HookList<AuraDispelHandler> OnDispel;
        // executed after aura is dispelled by a unit
        // example: AfterDispel += AuraDispelFn(class::function);
        // where function is: void function (DispelInfo* dispelInfo);
        HookList<AuraDispelHandler> AfterDispel;
        #define AuraDispelFn(F) AuraDispelFunction(&F)

        // executed when aura effect is applied with specified mode to target
        // should be used when when effect handler preventing/replacing is needed, do not use this hook for triggering spellcasts/removing auras etc - may be unsafe
        // example: OnEffectApply += AuraEffectApplyFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier, AuraEffectHandleModes);
        // where function is: void function (AuraEffect const* aurEff, AuraEffectHandleModes mode);
        HookList<EffectApplyHandler> OnEffectApply;
        // executed after aura effect is applied with specified mode to target
        // example: AfterEffectApply += AuraEffectApplyFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier, AuraEffectHandleModes);
        // where function is: void function (AuraEffect const* aurEff, AuraEffectHandleModes mode);
        HookList<EffectApplyHandler> AfterEffectApply;
        #define AuraEffectApplyFn(F, I, N, M) EffectApplyHandlerFunction(&F, I, N, M)

        // executed after aura effect is removed with specified mode from target
        // should be used when when effect handler preventing/replacing is needed, do not use this hook for triggering spellcasts/removing auras etc - may be unsafe
        // example: OnEffectRemove += AuraEffectRemoveFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier, AuraEffectHandleModes);
        // where function is: void function (AuraEffect const* aurEff, AuraEffectHandleModes mode);
        HookList<EffectApplyHandler> OnEffectRemove;
        // executed when aura effect is removed with specified mode from target
        // example: AfterEffectRemove += AuraEffectRemoveFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier, AuraEffectHandleModes);
        // where function is: void function (AuraEffect const* aurEff, AuraEffectHandleModes mode);
        HookList<EffectApplyHandler> AfterEffectRemove;
        #define AuraEffectRemoveFn(F, I, N, M) EffectApplyHandlerFunction(&F, I, N, M)

        // executed when periodic aura effect ticks on target
        // example: OnEffectPeriodic += AuraEffectPeriodicFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect const* aurEff);
        HookList<EffectPeriodicHandler> OnEffectPeriodic;
        #define AuraEffectPeriodicFn(F, I, N) EffectPeriodicHandlerFunction(&F, I, N)

        // executed when aura is updated
        // example: OnAuraUpdate += AuraUpdateFn(class::function);
        // where function is: void function (const uint32 diff);
        HookList<AuraUpdateHandler> OnAuraUpdate;
        #define AuraUpdateFn(F) AuraUpdateHandlerFunction(&F)

        // executed when aura effect is updated
        // example: OnEffectUpdate += AuraEffectUpdateFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect* aurEff);
        HookList<EffectUpdateHandler> OnEffectUpdate;
        #define AuraEffectUpdateFn(F, I, N) EffectUpdateHandlerFunction(&F, I, N)

        // executed when periodic aura effect is updated
        // example: OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect* aurEff);
        HookList<EffectUpdatePeriodicHandler> OnEffectUpdatePeriodic;
        #define AuraEffectUpdatePeriodicFn(F, I, N) EffectUpdatePeriodicHandlerFunction(&F, I, N)

        // executed when aura effect calculates amount
        // example: DoEffectCalcAmount += AuraEffectCalcAmounFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect* aurEff, int32& amount, bool& canBeRecalculated);
        HookList<EffectCalcAmountHandler> DoEffectBeforeCalcAmount;
        HookList<EffectCalcAmountHandler> DoEffectCalcAmount;
        #define AuraEffectCalcAmountFn(F, I, N) EffectCalcAmountHandlerFunction(&F, I, N)

        HookList<CalcMaxDurationHandler> DoCalcMaxDuration;
        #define AuraCalcMaxDurationFn(F) CalcMaxDurationHandlerFunction(&F)

        // executed when aura effect calculates amount
        // example: DoEffectChangeTickDamage += AuraEffectChangeTickDamageFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect* aurEff, int32& amount);
        HookList<EffectChangeTickDamageHandler> DoEffectChangeTickDamage;
        #define AuraEffectChangeTickDamageFn(F, I, N) EffectChangeTickDamageHandlerFunction(&F, I, N)

        // executed when aura effect calculates periodic data
        // example: DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect const* aurEff, bool& isPeriodic, int32& amplitude);
        HookList<EffectCalcPeriodicHandler> DoEffectCalcPeriodic;
        #define AuraEffectCalcPeriodicFn(F, I, N) EffectCalcPeriodicHandlerFunction(&F, I, N)

        // executed when aura effect calculates spellmod
        // example: DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect const* aurEff, SpellModifier*& spellMod);
        HookList<EffectCalcSpellModHandler> DoEffectCalcSpellMod;
        #define AuraEffectCalcSpellModFn(F, I, N) EffectCalcSpellModHandlerFunction(&F, I, N)

        // executed when absorb aura effect is going to reduce damage
        // example: OnEffectAbsorb += AuraEffectAbsorbFn(class::function, EffectIndexSpecifier);
        // where function is: void function (AuraEffect const* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount);
        HookList<EffectAbsorbHandler> OnEffectAbsorb;
        #define AuraEffectAbsorbFn(F, I, N) EffectAbsorbFunction(&F, I, N)

        // executed after absorb aura effect reduced damage to target - absorbAmount is real amount absorbed by aura
        // example: AfterEffectAbsorb += AuraEffectAbsorbFn(class::function, EffectIndexSpecifier);
        // where function is: void function (AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount);
        HookList<EffectAbsorbHandler> AfterEffectAbsorb;

        // executed when mana shield aura effect is going to reduce damage
        // example: OnEffectManaShield += AuraEffectAbsorbFn(class::function, EffectIndexSpecifier);
        // where function is: void function (AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount);
        HookList<EffectManaShieldHandler> OnEffectManaShield;
        #define AuraEffectManaShieldFn(F, I) EffectManaShieldFunction(&F, I)

        // executed after mana shield aura effect reduced damage to target - absorbAmount is real amount absorbed by aura
        // example: AfterEffectManaShield += AuraEffectAbsorbFn(class::function, EffectIndexSpecifier);
        // where function is: void function (AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount);
        HookList<EffectManaShieldHandler> AfterEffectManaShield;

        // example: OnEffectSplitDamage += AuraEffectAbsorbFn(class::function, EffectIndexSpecifier);
        HookList<EffectAbsorbHandler> OnEffectSplitDamage;

        // executed when aura effect procs
        // example: OnEffectProc += AuraEffectProcFn(class::function, EffectIndexSpecifier, EffectAuraNameSpecifier);
        // where function is: void function (AuraEffect const* aurEff, ProcEventInfo& procInfo);
        HookList<EffectProcHandler> OnEffectProc;
        #define AuraEffectProcFn(F, I, N) EffectProcHandlerFunction(&F, I, N)

        // AuraScript interface - hook/effect execution manipulators

        // prevents default action of a hook from being executed (works only while called in a hook which default action can be prevented)
        void PreventDefaultAction();

        // AuraScript interface - functions which are redirecting to Aura class

        // returns proto of the spell
        SpellInfo const* GetSpellInfo() const;
        // returns spellid of the spell
        uint32 GetId() const;

        // returns guid of object which casted the aura (m_originalCaster of the Spell class)
        ObjectGuid GetCasterGUID() const;
        // returns unit which casted the aura or NULL if not avalible (caster logged out for example)
        Unit* GetCaster() const;
        // returns object on which aura was casted, target for non-area auras, area aura source for area auras
        WorldObject* GetOwner() const;
        // returns owner if it's unit or unit derived object, NULL otherwise (only for persistent area auras NULL is returned)
        Unit* GetUnitOwner() const;
        // returns owner if it's dynobj, NULL otherwise
        DynamicObject* GetDynobjOwner() const;

        Position const* GetExplTargetDest();

        // removes aura with remove mode (see AuraRemoveMode enum)
        void Remove(uint32 removeMode = 0);
        // returns aura object of script
        Aura* GetAura() const;

        // returns type of the aura, may be dynobj owned aura or unit owned aura
        AuraObjectType GetType() const;

        // aura duration manipulation - when duration goes to 0 aura is removed
        int32 GetDuration() const;
        void SetDuration(int32 duration, bool withMods = false);
        void SetAuraTimer(int32 time, ObjectGuid guid = ObjectGuid::Empty);
        // sets duration to maxduration
        void RefreshDuration();
        time_t GetApplyTime() const;
        int32 GetMaxDuration() const;
        void SetMaxDuration(int32 duration);
        int32 CalcMaxDuration() const;
        // expired - duration just went to 0
        bool IsExpired() const;
        // permament - has infinite duration
        bool IsPermanent() const;

        // charges manipulation - 0 - not charged aura
        uint8 GetCharges() const;
        void SetCharges(uint8 charges);
        uint8 CalcMaxCharges() const;
        bool ModCharges(int8 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        // returns true if last charge dropped
        bool DropCharge(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        // stack amount manipulation
        uint16 GetStackAmount() const;
        void SetStackAmount(uint16 num);
        bool ModStackAmount(int16 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        // passive - "working in background", not saved, not removed by immunities, not seen by player
        bool IsPassive() const;
        // death persistent - not removed on death
        bool IsDeathPersistent() const;

        // check if aura has effect of given effindex
        bool HasEffect(uint8 effIndex) const;
        // returns aura effect of given effect index or NULL
        AuraEffect* GetEffect(uint8 effIndex) const;

        // check if aura has effect of given aura type
        bool HasEffectType(AuraType type) const;

        // Permit to communicate some data with the script
        virtual void SetData(uint32 type, uint32 data) {}

        // Permit to communicate some uint64 data with the script
        virtual void SetGuid(uint32 type, ObjectGuid const& data) {}

        // AuraScript interface - functions which are redirecting to AuraApplication class
        // Do not call these in hooks in which AuraApplication is not avalible, otherwise result will differ from expected (the functions will return NULL)

        // returns currently processed target of an aura
        // Return value does not need to be NULL-checked, the only situation this will (always)
        // return NULL is when the call happens in an unsupported hook, in other cases, it is always valid
        Unit* GetTarget() const;
        // returns AuraApplication object of currently processed target
        AuraApplication const* GetTargetApplication() const;

        // Call special function from scripts
        virtual uint32 CallSpecialFunction(uint32 Num = 0) { return 0; }
        virtual uint32 CallSpecialFunction(ObjectGuid const& GUID) { return 0; }
        virtual uint32 CallSpecialFunction(ObjectGuid& GUID) { return 0; }
};

//
// definitions:
//
// EffectIndexSpecifier - specifies conditions for effects
// EFFECT_0 - first effect matches
// EFFECT_1 - second effect matches
// EFFECT_2 - third effect matches
// EFFECT_FIRST_FOUND - first effect matching other conditions matches
// EFFECT_ALL - all effects of spell match
//
// EffectNameSpecifier - specifies conditions for spell effect names
// SPELL_EFFECT_ANY - any effect but not 0 matches condition
// SPELL_EFFECT_XXX - one of values of enum SpellEffects - effect with equal name matches
//

#endif // __SPELL_SCRIPT_H
