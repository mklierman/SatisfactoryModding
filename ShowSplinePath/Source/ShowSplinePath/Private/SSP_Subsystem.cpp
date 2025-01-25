#include "SSP_Subsystem.h"
#include "FGSplineMeshGenerationLibrary.h"
#include <Kismet/GameplayStatics.h>

#pragma optimize("", off)

ASSP_Subsystem::ASSP_Subsystem() : Super(), splinePathMesh(nullptr), mantaPathMesh(nullptr)
{
	//Spawn on client will help us catch exceptions where the server tries to spawn the path mesh components itself.
	//We don't want that as other players should not be bothered with our path visualization.
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
}

bool ASSP_Subsystem::GetHasAuthority()
{
	return HasAuthority();
}

bool ASSP_Subsystem::ShouldSave_Implementation() const
{
	return true;
}

USplineMeshComponent* SplineMeshConstructor(USplineComponent* spline)
{
	USplineMeshComponent* newComponent = NewObject<USplineMeshComponent>(
		spline, USplineMeshComponent::StaticClass());
	newComponent->SetupAttachment(spline);
	newComponent->Mobility = EComponentMobility::Static;
	return newComponent;
}

void BuildSplineMeshes(
	class USplineComponent* spline,
	float maxSplineLengthToFill,
	UStaticMesh* mesh,
	float meshLength,
	TArray<USplineMeshComponent*>& localMeshPool,
	bool makeMovable = false)
{
	const float splineLengthToFill = FMath::Clamp(spline->GetSplineLength(), 0.f, maxSplineLengthToFill);
	const int32 numMeshes = splineLengthToFill > SMALL_NUMBER
		                        ? FMath::Max(1, FMath::RoundToInt(splineLengthToFill / meshLength))
		                        : 0;

	// Create more or remove the excess meshes.
	if (numMeshes < localMeshPool.Num())
	{
		while (localMeshPool.Num() > numMeshes)
		{
			localMeshPool.Last()->DestroyComponent();
			localMeshPool.Pop();
		}
	}
	else if (numMeshes > localMeshPool.Num())
	{
		while (localMeshPool.Num() < numMeshes)
		{
			if (auto newMesh = SplineMeshConstructor(spline))
			{
				if (makeMovable)
				{
					newMesh->SetMobility(EComponentMobility::Movable);
				}
				localMeshPool.Push(newMesh);
			}
			else
			{
				break;
			}
		}
	}

	// Put all pieces along the spline.
	{
		const float segmentLength = splineLengthToFill / numMeshes;

		for (int32 i = 0; i < localMeshPool.Num(); ++i)
		{
			const float startDistance = (float)i * segmentLength;
			const float endDistance = (float)(i + 1) * segmentLength;
			const FVector startPos = spline->GetLocationAtDistanceAlongSpline(
				startDistance, ESplineCoordinateSpace::Local);
			const FVector startTangent = spline->GetTangentAtDistanceAlongSpline(
				startDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;
			const FVector endPos = spline->GetLocationAtDistanceAlongSpline(endDistance, ESplineCoordinateSpace::Local);
			const FVector endTangent = spline->GetTangentAtDistanceAlongSpline(
				endDistance, ESplineCoordinateSpace::Local).GetSafeNormal() * segmentLength;

			localMeshPool[i]->SetStartAndEnd(startPos, startTangent, endPos, endTangent, true);
			localMeshPool[i]->SetStaticMesh(mesh);
		}
	}

	// Register new meshes, needs to happen after the properties are set for static components.
	for (auto meshComp : localMeshPool)
	{
		if (!meshComp->IsRegistered())
		{
			meshComp->RegisterComponent();
		}
	}
}

void ASSP_Subsystem::HandlePathSplines(AFGDrivingTargetList* targetList, bool show)
{
	if (targetList)
	{
		//targetList->SetPathVisible(show);
		if (show)
		{
			targetList->CalculateTargetCount();
			targetList->CreatePath();
			auto spline = targetList->GetPath();
			if (spline)
			{
				TArray<USplineMeshComponent*> localPool;
				BuildSplineMeshes(spline, spline->GetSplineLength(), splinePathMesh, 100, localPool);

				FSSP_MeshPoolStruct pool;
				pool.MeshPool = localPool;
				TargetListMeshPools.Add(targetList, pool);
			}
		}
		else
		{
			if (TargetListMeshPools.Contains(targetList))
			{
				TArray<USplineMeshComponent*> meshPool = TargetListMeshPools[targetList].MeshPool;
				for (auto meshComp : meshPool)
				{
					if (meshComp && meshComp->IsRegistered())
					{
						meshComp->UnregisterComponent();
					}
				}
				meshPool.Empty();
				TargetListMeshPools.Remove(targetList);
			}
		}
	}
}

void ASSP_Subsystem::ShowTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		targetList->SetPathVisible(true);
		HandlePathSplines(targetList, true);
	}
}

void ASSP_Subsystem::HideTargetPath(AFGTargetPoint* targetPoint)
{
	auto targetList = targetPoint->GetOwningList();
	if (targetList)
	{
		targetList->SetPathVisible(false);
		HandlePathSplines(targetList, false);
	}
}

void ASSP_Subsystem::ToggleVehiclePath(AFGWheeledVehicle* vehicle)
{
	auto targetList = vehicle->GetTargetList();
	if (targetList)
	{
		HandlePathSplines(targetList, !targetList->IsPathVisible());
	}
}

void ASSP_Subsystem::ShowInitialPaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (AActor* actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			if (tpList && tpList->mIsPathVisible)
			{
				HandlePathSplines(tpList, true);
			}
		}
	}
}

void ASSP_Subsystem::ShowAllMantaPaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGManta::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto manta = Cast<AFGManta>(actor1);
			if (manta)
			{
				auto spline = manta->mCachedSpline;
				if (spline)
				{
					TArray<USplineMeshComponent*> localPool;
					BuildSplineMeshes(spline, spline->GetSplineLength(), mantaPathMesh, 2500, localPool,
					                  true);

					FSSP_MeshPoolStruct pool;
					pool.MeshPool = localPool;
					MantaMeshPools.Add(manta, pool);
				}
			}
		}
	}
}

void ASSP_Subsystem::HideAllMantaPaths(AActor* actor)
{
	for (auto mantaPair : MantaMeshPools)
	{
		TArray<USplineMeshComponent*> meshPool = mantaPair.Value.MeshPool;
		for (auto meshComp : meshPool)
		{
			if (meshComp && meshComp->IsRegistered())
			{
				meshComp->UnregisterComponent();
			}
		}
		meshPool.Empty();
	}
	MantaMeshPools.Empty();
}

void ASSP_Subsystem::ShowAllVehiclePaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			tpList->SetPathVisible(true);
		}
	}
}

void ASSP_Subsystem::HideAllVehiclePaths(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGDrivingTargetList::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (auto actor1 : FoundActors)
		{
			auto tpList = Cast<AFGDrivingTargetList>(actor1);
			tpList->SetPathVisible(false);
		}
	}
}
#pragma optimize("", on)
