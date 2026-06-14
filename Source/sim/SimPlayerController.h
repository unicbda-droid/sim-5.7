#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimPlayerController.generated.h"

UCLASS()
class SIM_API ASimPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASimPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnRep_PlayerState() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowBuildModeUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideBuildModeUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowInteractionUI();

    UFUNCTION(Client, Reliable, Category = "Chat")
    void ClientReceiveChatMessage(const FString& SenderName, const FString& Message);

    UFUNCTION(Server, Reliable, WithValidation, Category = "Chat")
    void ServerSendChatMessage(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraModeById(uint8 Mode);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* ToggleBuildModeAction;

    void OnMove(const struct FInputActionValue& Value);
    void OnLook(const FInputActionValue& Value);
    void OnInteract();
    void OnToggleBuildMode();
};
