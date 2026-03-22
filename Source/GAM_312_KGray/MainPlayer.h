#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableItem.h" 
#include "BuildablePiece.h" 
#include "MainPlayer.generated.h"

// [Extra Polish] A custom struct to handle a dynamic inventory system. 
// This lets me easily add or remove any item type without hardcoding a million integer variables.
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

	// [Week 1] Core movement functions tied to the WASD input axes.
	void MoveForward(float Value);
	void MoveRight(float Value);

	// [Week 1] The first-person camera component attached to the player capsule.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	// [Week 2] Player Stats setup. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	// [Week 2] Basic raw resource tracking.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Wood;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Stone;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Berry;

	// [Extra Polish] My dynamic array for the inventory system.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventoryItem> InventoryArray;

	// [Week 2] Adds gathered resources to the array.
	void AddResourceToInventory(EResourceType GatheredType, int32 Amount);

	// [Week 3] Helper functions to check if we have enough pre-made walls/floors/roofs in the inventory before building.
	bool HasBuildingItem(EPieceType Type);
	void ConsumeBuildingItem(EPieceType Type);

	// [Week 2] A timer function to slowly drain hunger and health over time.
	void HandleStatsOverTime();
	FTimerHandle StatTimerHandle;

	// [Week 2] The function that fires a trace collision to hit rocks and trees.
	void Interact();

	// [Extra Polish] Toggles the visibility of the player's backpack/inventory UI.
	void ToggleInventoryMenu();
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool bIsInventoryOpen;

	// --- [WEEK 3] BUILDING SYSTEM VARIABLES --- //

	// Toggles the build menu HUD on and off.
	void ToggleBuildMenu();
	UPROPERTY(BlueprintReadWrite, Category = "Building")
	bool bIsBuildMenuOpen;

	// [Extra Polish] The material applied to the preview piece to make it look like a hologram/ghost before placing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UMaterialInterface* GhostMaterial;

	// [Week 3] Arrays to hold the different blueprints we can spawn for our shelter.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> WallVariations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> FloorVariations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Categories")
	TArray<TSubclassOf<ABuildablePiece>> RoofVariations;

	// [Week 3] Tracks which category (1=Wall, 2=Floor, etc) the player is currently holding.
	UPROPERTY(BlueprintReadWrite, Category = "Building")
	int32 EquippedPieceIndex;

	// [Week 3] The mathematical size of the grid we snap pieces to. Exposed to BP for easy tuning!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float SnapGridSize;

	// [Week 3] A safety flag to ensure we don't place pieces inside of each other or in invalid spots.
	UPROPERTY(BlueprintReadOnly, Category = "Building")
	bool bCanPlace;

	// [Week 3] The core functions handling the building logic.
	void SelectWall();
	void SelectFloor();
	void SelectRoof();
	void PlaceBuilding();
	void RotateBuilding();
	void CycleBuildingVariation();
	void UpdateBuildingPreview();

	// [Week 3] This event talks directly to the WBP_BuildHUD to update the image icon dynamically!
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBuildUI(int32 PieceIndex, bool bHasResources);

	// [Week 3] A reference to the transparent piece currently floating in front of the player.
	UPROPERTY()
	ABuildablePiece* PreviewPiece;

	float BuildRotationYaw;
	int32 CurrentVariationIndex;
	TArray<TSubclassOf<ABuildablePiece>> CurrentBuildingVariations;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// [Week 3] Widget references for Stats and Menus.
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

	// [Week 3] Kicks off the placement system when a player chooses a category.
	UFUNCTION(BlueprintCallable, Category = "Building")
	void StartBuilding(TArray<TSubclassOf<ABuildablePiece>> Variations);
};