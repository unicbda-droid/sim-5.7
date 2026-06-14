#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimNeedsTypes.h"
#include "SimInteractionComponent.generated.h"

class ASimCharacter;

USTRUCT(BlueprintType)
struct FSimInteraction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FText InteractionName;

    UPROPERTY(BlueprintReadWrite)
    float Duration;

    UPROPERTY(BlueprintReadWrite)
    TArray<FNeedModifier> NeedModifiers;

    UPROPERTY(BlueprintReadWrite)
    bool bRequiresTarget;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStarted, FSimInteraction, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, FSimInteraction, Interaction);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIM_API USimInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USimInteractionComponent();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Interact();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(FSimInteraction Interaction);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CancelInteraction();

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionStarted OnInteractionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionCompleted OnInteractionCompleted;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Interaction")
    bool bIsInteracting;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(Replicated)
    FSimInteraction CurrentInteraction;

    UPROPERTY(Replicated)
    float InteractionProgress;

    ASimCharacter* OwnerCharacter;

    void PerformInteraction();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
