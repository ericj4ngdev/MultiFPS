// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "MultiFPSCharacter.h"
#include "MultiFPSProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MultiFPS.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	// static ConstructorHelpers::FObjectFinder<USkeletalMeshComponent> 
	// /Script/Engine.SkeletalMesh'/Game/FPWeapon/Mesh/SK_FPGun.SK_FPGun'


	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundRef(TEXT("/Script/Engine.SoundWave'/Game/FPWeapon/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02'"));
	if(FireSoundRef.Object)
	{
		FireSound = FireSoundRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> FireAnimationRef(TEXT("/Script/Engine.AnimMontage'/Game/FirstPersonArms/Animations/FP_Rifle_Shoot_Montage.FP_Rifle_Shoot_Montage'"));
	if(FireAnimationRef.Object)
	{
		FireAnimation = FireAnimationRef.Object;
	}
	
	/*static ConstructorHelpers::FObjectFinder<UInputMappingContext> FireMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/FirstPerson/Input/IMC_Weapons.IMC_Weapons'"));
	if(FireMappingContextRef.Object)
	{
		FireMappingContext = FireMappingContextRef.Object;
	}*/

	static ConstructorHelpers::FObjectFinder<UInputAction> FireActionRef(TEXT("/Script/EnhancedInput.InputAction'/Game/FirstPerson/Input/Actions/IA_Shoot.IA_Shoot'"));
	if(FireActionRef.Object)
	{
		FireAction = FireActionRef.Object;
	}
}

void UTP_WeaponComponent::InitializeComponent()
{
	SetIsReplicated(true);
	Super::InitializeComponent();
	
}

void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTP_WeaponComponent, Character);
}

void UTP_WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());

	// 발사 위치 계산
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

	if(!Character->HasAuthority())
	{
		OnFire(SpawnLocation, SpawnRotation);
	}
	
	ServerFire(SpawnLocation, SpawnRotation);
}

void UTP_WeaponComponent::OnFire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
	if(Character->HasAuthority())
	{
		CreateProjectile(SpawnLocation,SpawnRotation);
	}
	
	PlayEffect(SpawnLocation,SpawnRotation);
	PlaySound();
	PlayMontage();	
}

void UTP_WeaponComponent::CreateProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (ProjectileClass != nullptr)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AMultiFPSProjectile* Projectile = nullptr;
		// Spawn the projectile at the muzzle
		Projectile = GetWorld()->SpawnActor<AMultiFPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		if(Projectile) Projectile->SetOwner(Character);	
	}
}

void UTP_WeaponComponent::PlayEffect(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if(MuzzleFlashEffect != nullptr)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			MuzzleFlashEffect,
			SpawnLocation,
			SpawnRotation);
	}
}

void UTP_WeaponComponent::PlaySound()
{
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
}

void UTP_WeaponComponent::PlayMontage()
{
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::ServerFire_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
	MulticastFire(SpawnLocation, SpawnRotation);
}

void UTP_WeaponComponent::MulticastFire_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
	// 격발자, 서버 제외
	if(Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	OnFire(SpawnLocation, SpawnRotation);
}

void UTP_WeaponComponent::AttachWeapon(AMultiFPSCharacter* TargetCharacter)
{
	MF_SUBLOG(LogMFTemp, Warning, TEXT("Begin"))
	Character = TargetCharacter;
	// Owner는 총임
	AActor* OwnerActor = GetOwner();
	AActor* OwnerCharacter = OwnerActor->GetOwner();
	
	if(OwnerCharacter)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Owner : %s"), *OwnerCharacter->GetName())
	}
	else
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("%s"), TEXT("No Owner"))
	}

	OwnerCharacter = Character;	
	OwnerActor->SetOwner(Character);
	
	if(OwnerCharacter)
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("Owner : %s"), *OwnerCharacter->GetName())
	}
	else
	{
		MF_SUBLOG(LogMFTemp, Warning, TEXT("%s"), TEXT("No Owner"))
	}
	
	
	// Check that the character is valid, and has no rifle yet
	if (Character == nullptr || Character->GetHasRifle())
	{
		return;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	
	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);

	
	
	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
