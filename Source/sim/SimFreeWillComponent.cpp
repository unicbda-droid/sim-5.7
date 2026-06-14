#include "SimFreeWillComponent.h"
#include "SimCharacter.h"
#include "SimNeedsComponent.h"
#include "SimActionQueueComponent.h"
#include "Net/UnrealNetwork.h"

USimFreeWillComponent::USimFreeWillComponent()
    : EvaluationInterval(5.f)
    , bEnabled(true)
    , TimeSinceLastEvaluation(0.f)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.f;
    SetIsReplicatedByDefault(true);
}

void USimFreeWillComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ASimCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        ActionQueue = OwnerCharacter->FindComponentByClass<USimActionQueueComponent>();
        NeedsComponent = OwnerCharacter->NeedsComponent;
    }
}

void USimFreeWillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnabled || GetOwnerRole() != ROLE_Authority) return;
    if (!ActionQueue || !NeedsComponent) return;

    TimeSinceLastEvaluation += DeltaTime;

    // Only evaluate periodically
    if (TimeSinceLastEvaluation >= EvaluationInterval)
    {
        TimeSinceLastEvaluation = 0.f;

        // Don't override user actions
        if (!ActionQueue->IsBusy())
        {
            EvaluateNeeds();
        }
    }
}

void USimFreeWillComponent::EvaluateNeeds()
{
    if (!NeedsComponent) return;

    ENeedsType MostCritical = GetMostCriticalNeed();

    // Only act if need is actually critical
    if (NeedsComponent->IsNeedCritical(MostCritical))
    {
        FSimAction BestAction = FindBestActionForNeed(MostCritical);
        if (!BestAction.ActionName.IsEmpty())
        {
            ActionQueue->EnqueueAction(BestAction);
        }
    }
}

ENeedsType USimFreeWillComponent::GetMostCriticalNeed() const
{
    if (!NeedsComponent) return ENeedsType::MAX;

    ENeedsType MostCritical = ENeedsType::MAX;
    float LowestValue = 100.f;

    int32 MaxNeeds = static_cast<int32>(ENeedsType::MAX);
    for (int32 i = 0; i < MaxNeeds; i++)
    {
        ENeedsType NeedType = static_cast<ENeedsType>(i);
        float Value = NeedsComponent->GetNeedValue(NeedType);
        if (Value < LowestValue)
        {
            LowestValue = Value;
            MostCritical = NeedType;
        }
    }

    return MostCritical;
}

FSimAction USimFreeWillComponent::FindBestActionForNeed(ENeedsType Need)
{
    FSimAction Result;

    // Search known objects for interactions fulfilling this need
    for (const FSimObjectDef& ObjDef : KnownObjects)
    {
        for (const FObjectInteractionDef& Interaction : ObjDef.Interactions)
        {
            if (Interaction.FulfillsNeed == Need)
            {
                Result.ActionName = Interaction.InteractionName;
                Result.Duration = Interaction.Duration;
                Result.NeedModifiers = Interaction.NeedModifiers;
                Result.Priority = EActionPriority::Low;
                Result.bInterruptable = true;
                return Result;
            }
        }
    }

    return Result;
}

void USimFreeWillComponent::SetEnabled(bool bInEnabled)
{
    bEnabled = bInEnabled;
}
