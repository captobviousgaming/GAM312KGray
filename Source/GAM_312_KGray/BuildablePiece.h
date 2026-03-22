#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildablePiece.generated.h"

// [Week 3] I created this enum so the building system knows exactly what type of piece it's handling. 
// This is super important for enforcing specific placement rules, like making sure a roof only snaps to a wall.
UENUM(BlueprintType)
enum class EPieceType : uint8
{
	Wall UMETA(DisplayName = "Wall"),
	Floor UMETA(DisplayName = "Floor"),
	Roof UMETA(DisplayName = "Roof")
};

UCLASS()
class GAM_312_KGRAY_API ABuildablePiece : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildablePiece();

protected:
	// [Week 3] The physical 3D model of the building piece that the player places in the world.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PieceMesh;

	// [Week 3] Setting up resource costs for crafting. 
	// I expose these to Blueprint so I can easily tweak the balancing later without recompiling C++.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 WoodCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 StoneCost;

	// [Week 3] This tags the piece as a Wall, Floor, or Roof.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Rules")
	EPieceType PieceType;

public:
	// [Week 3] Getter functions so my MainPlayer script can check if we have enough resources before placing.
	int32 GetWoodCost() const { return WoodCost; }
	int32 GetStoneCost() const { return StoneCost; }

	// [Week 3] Allows the player to ask the piece what type it is during the line trace.
	EPieceType GetPieceType() const { return PieceType; }

	// [Extra Polish] Exposing the mesh so the Player script can swap its material to a translucent ghost silhouette before placement!
	class UStaticMeshComponent* GetPieceMesh() const { return PieceMesh; }
};