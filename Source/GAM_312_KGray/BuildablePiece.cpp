#include "BuildablePiece.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABuildablePiece::ABuildablePiece()
{
	// [Week 3] Building pieces are static structures once placed, so I disabled Tick to save on performance.
	PrimaryActorTick.bCanEverTick = false;

	// [Week 3] I initialize the visual mesh component and set it as the root of the object.
	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
	RootComponent = PieceMesh;

	// [Week 3] Initializing default material costs here. These get overridden in the individual Blueprints for walls, floors, etc.
	WoodCost = 5;
	StoneCost = 0;

	// [Week 3] Defaulting to wall so it has a baseline type.
	PieceType = EPieceType::Wall;
}