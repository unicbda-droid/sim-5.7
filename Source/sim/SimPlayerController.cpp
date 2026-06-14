#include "SimPlayerController.h"
#include "SimCharacter.h"
#include "SimPlayerState.h"
#include "SimGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

ASimPlayerController::ASimPlayerController()
{
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void ASimPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ASimPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASimPlayerController::OnMove);
        }
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASimPlayerController::OnLook);
        }
        if (InteractAction)
        {
            EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASimPlayerController::OnInteract);
        }
        if (ToggleBuildModeAction)
        {
            EnhancedInput->BindAction(ToggleBuildModeAction, ETriggerEvent::Started, this, &ASimPlayerController::OnToggleBuildMode);
        }
    }
}

void ASimPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
}

void ASimPlayerController::OnMove(const FInputActionValue& Value)
{
    if (ASimCharacter* SimChar = Cast<ASimCharacter>(GetPawn()))
    {
        const FVector2D MoveVector = Value.Get<FVector2D>();
        SimChar->MoveForward(MoveVector.Y);
        SimChar->MoveRight(MoveVector.X);
    }
}

void ASimPlayerController::OnLook(const FInputActionValue& Value)
{
    if (ASimCharacter* SimChar = Cast<ASimCharacter>(GetPawn()))
    {
        const FVector2D LookVector = Value.Get<FVector2D>();
        SimChar->Turn(LookVector.X);
        SimChar->LookUp(LookVector.Y);
    }
}

void ASimPlayerController::OnInteract()
{
    if (ASimCharacter* SimChar = Cast<ASimCharacter>(GetPawn()))
    {
        SimChar->Interact();
    }
}

void ASimPlayerController::OnToggleBuildMode()
{
    if (ASimCharacter* SimChar = Cast<ASimCharacter>(GetPawn()))
    {
        SimChar->ToggleBuildMode();
    }
}

void ASimPlayerController::ShowBuildModeUI()
{
    // Placeholder for build mode UI
}

void ASimPlayerController::HideBuildModeUI()
{
    // Placeholder for build mode UI
}

void ASimPlayerController::ShowInteractionUI()
{
    // Placeholder for interaction UI
}

void ASimPlayerController::ClientReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    // Will show chat message in UI
}

void ASimPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
    if (ASimPlayerState* PS = GetPlayerState<ASimPlayerState>())
    {
        FString SenderName = PS->CharacterName.ToString();
        if (ASimGameMode* GM = GetWorld()->GetAuthGameMode<ASimGameMode>())
        {
            GM->BroadcastChatMessage(SenderName, Message);
        }
    }
}

bool ASimPlayerController::ServerSendChatMessage_Validate(const FString& Message)
{
    return Message.Len() <= 256;
}

void ASimPlayerController::SetCameraModeById(uint8 Mode)
{
    if (ASimCharacter* SimChar = Cast<ASimCharacter>(GetPawn()))
    {
        SimChar->SwitchCameraMode(static_cast<ECameraMode>(Mode));
    }
}
