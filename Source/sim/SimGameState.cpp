#include "SimGameState.h"
#include "Net/UnrealNetwork.h"

ASimGameState::ASimGameState()
    : GameDay(1)
    , GameTimeOfDay(28800.f)
    , TimeScale(1.f)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.f;
}

void ASimGameState::AdvanceGameTime(float DeltaTime)
{
    if (!HasAuthority()) return;

    GameTimeOfDay += DeltaTime * TimeScale;
    if (GameTimeOfDay >= DayLength)
    {
        GameTimeOfDay -= DayLength;
        GameDay++;
    }
}

float ASimGameState::GetTimeOfDayNormalized() const
{
    return GameTimeOfDay / DayLength;
}

void ASimGameState::SetTimeScale(float NewScale)
{
    TimeScale = FMath::Clamp(NewScale, 0.f, 100.f);
}

void ASimGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASimGameState, GameDay);
    DOREPLIFETIME(ASimGameState, GameTimeOfDay);
    DOREPLIFETIME(ASimGameState, TimeScale);
}
