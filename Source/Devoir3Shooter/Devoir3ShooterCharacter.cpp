// Copyright Epic Games, Inc. All Rights Reserved.

#include "Devoir3ShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Devoir3Shooter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

ADevoir3ShooterCharacter::ADevoir3ShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ADevoir3ShooterCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    bIsDead = false;
    BP_OnHealthChanged(CurrentHealth, MaxHealth);
}

float ADevoir3ShooterCharacter::GetHealthPercent() const
{
    return MaxHealth > 0.f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;
}

float ADevoir3ShooterCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead || DamageAmount <= 0.f)
    {
        return 0.f;
    }

    const float AppliedDamage = FMath::Min(CurrentHealth, DamageAmount);
    CurrentHealth = FMath::Max(0.f, CurrentHealth - AppliedDamage);
    BP_OnHealthChanged(CurrentHealth, MaxHealth);

    UE_LOG(LogDevoir3Shooter, Log, TEXT("%s health: %.1f / %.1f"),
        *GetName(), CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        HandleDeath();
    }
    return AppliedDamage;
}

void ADevoir3ShooterCharacter::HandleDeath()
{
    if (bIsDead) return;
    bIsDead = true;
    ControllerBeforeDeath = GetController();

    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorHiddenInGame(true);
    BP_OnDeath();

    // For now the automatic respawn is for the human player.
    // AI respawn will use team-specific spawn points later.
    if (Cast<APlayerController>(ControllerBeforeDeath.Get()))
    {
        GetWorldTimerManager().SetTimer(RespawnTimer, this,
            &ADevoir3ShooterCharacter::RespawnPlayer, RespawnDelay, false);
    }
}


void ADevoir3ShooterCharacter::RespawnPlayer()
{
	APlayerController* PC =
		Cast<APlayerController>(ControllerBeforeDeath.Get());

	AGameModeBase* GM =
		GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;

	if (!PC || !GM)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Respawn failed: missing PlayerController or GameMode"));
		return;
	}

	// Release the dead character
	PC->UnPossess();

	// Ask GameMode to spawn and possess a new character
	GM->RestartPlayer(PC);

	if (PC->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player respawn successful!"));

		// Remove the old character
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error,
			TEXT("Respawn failed: GameMode did not spawn a pawn"));
	}
}


void ADevoir3ShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Shooting
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ADevoir3ShooterCharacter::Fire);
		}

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADevoir3ShooterCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADevoir3ShooterCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADevoir3ShooterCharacter::Look);
	}
	else
	{
		UE_LOG(LogDevoir3Shooter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ADevoir3ShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ADevoir3ShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ADevoir3ShooterCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ADevoir3ShooterCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADevoir3ShooterCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ADevoir3ShooterCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ADevoir3ShooterCharacter::Fire()
{
	if (bIsDead || !GetWorld() || !FollowCamera)
	{
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * 10000.0f;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShooterFire), false, this);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	const FVector TraceEnd = bHit ? Hit.ImpactPoint : End;
	DrawDebugLine(GetWorld(), Start, TraceEnd, FColor::Red, false, 1.0f, 0, 1.5f);

	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyDamage(Hit.GetActor(), 20.0f, GetController(), this, nullptr);
		UE_LOG(LogDevoir3Shooter, Log, TEXT("Hit: %s"), *GetNameSafe(Hit.GetActor()));
	}
}
