#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimBuildingSystem.generated.h"

USTRUCT(BlueprintType)
struct FSimBuildableObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D GridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Thumbnail;
};

UCLASS()
class SIM_API ASimBuildingSystem : public AActor
{
    GENERATED_BODY()

public:
    ASimBuildingSystem();

    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable, WithValidation, Category = "Building")
    void ServerPlaceObject(TSubclassOf<AActor> ObjectClass, FVector Location, FRotator Rotation);

    UFUNCTION(Server, Reliable, WithValidation, Category = "Building")
    void ServerRemoveObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Building")
    TArray<FSimBuildableObject> GetAvailableObjects() const { return AvailableObjects; }

    UFUNCTION(BlueprintPure, Category = "Building")
    FVector SnapToGrid(FVector WorldLocation, float GridSize = 100.f) const;

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool CanPlaceObjectAt(FVector Location, FVector2D GridSize) const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
    TArray<FSimBuildableObject> AvailableObjects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
    float GridCellSize;

    UPROPERTY()
    TArray<AActor*> PlacedObjects;
};
