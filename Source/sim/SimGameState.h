#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SimGameState.generated.h"

UCLASS()
class SIM_API ASimGameState : public AGameState
{
    GENERATED_BODY()

public:
    ASimGameState();

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
    int32 GameDay;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
    float GameTimeOfDay;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
    float TimeScale;

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void AdvanceGameTime(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetTimeOfDayNormalized() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    const float DayLength = 86400.f;
};
