#include "MainPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h" 
#include "InteractableItem.h"

// --- CUSTOM UI: INCLUDE WIDGET MODULE ---
#include "Blueprint/UserWidget.h"

// Sets default values
AMainPlayer::AMainPlayer()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// --- WEEK 1: CAMERA SETUP ---
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// --- WEEK 2: INITIALIZE STATS & INVENTORY ---
	Health = 100.0f;
	Hunger = 100.0f;
	Stamina = 100.0f;

	Wood = 0;
	Stone = 0;
	Berry = 0;
}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();

	// --- WEEK 2: START STAT TIMER ---
	GetWorldTimerManager().SetTimer(StatTimerHandle, this, &AMainPlayer::HandleStatsOverTime, 1.0f, true);

	// --- CUSTOM UI: SPAWN THE WIDGET ---
	// Check if we assigned a UI in the Unreal Editor
	if (StatsWidgetClass)
	{
		// Create the widget and add it to the player's screen
		StatsWidget = CreateWidget<UUserWidget>(GetWorld(), StatsWidgetClass);
		if (StatsWidget)
		{
			StatsWidget->AddToViewport();
		}
	}
}

// Called every frame
void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainPlayer::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMainPlayer::Interact);
}

void AMainPlayer::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMainPlayer::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMainPlayer::HandleStatsOverTime()
{
	if (Hunger > 0.0f)
	{
		Hunger -= 1.0f;
	}
	else
	{
		Health -= 2.0f;
	}

	if (Stamina < 100.0f)
	{
		Stamina += 1.0f;
	}
}

// --- WEEK 2: LINE TRACE & GATHER LOGIC ---
void AMainPlayer::Interact()
{
	if (!FirstPersonCameraComponent) return;

	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * 500.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

	FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, LineColor, false, 2.0f, 0, 2.0f);

	if (bHit)
	{
		if (Stamina > 0.0f)
		{
			Stamina -= 5.0f;
		}

		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			AInteractableItem* HitItem = Cast<AInteractableItem>(HitActor);

			if (HitItem)
			{
				EResourceType GatheredType;

				if (HitItem->GatherResource(GatheredType))
				{
					switch (GatheredType)
					{
					case EResourceType::Wood:
						Wood++;
						ShowResourcePopup(GatheredType, 1); // <--- NEW TRIGGER CALL
						UE_LOG(LogTemp, Warning, TEXT("Gathered Wood! Total: %d"), Wood);
						break;
					case EResourceType::Stone:
						Stone++;
						ShowResourcePopup(GatheredType, 1); // <--- NEW TRIGGER CALL
						UE_LOG(LogTemp, Warning, TEXT("Gathered Stone! Total: %d"), Stone);
						break;
					case EResourceType::Berry:
						Berry++;
						ShowResourcePopup(GatheredType, 1); // <--- NEW TRIGGER CALL
						UE_LOG(LogTemp, Warning, TEXT("Gathered Berry! Total: %d"), Berry);
						break;
					}
				}
			}
		}
	}
}