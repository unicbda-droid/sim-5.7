#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimNeedsTypes.h"
#include "SimNeedsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNeedChanged, ENeedsType, NeedType, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalNeed, ENeedsType, NeedType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIM_API USimNeedsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USimNeedsComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Needs")
    TArray<float> NeedValues;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Needs")
    TArray<float> NeedDecayRates;

    UPROPERTY(BlueprintAssignable, Category = "Needs")
    FOnNeedChanged OnNeedChanged;

    UPROPERTY(BlueprintAssignable, Category = "Needs")
    FOnCriticalNeed OnCriticalNeed;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SetNeedValue(ENeedsType NeedType, float Value);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    float GetNeedValue(ENeedsType NeedType) const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void ModifyNeed(ENeedsType NeedType, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    bool IsNeedCritical(ENeedsType NeedType) const;

    UFUNCTION(BlueprintPure, Category = "Needs")
    float GetOverallHappiness() const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SetNeedsDecayEnabled(bool bEnabled);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(Replicated)
    bool bDecayEnabled;

    const float MinNeed = 0.f;
    const float MaxNeed = 100.f;
    const float CriticalThreshold = 15.f;

    void InitializeNeeds();
    void DecayNeeds(float DeltaTime);

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
