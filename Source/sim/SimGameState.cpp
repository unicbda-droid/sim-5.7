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

int32 ASimGameState::GetHours() const
{
    return FMath::FloorToInt(GameTimeOfDay / 3600.f);
}

int32 ASimGameState::GetMinutes() const
{
    float Remainder = GameTimeOfDay - (GetHours() * 3600.f);
    return FMath::FloorToInt(Remainder / 60.f);
}

int32 ASimGameState::GetSeconds() const
{
    float Remainder = GameTimeOfDay - (GetHours() * 3600.f) - (GetMinutes() * 60.f);
    return FMath::FloorToInt(Remainder);
}

FText ASimGameState::GetFormattedTime() const
{
    int32 H = GetHours();
    int32 M = GetMinutes();
    FNumberFormattingOptions Opts = FNumberFormattingOptions::DefaultNoGrouping();
    Opts.SetMinimumIntegralDigits(2);
    return FText::Format(NSLOCTEXT("SimLife", "TimeFormat", "{0}:{1}"),
        FText::AsNumber(H, &Opts),
        FText::AsNumber(M, &Opts));
}

bool ASimGameState::IsDayTime() const
{
    float H = GetHours();
    return H >= DawnHour && H < DuskHour;
}

bool ASimGameState::IsNightTime() const
{
    return !IsDayTime();
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
