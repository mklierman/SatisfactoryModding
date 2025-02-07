#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGGenericBuildableHologram.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGHologram.h"
#include "../../../../SML/Source/SML/Public/Patching/NativeHookManager.h"
#include "../../../../../Source/FactoryGame/Public/Hologram/FGBuildableHologram.h"
#include "Modules/ModuleManager.h"

class FInfiniteNudgeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	static FVector NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction);

	static void NudgeHologram(const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult);
	static void NudgeGenericHologram(const AFGGenericBuildableHologram* self, const FVector& NudgeInput, const FHitResult& HitResult);
	static FVector AddNudgeOffset(AFGHologram* self, const FVector& Direction);

	static void RotateLockedHologram(AFGHologram* self, int32 delta);
	static void ScaleHologram(AFGHologram* self);

	static void DebugChecker();
};