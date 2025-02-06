#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "Components/SplineMeshComponent.h"
#include "FGManta.h"

#include "SSP_Subsystem.generated.h"

USTRUCT(BlueprintType)
struct FSSP_MeshPoolStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<USplineMeshComponent*> MeshPool;
};

UCLASS()
class SHOWSPLINEPATH_API ASSP_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	ASSP_Subsystem();

	bool GetHasAuthority();

	virtual bool ShouldSave_Implementation() const override;
	
	UFUNCTION(BlueprintCallable)
	void HandlePathSplines(AFGDrivingTargetList* targetList, bool show);

	UFUNCTION(BlueprintCallable)
	virtual void ShowTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable)
	virtual void HideTargetPath(AFGTargetPoint* targetPoint);

	UFUNCTION(BlueprintCallable)
	virtual void ToggleVehiclePath(AFGWheeledVehicle* vehicle);

	UFUNCTION(BlueprintCallable)
	void ShowInitialPaths(AActor* actor);

	UFUNCTION(BlueprintCallable)
	void ShowAllMantaPaths(AActor* actor);
	
	UFUNCTION(BlueprintCallable)
	void HideAllMantaPaths(AActor* actor);

	UFUNCTION(BlueprintCallable)
	void ShowAllVehiclePaths(AActor* actor);
	
	UFUNCTION(BlueprintCallable)
	void HideAllVehiclePaths(AActor* actor);


	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* splinePathMesh;

	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* mantaPathMesh;

	UPROPERTY(BlueprintReadWrite)
	TMap<AFGDrivingTargetList*, FSSP_MeshPoolStruct> TargetListMeshPools;

	UPROPERTY(BlueprintReadWrite)
	TMap<AFGManta*, FSSP_MeshPoolStruct> MantaMeshPools;
};
