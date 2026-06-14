#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SimCharacter.generated.h"

class USimNeedsComponent;
class USimInteractionComponent;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ECameraMode : uint8
{
    FirstPerson UMETA(DisplayName = "First Person"),
    BuildMode  UMETA(DisplayName = "Build Mode (Isometric)")
};

UCLASS()
class SIM_API ASimCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASimCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* BuildModeSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* BuildModeCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs")
    USimNeedsComponent* NeedsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    USimInteractionComponent* InteractionComponent;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SwitchCameraMode(ECameraMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    ECameraMode GetCurrentCameraMode() const { return CurrentCameraMode; }

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Turn(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Interact();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ToggleBuildMode();

protected:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Camera")
    ECameraMode CurrentCameraMode;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    void UpdateCamera();
};
