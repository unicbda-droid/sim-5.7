#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SimPlayerState.generated.h"

UCLASS()
class SIM_API ASimPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASimPlayerState();

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
    int32 Simoleons;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
    int32 SkillPoints;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void AddSimoleons(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool SpendSimoleons(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void AddSkillPoints(int32 Amount);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
