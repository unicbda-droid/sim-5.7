#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimNeedsTypes.h"
#include "SimActionQueueComponent.generated.h"

UENUM(BlueprintType)
enum class EActionPriority : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Low         UMETA(DisplayName = "Low"),
    UserDriven  UMETA(DisplayName = "User Driven"),
    Critical    UMETA(DisplayName = "Critical"),
    MAX         UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSimAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FText ActionName;

    UPROPERTY(BlueprintReadWrite)
    float Duration;

    UPROPERTY(BlueprintReadWrite)
    EActionPriority Priority;

    UPROPERTY(BlueprintReadWrite)
    TArray<FNeedModifier> NeedModifiers;

    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(BlueprintReadWrite)
    bool bInterruptable;

    FSimAction()
        : Duration(0.f)
        , Priority(EActionPriority::Idle)
        , TargetActor(nullptr)
        , bInterruptable(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStarted, const FSimAction&, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionCompleted, const FSimAction&, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionInterrupted, const FSimAction&, Action);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIM_API USimActionQueueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USimActionQueueComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void EnqueueAction(FSimAction Action);

    UFUNCTION(BlueprintCallable, Category = "Actions")
    bool DequeueAction(FSimAction& OutAction);

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void ClearQueue();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void InterruptCurrent(EActionPriority NewPriority);

    UFUNCTION(BlueprintPure, Category = "Actions")
    bool IsBusy() const { return bIsExecuting; }

    UFUNCTION(BlueprintPure, Category = "Actions")
    FSimAction GetCurrentAction() const { return CurrentAction; }

    UFUNCTION(BlueprintPure, Category = "Actions")
    int32 GetQueueSize() const { return ActionQueue.Num(); }

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Actions")
    FOnActionStarted OnActionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Actions")
    FOnActionCompleted OnActionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Actions")
    FOnActionInterrupted OnActionInterrupted;

protected:
    UPROPERTY(Replicated)
    TArray<FSimAction> ActionQueue;

    UPROPERTY(Replicated)
    FSimAction CurrentAction;

    UPROPERTY(Replicated)
    float ActionProgress;

    UPROPERTY(Replicated)
    bool bIsExecuting;

    void ExecuteCurrentAction();
    void CompleteCurrentAction();
    void AdvanceToNextAction();

    class ASimCharacter* OwnerCharacter;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
