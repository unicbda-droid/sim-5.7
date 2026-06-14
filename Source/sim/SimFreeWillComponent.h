#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimNeedsTypes.h"
#include "SimActionQueueComponent.h"
#include "SimFreeWillComponent.generated.h"

class ASimCharacter;
class USimNeedsComponent;

USTRUCT(BlueprintType)
struct FObjectInteractionDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText InteractionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENeedsType FulfillsNeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FulfillmentStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNeedModifier> NeedModifiers;
};

USTRUCT(BlueprintType)
struct FSimObjectDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FObjectInteractionDef> Interactions;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIM_API USimFreeWillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USimFreeWillComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "FreeWill")
    void EvaluateNeeds();

    UFUNCTION(BlueprintCallable, Category = "FreeWill")
    FSimAction FindBestActionForNeed(ENeedsType Need);

    UFUNCTION(BlueprintCallable, Category = "FreeWill")
    void SetEnabled(bool bInEnabled);

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeWill")
    TArray<FSimObjectDef> KnownObjects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FreeWill")
    float EvaluationInterval;

    UPROPERTY()
    ASimCharacter* OwnerCharacter;

    UPROPERTY()
    USimActionQueueComponent* ActionQueue;

    UPROPERTY()
    USimNeedsComponent* NeedsComponent;

    UPROPERTY()
    bool bEnabled;

    float TimeSinceLastEvaluation;

    void PerformAutonomousAction();
    ENeedsType GetMostCriticalNeed() const;
};
