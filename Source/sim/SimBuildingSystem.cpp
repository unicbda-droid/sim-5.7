#include "SimBuildingSystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ASimBuildingSystem::ASimBuildingSystem()
    : GridCellSize(100.f)
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ASimBuildingSystem::BeginPlay()
{
    Super::BeginPlay();
}

FVector ASimBuildingSystem::SnapToGrid(FVector WorldLocation, float GridSize) const
{
    float CellSize = GridSize > 0.f ? GridSize : GridCellSize;
    float X = FMath::GridSnap(WorldLocation.X, CellSize);
    float Y = FMath::GridSnap(WorldLocation.Y, CellSize);
    float Z = WorldLocation.Z;
    return FVector(X, Y, Z);
}

bool ASimBuildingSystem::CanPlaceObjectAt(FVector Location, FVector2D ObjectGridSize) const
{
    for (const AActor* Placed : PlacedObjects)
    {
        if (Placed && Placed->GetActorLocation().Equals(Location, GridCellSize * 0.5f))
        {
            return false;
        }
    }
    return true;
}

void ASimBuildingSystem::ServerPlaceObject_Implementation(TSubclassOf<AActor> ObjectClass, FVector Location, FRotator Rotation)
{
    if (!ObjectClass || !HasAuthority()) return;

    FVector SnappedLocation = SnapToGrid(Location);
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (AActor* NewObject = GetWorld()->SpawnActor<AActor>(ObjectClass, SnappedLocation, Rotation, SpawnParams))
    {
        PlacedObjects.Add(NewObject);
    }
}

bool ASimBuildingSystem::ServerPlaceObject_Validate(TSubclassOf<AActor> ObjectClass, FVector Location, FRotator Rotation)
{
    return ObjectClass != nullptr;
}

void ASimBuildingSystem::ServerRemoveObject_Implementation(AActor* Object)
{
    if (Object && HasAuthority())
    {
        PlacedObjects.Remove(Object);
        Object->Destroy();
    }
}

bool ASimBuildingSystem::ServerRemoveObject_Validate(AActor* Object)
{
    return Object != nullptr;
}
