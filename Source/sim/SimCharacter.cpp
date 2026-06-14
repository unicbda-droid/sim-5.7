#include "SimCharacter.h"
#include "SimNeedsComponent.h"
#include "SimInteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"

ASimCharacter::ASimCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentCameraMode(ECameraMode::FirstPerson)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetMesh(), "head");
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 10.f, 0.f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    BuildModeSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("BuildModeSpringArm"));
    BuildModeSpringArm->SetupAttachment(RootComponent);
    BuildModeSpringArm->TargetArmLength = 1200.f;
    BuildModeSpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    BuildModeSpringArm->bUsePawnControlRotation = false;
    BuildModeSpringArm->bInheritPitch = false;
    BuildModeSpringArm->bInheritYaw = true;
    BuildModeSpringArm->bInheritRoll = false;

    BuildModeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BuildModeCamera"));
    BuildModeCamera->SetupAttachment(BuildModeSpringArm);

    NeedsComponent = CreateDefaultSubobject<USimNeedsComponent>(TEXT("NeedsComponent"));

    InteractionComponent = CreateDefaultSubobject<USimInteractionComponent>(TEXT("InteractionComponent"));
}

void ASimCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateCamera();
}

void ASimCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASimCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Will bind actions once InputMappingContext is set up
    }
}

void ASimCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ASimCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ASimCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ASimCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ASimCharacter::Interact()
{
    if (InteractionComponent)
    {
        InteractionComponent->Interact();
    }
}

void ASimCharacter::ToggleBuildMode()
{
    if (CurrentCameraMode == ECameraMode::FirstPerson)
    {
        SwitchCameraMode(ECameraMode::BuildMode);
    }
    else
    {
        SwitchCameraMode(ECameraMode::FirstPerson);
    }
}

void ASimCharacter::SwitchCameraMode(ECameraMode NewMode)
{
    CurrentCameraMode = NewMode;
    UpdateCamera();
}

void ASimCharacter::UpdateCamera()
{
    FirstPersonCamera->SetActive(CurrentCameraMode == ECameraMode::FirstPerson);
    BuildModeCamera->SetActive(CurrentCameraMode == ECameraMode::BuildMode);

    if (Controller)
    {
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            if (CurrentCameraMode == ECameraMode::FirstPerson)
            {
                PC->SetViewTargetWithBlend(this, 0.5f);
            }
        }
    }
}

void ASimCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ASimCharacter, CurrentCameraMode, COND_SkipOwner);
}
