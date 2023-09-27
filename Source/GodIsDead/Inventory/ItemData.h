#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum ItemType
{
	Material,
	Sword,
	Shield
};

UCLASS()
class GODISDEAD_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ItemValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USceneCaptureComponent2D* ItemScreenshot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ItemType> ItemType;
};
