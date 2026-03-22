#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableItem.h" 
#include "BuildablePiece.h" 
#include "MainPlayer.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	EResourceType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Quantity;

	FInventoryItem() { ItemType = EResourceType::Wood; Quantity = 0; }
	FInventoryItem(EResourceType InType, int32 InQuantity) { ItemType = InType; Quantity = InQuantity; }
};

UCLASS()
class GAM_312_KGRAY_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AMainPlayer();

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	// Resources & Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Wood;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Stone;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Berry;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventoryItem> InventoryArray;

	void AddResourceToInventory(EResourceType GatheredType, int32 Amount);

	// NEW: Helper functions to check and consume specific building pieces
	bool HasBuildingItem(EPieceType Type);
	void ConsumeBuildingItem(EPieceType Type);

	void HandleStatsOverTime();
	FTimerHandle StatTimerHandle;
	void Interact();

	// UI
	void ToggleInventoryMenu();
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool bIsInventoryOpen;

	// Building System
	void ToggleBuildMenu();
	UPROPERTY(BlueprintReadWrite, Category = "Building")
	bool bIsBuildMenuOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UMaterialInterface* GhostMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> WallVariations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> FloorVariations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> RoofVariations;

	UPROPERTY(BlueprintReadWrite, Category = "Building")
	int32 EquippedPieceIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float SnapGridSize;

	UPROPERTY(BlueprintReadOnly, Category = "Building")
	bool bCanPlace;

	void SelectWall();
	void SelectFloor();
	void SelectRoof();
	void PlaceBuilding();
	void RotateBuilding();
	void CycleBuildingVariation();
	void UpdateBuildingPreview();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBuildUI(int32 PieceIndex, bool bHasResources);

	UPROPERTY()
	ABuildablePiece* PreviewPiece;

	float BuildRotationYaw;
	int32 CurrentVariationIndex;
	TArray<TSubclassOf<ABuildablePiece>> CurrentBuildingVariations;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> StatsWidgetClass;
	UPROPERTY()
	class UUserWidget* StatsWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ShowResourcePopup(EResourceType ResourceType, int32 Amount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> InventoryMenuClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	class UUserWidget* InventoryMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> BuildMenuClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	class UUserWidget* BuildMenuWidget;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void StartBuilding(TArray<TSubclassOf<ABuildablePiece>> Variations);
};