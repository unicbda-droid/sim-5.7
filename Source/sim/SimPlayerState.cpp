#include "SimPlayerState.h"
#include "Net/UnrealNetwork.h"

ASimPlayerState::ASimPlayerState()
    : CharacterName(NSLOCTEXT("SimLife", "DefaultCharName", "Sim"))
    , Simoleons(500)
    , SkillPoints(0)
{
    bReplicates = true;
}

void ASimPlayerState::AddSimoleons(int32 Amount)
{
    if (HasAuthority())
    {
        Simoleons += Amount;
    }
}

bool ASimPlayerState::SpendSimoleons(int32 Amount)
{
    if (HasAuthority() && Simoleons >= Amount)
    {
        Simoleons -= Amount;
        return true;
    }
    return false;
}

void ASimPlayerState::AddSkillPoints(int32 Amount)
{
    if (HasAuthority())
    {
        SkillPoints += Amount;
    }
}

void ASimPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASimPlayerState, CharacterName);
    DOREPLIFETIME(ASimPlayerState, Simoleons);
    DOREPLIFETIME(ASimPlayerState, SkillPoints);
}
