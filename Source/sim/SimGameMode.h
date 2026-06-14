#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SimGameMode.generated.h"

UCLASS()
class SIM_API ASimGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASimGameMode();

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION(BlueprintCallable, Category = "Game")
    int32 GetConnectedPlayerCount() const { return ConnectedPlayers.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Game")
    void BroadcastChatMessage(const FString& SenderName, const FString& Message);

protected:
    UPROPERTY()
    TArray<APlayerController*> ConnectedPlayers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    TSubclassOf<APawn> SimCharacterClass;
};
