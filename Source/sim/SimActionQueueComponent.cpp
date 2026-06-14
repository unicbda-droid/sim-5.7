#include "SimActionQueueComponent.h"
#include "SimCharacter.h"
#include "SimNeedsComponent.h"
#include "Net/UnrealNetwork.h"

USimActionQueueComponent::USimActionQueueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    SetIsReplicatedByDefault(true);
    bIsExecuting = false;
    ActionProgress = 0.f;
}

void USimActionQueueComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ASimCharacter>(GetOwner());
}

void USimActionQueueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetOwnerRole() != ROLE_Authority) return;

    if (!bIsExecuting)
    {
        AdvanceToNextAction();
    }

    if (bIsExecuting)
    {
        ActionProgress += DeltaTime;
        if (ActionProgress >= CurrentAction.Duration)
        {
            CompleteCurrentAction();
        }
    }
}

void USimActionQueueComponent::EnqueueAction(FSimAction Action)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    // Insert sorted by priority (higher = sooner)
    int32 InsertIndex = 0;
    for (; InsertIndex < ActionQueue.Num(); InsertIndex++)
    {
        if (ActionQueue[InsertIndex].Priority < Action.Priority)
        {
            break;
        }
    }
    ActionQueue.Insert(Action, InsertIndex);

    // If current action can be interrupted by higher priority
    if (bIsExecuting && Action.Priority > CurrentAction.Priority)
    {
        InterruptCurrent(Action.Priority);
    }
}

bool USimActionQueueComponent::DequeueAction(FSimAction& OutAction)
{
    if (ActionQueue.Num() == 0) return false;
    OutAction = ActionQueue[0];
    ActionQueue.RemoveAt(0);
    return true;
}

void USimActionQueueComponent::ClearQueue()
{
    ActionQueue.Empty();
    if (bIsExecuting)
    {
        bIsExecuting = false;
        ActionProgress = 0.f;
        OnActionInterrupted.Broadcast(CurrentAction);
    }
}

void USimActionQueueComponent::InterruptCurrent(EActionPriority NewPriority)
{
    if (!bIsExecuting || !CurrentAction.bInterruptable) return;

    OnActionInterrupted.Broadcast(CurrentAction);
    bIsExecuting = false;
    ActionProgress = 0.f;
    AdvanceToNextAction();
}

void USimActionQueueComponent::AdvanceToNextAction()
{
    FSimAction NextAction;
    if (DequeueAction(NextAction))
    {
        bIsExecuting = true;
        ActionProgress = 0.f;
        CurrentAction = NextAction;
        OnActionStarted.Broadcast(CurrentAction);
    }
}

void USimActionQueueComponent::CompleteCurrentAction()
{
    if (!OwnerCharacter) return;

    USimNeedsComponent* Needs = OwnerCharacter->NeedsComponent;
    if (Needs)
    {
        for (const FNeedModifier& Mod : CurrentAction.NeedModifiers)
        {
            Needs->ModifyNeed(Mod.NeedType, Mod.Delta);
        }
    }

    OnActionCompleted.Broadcast(CurrentAction);
    bIsExecuting = false;
    ActionProgress = 0.f;
}

void USimActionQueueComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USimActionQueueComponent, ActionQueue);
    DOREPLIFETIME(USimActionQueueComponent, CurrentAction);
    DOREPLIFETIME(USimActionQueueComponent, ActionProgress);
    DOREPLIFETIME(USimActionQueueComponent, bIsExecuting);
}
