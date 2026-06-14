#include "SimInteractionComponent.h"
#include "SimCharacter.h"
#include "SimNeedsComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

USimInteractionComponent::USimInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    SetIsReplicatedByDefault(true);
    bIsInteracting = false;
    InteractionProgress = 0.f;
}

void USimInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ASimCharacter>(GetOwner());
}

void USimInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsInteracting && GetOwnerRole() == ROLE_Authority)
    {
        InteractionProgress += DeltaTime;
        if (InteractionProgress >= CurrentInteraction.Duration)
        {
            PerformInteraction();
            bIsInteracting = false;
            OnInteractionCompleted.Broadcast(CurrentInteraction);
        }
    }
}

void USimInteractionComponent::Interact()
{
    if (bIsInteracting) return;
    if (!OwnerCharacter) return;

    FSimInteraction DummyInteraction;
    DummyInteraction.InteractionName = NSLOCTEXT("SimLife", "DefaultInteract", "Interact");
    DummyInteraction.Duration = 2.f;
    DummyInteraction.bRequiresTarget = false;

    StartInteraction(DummyInteraction);
}

void USimInteractionComponent::StartInteraction(FSimInteraction Interaction)
{
    if (bIsInteracting || GetOwnerRole() != ROLE_Authority) return;

    bIsInteracting = true;
    InteractionProgress = 0.f;
    CurrentInteraction = Interaction;
    OnInteractionStarted.Broadcast(Interaction);
}

void USimInteractionComponent::CancelInteraction()
{
    bIsInteracting = false;
    InteractionProgress = 0.f;
}

void USimInteractionComponent::PerformInteraction()
{
    if (!OwnerCharacter) return;

    USimNeedsComponent* Needs = OwnerCharacter->NeedsComponent;
    if (!Needs) return;

    for (const FNeedModifier& Modifier : CurrentInteraction.NeedModifiers)
    {
        Needs->ModifyNeed(Modifier.NeedType, Modifier.Delta);
    }
}

void USimInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USimInteractionComponent, bIsInteracting);
    DOREPLIFETIME(USimInteractionComponent, CurrentInteraction);
    DOREPLIFETIME(USimInteractionComponent, InteractionProgress);
}
