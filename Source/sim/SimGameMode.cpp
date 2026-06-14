#include "SimGameMode.h"
#include "SimCharacter.h"
#include "SimPlayerController.h"
#include "SimGameState.h"
#include "SimPlayerState.h"
#include "UObject/ConstructorHelpers.h"

ASimGameMode::ASimGameMode()
{
    DefaultPawnClass = ASimCharacter::StaticClass();
    PlayerControllerClass = ASimPlayerController::StaticClass();
    GameStateClass = ASimGameState::StaticClass();
    PlayerStateClass = ASimPlayerState::StaticClass();

    bUseSeamlessTravel = true;
}

void ASimGameMode::BeginPlay()
{
    Super::BeginPlay();
}

void ASimGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (NewPlayer)
    {
        ConnectedPlayers.AddUnique(NewPlayer);
    }
}

void ASimGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    if (Exiting)
    {
        ConnectedPlayers.Remove(Cast<APlayerController>(Exiting));
    }
}

AActor* ASimGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    return Super::ChoosePlayerStart_Implementation(Player);
}

void ASimGameMode::BroadcastChatMessage(const FString& SenderName, const FString& Message)
{
    for (APlayerController* PC : ConnectedPlayers)
    {
        if (PC)
        {
            PC->ClientMessage(FString::Printf(TEXT("[%s]: %s"), *SenderName, *Message));
        }
    }
}
