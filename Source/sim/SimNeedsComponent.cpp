#include "SimNeedsComponent.h"
#include "Net/UnrealNetwork.h"

USimNeedsComponent::USimNeedsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    SetIsReplicatedByDefault(true);
    bDecayEnabled = true;

    const int32 NeedCount = static_cast<int32>(ENeedsType::MAX);
    NeedValues.SetNum(NeedCount);
    NeedDecayRates.SetNum(NeedCount);

    NeedDecayRates[static_cast<int32>(ENeedsType::Hunger)]  = 1.5f;
    NeedDecayRates[static_cast<int32>(ENeedsType::Bladder)] = 1.2f;
    NeedDecayRates[static_cast<int32>(ENeedsType::Energy)]  = 0.8f;
    NeedDecayRates[static_cast<int32>(ENeedsType::Hygiene)] = 0.5f;
    NeedDecayRates[static_cast<int32>(ENeedsType::Fun)]     = 0.6f;
    NeedDecayRates[static_cast<int32>(ENeedsType::Social)]  = 0.7f;
}

void USimNeedsComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeNeeds();
}

void USimNeedsComponent::InitializeNeeds()
{
    for (int32 i = 0; i < NeedValues.Num(); i++)
    {
        NeedValues[i] = MaxNeed;
    }
}

void USimNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bDecayEnabled && GetOwnerRole() == ROLE_Authority)
    {
        DecayNeeds(DeltaTime);
    }
}

void USimNeedsComponent::DecayNeeds(float DeltaTime)
{
    for (int32 i = 0; i < NeedValues.Num(); i++)
    {
        float NewValue = NeedValues[i] - NeedDecayRates[i] * DeltaTime;
        NewValue = FMath::Clamp(NewValue, MinNeed, MaxNeed);

        if (NewValue != NeedValues[i])
        {
            NeedValues[i] = NewValue;
            ENeedsType NeedType = static_cast<ENeedsType>(i);
            OnNeedChanged.Broadcast(NeedType, NewValue);

            if (NewValue <= CriticalThreshold)
            {
                OnCriticalNeed.Broadcast(NeedType);
            }
        }
    }
}

void USimNeedsComponent::SetNeedValue(ENeedsType NeedType, float Value)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    const int32 Index = static_cast<int32>(NeedType);
    if (!NeedValues.IsValidIndex(Index)) return;

    NeedValues[Index] = FMath::Clamp(Value, MinNeed, MaxNeed);
    OnNeedChanged.Broadcast(NeedType, NeedValues[Index]);
}

float USimNeedsComponent::GetNeedValue(ENeedsType NeedType) const
{
    const int32 Index = static_cast<int32>(NeedType);
    if (!NeedValues.IsValidIndex(Index)) return 0.f;
    return NeedValues[Index];
}

void USimNeedsComponent::ModifyNeed(ENeedsType NeedType, float Delta)
{
    if (GetOwnerRole() != ROLE_Authority) return;
    SetNeedValue(NeedType, GetNeedValue(NeedType) + Delta);
}

bool USimNeedsComponent::IsNeedCritical(ENeedsType NeedType) const
{
    return GetNeedValue(NeedType) <= CriticalThreshold;
}

float USimNeedsComponent::GetOverallHappiness() const
{
    float Total = 0.f;
    for (const float Value : NeedValues)
    {
        Total += Value;
    }
    return Total / static_cast<float>(NeedValues.Num());
}

void USimNeedsComponent::SetNeedsDecayEnabled(bool bEnabled)
{
    bDecayEnabled = bEnabled;
}

void USimNeedsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USimNeedsComponent, NeedValues);
    DOREPLIFETIME(USimNeedsComponent, bDecayEnabled);
}
