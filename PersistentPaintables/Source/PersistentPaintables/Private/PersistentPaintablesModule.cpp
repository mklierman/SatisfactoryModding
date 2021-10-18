#include "PersistentPaintablesModule.h"
#include "Buildables/FGBuildable.h"
#include "Patching/NativeHookManager.h"
#include "PersistentPaintablesCppSubSystem.h"
#include "Subsystem/SubsystemActorManager.h"

void FPersistentPaintablesModule::StartupModule() {
	//virtual void PlayBuildEffects( AActor* inInstigator );
	AFGBuildable* CDOBuildable = GetMutableDefault<AFGBuildable>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::PlayBuildEffects, CDOBuildable, [](auto& scope, AFGBuildable* self, AActor* instigator) {
		// do some nice stuff there

		UWorld* WorldObject = self->GetWorld();
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		APersistentPaintablesCppSubSystem* subsystem = SubsystemActorManager->GetSubsystemActor<APersistentPaintablesCppSubSystem>();
		subsystem->OnBuildEffectStarted.Broadcast(self, instigator);
		});
#endif
}


IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);