#pragma once

#include "CoreMinimal.h"
#include "SimNeedsTypes.generated.h"

UENUM(BlueprintType)
enum class ENeedsType : uint8
{
    Hunger  UMETA(DisplayName = "Hunger"),
    Bladder UMETA(DisplayName = "Bladder"),
    Energy  UMETA(DisplayName = "Energy"),
    Hygiene UMETA(DisplayName = "Hygiene"),
    Fun     UMETA(DisplayName = "Fun"),
    Social  UMETA(DisplayName = "Social"),
    MAX     UMETA(Hidden)
};
