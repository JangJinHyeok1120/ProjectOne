// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProjectOneCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "ProjectOne/Environments/GameModes/ProjectOneGameInstance.h"
#include "POComponents/PlayerStatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Widget.h"
#include "Components/InputComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "PaperSpriteComponent.h"
#include "AI/AICharacter.h"
#include "LauncherCharacter.h"
#include "Weapons/SuperGun.h"
#include "POComponents/InventoryComponent.h"
#include "POComponents/POPlayerAkComponent.h"
#include "Environments/GameModes/ProjectOneGameState.h" 

void AProjectOneCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	SetComponents();
}

// *** 리소스 로드 (초기화) ***
void AProjectOneCharacter::SetResources() {
}

void AProjectOneCharacter::SetComponents()
{
	APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());
}



AProjectOneCharacter::AProjectOneCharacter()
{
	SetResources();
	GetCapsuleComponent()->InitCapsuleSize(25.0f, 30.0f);

	APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;


	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->ProbeSize = 20.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FollowCamera->SetRelativeLocation(FVector(-50.0f, 0, 0));

	CameraBoom->TargetArmLength = 350.0f;

	CameraBoom->SocketOffset = FVector(240.0f, 40.0f, 65.0f);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	Inventory->SetupAttachment(RootComponent);
	Inventory->SetRelativeLocation(FVector(0.0f));

	InputVector = FVector::ZeroVector;

	CharacterStat = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("CharcterStat"));
	CharacterStat->CurHP = 200.0f;
	CharacterStat->SetCharacterID(1);


	IsAlive = true;
	DeadTime = 3;
	CurLevel = 0;
	SheildEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SHEILDEFFECT"));
	SheildEffect->SetupAttachment(RootComponent);

	HealEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HEALEFFECT"));
	HealEffect->SetupAttachment(RootComponent);

	Gun = CreateDefaultSubobject<USuperGun>(TEXT("Gun"));

	SoundManager = CreateDefaultSubobject<UPOPlayerAkComponent>(TEXT("Sound"));
	SoundManager->SetupAttachment(RootComponent);

	isPause = false;
}

void AProjectOneCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Shot", IE_Pressed, this, &AProjectOneCharacter::Shot);
	PlayerInputComponent->BindAction("Shot", IE_Released, this, &AProjectOneCharacter::ShotRelease);
	PlayerInputComponent->BindAction("Alt", IE_Pressed, this, &AProjectOneCharacter::CameraRotateInput);
	PlayerInputComponent->BindAction("Alt", IE_Released, this, &AProjectOneCharacter::CameraRotateRelease);

	PlayerInputComponent->BindAction("ReLoad", IE_Pressed, this, &AProjectOneCharacter::ReloadAnimPlay);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AProjectOneCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AProjectOneCharacter::AimRelease);


	PlayerInputComponent->BindAction("Item1", IE_Pressed, this, &AProjectOneCharacter::UseItem1);
	PlayerInputComponent->BindAction("Item2", IE_Pressed, this, &AProjectOneCharacter::UseItem2);
	PlayerInputComponent->BindAction("Item3", IE_Pressed, this, &AProjectOneCharacter::UseItem3);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AProjectOneCharacter::Roll);
	PlayerInputComponent->BindAction("Evolution", IE_Pressed, this, &AProjectOneCharacter::Evolution);


	PlayerInputComponent->BindAction("MoveInput", IE_Released, this, &AProjectOneCharacter::MoveReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectOneCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectOneCharacter::MoveRight);


	//PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProjectOneCharacter::TurnAtRate);
//	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectOneCharacter::LookUpAtRate);

}


void AProjectOneCharacter::Tick(float delta) {
	Super::Tick(delta);

	switch (AimState) 
	{
	case AIM::IDLE:
		GetFollowCamera()->SetFieldOfView(FMath::Lerp(GetFollowCamera()->FieldOfView, 100.0f, 0.1f));
		GetCameraBoom()->TargetArmLength = 350;
		GetMesh()->SetVisibility(true);
		isAim = false;
		break;
	case AIM::HALF_AIM:
		GetFollowCamera()->SetFieldOfView(FMath::Lerp(GetFollowCamera()->FieldOfView, 60.0f, 0.1f));
		GetCameraBoom()->TargetArmLength = 350;
		GetMesh()->SetVisibility(true);
		isAim = false;
		break;
	case AIM::AIM:
		GetFollowCamera()->SetFieldOfView(FMath::Lerp(GetFollowCamera()->FieldOfView, 20.0f, 0.1f));
		GetCameraBoom()->TargetArmLength = 240;
		GetMesh()->SetVisibility(false);
		isAim = true;
		break;

	}

	Shooting(delta);
}

void AProjectOneCharacter::BeginPlay()
{
	Super::BeginPlay();

	AProjectOneGameState* GameState = Cast<AProjectOneGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (IsValid(GameState))
		GameState->AlivePlayers.Add(this);

}


void AProjectOneCharacter::PossessedBy(AController * ct)
{
	Super::PossessedBy(ct);

}

void AProjectOneCharacter::Shot()
{
	isShooting = true;
}

void AProjectOneCharacter::ShotRelease()
{
	isShooting = false;
	APAnim->isShooting = isShooting;
}

void AProjectOneCharacter::Roll()
{
	if (!APAnim->isRoll)
	{
		switch (rollDir)
		{
		case RollDir::RIGHT:
			APAnim->Montage_Play(APAnim->R_RollMontage);
			APAnim->isRoll = true;
			break;
		case RollDir::LEFT:
			APAnim->Montage_Play(APAnim->L_RollMontage);
			APAnim->isRoll = true;
			break;
		}
		InputVector = FVector::ZeroVector;
	}
}

void AProjectOneCharacter::Aim()
{
	switch (AimState) {
	case AIM::IDLE:
		AimState = AIM::HALF_AIM;
		break;
	}
	//isAim = true;
	//GetCharacterMovement()->bUseControllerDesiredRotation = true;
	//GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AProjectOneCharacter::AimRelease()
{
	switch (AimState) {
	case AIM::HALF_AIM:
		AimState = AIM::IDLE;
		break;
	}
	//isAim = false;
	//GetCharacterMovement()->bUseControllerDesiredRotation = true;
	//GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AProjectOneCharacter::ReloadAnimPlay()
{
	if (APAnim->isRoll ||
		APAnim->Montage_IsPlaying(APAnim->ReloadMontage))
		return;
	APAnim->Montage_Play(APAnim->ReloadMontage);
}

void AProjectOneCharacter::MoveReleased()
{
	//GetCharacterMovement()->bUseControllerDesiredRotation = true;
	//GetCharacterMovement()->bOrientRotationToMovement = false;
	rollDir = RollDir::RELEASE;
}


void AProjectOneCharacter::Shooting(float tick)
{
	//자동 재장전
	if (Gun->GetCurBullet() <= 0) {
		ReloadAnimPlay();
	}

	if (!isShooting || APAnim->Montage_IsPlaying(APAnim->ReloadMontage) || !IsAlive)
		return;


	Gun->Fire(GetMesh()->GetSocketLocation(TEXT("Muzzle")), false);
}

void AProjectOneCharacter::ReLoad()
{
	Gun->Reload();
}

void AProjectOneCharacter::UseItem1()
{
	Inventory->UseItem(0);
}

void AProjectOneCharacter::UseItem2()
{
	Inventory->UseItem(1);
}

void AProjectOneCharacter::UseItem3()
{
	Inventory->UseItem(2);
}

void AProjectOneCharacter::CameraRotateInput()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AProjectOneCharacter::CameraRotateRelease()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}


void AProjectOneCharacter::Win()
{
}

void AProjectOneCharacter::Lose()
{
}

void AProjectOneCharacter::Hit(float Damage, bool isHead, AActor * Causer)
{
	/*if (!Causer->IsA(ALauncherCharacter::StaticClass()) && Causer == this)
		return;*/
	if (isPause)
		return;
	//데미지계산식나중에 넣을것
	if (CharacterStat->CurHP - CalcDamage(Damage, isHead) <= 0) {
		CharacterStat->CurHP = 0;
		auto CauserPlayer = Cast<AProjectOneCharacter>(Causer);

		if (IsAlive && CauserPlayer)
		{
			CauserPlayer->Evolution();
			CauserPlayer->KillCount++;
		}

		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
		GetMesh()->SetCollisionProfileName(TEXT("BlockAll"));
		GetMesh()->SetSimulatePhysics(true);

		if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0) == Causer)
		{
			Cast<AProjectOneCharacter>(Causer)->isKillOtherPlayer = true;
		}


		if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0) == this)
		{
			DeadTime = 1000;

		}
		GetWorld()->GetTimerManager().SetTimer(DeadTimer, this, &AProjectOneCharacter::Dead, 1.0f, true);

		auto AI = Cast<AAICharacter>(Causer);
		if (AI)
			AI->TargetPlayer = NULL;


		AProjectOneGameState* GameState = Cast<AProjectOneGameState>(UGameplayStatics::GetGameState(GetWorld()));
		GameState->AlivePlayers.Remove(this);
		Rank = GameState->AlivePlayers.Num();
		IsAlive = false;

		//자기장에 사망했을 때
		auto ElectricField = Cast<AProjectOneGameState>(Causer);
		if (ElectricField)
		{
			// To do Something...
		}
	}
	else {

		if (!Causer)
			return;

		if (!Causer->IsA(AProjectOneGameState::StaticClass()))
		{
			isHit = true;
			LastHitByChracter = Causer;
		}

		CharacterStat->CurHP = CharacterStat->CurHP - CalcDamage(Damage, isHead);


		if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0) == Causer
			&& Causer != this) {
			FName HitStr = "IsHit";

			AHUD* HUD = Cast<AHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
			UBoolProperty* isHit = FindField<UBoolProperty>(HUD->GetClass(), HitStr);

			if (HPBarWidgetComponent->GetUserWidgetObject())
				HPBarWidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Visible);

			if (isHit != NULL) {
				isHit->SetPropertyValue_InContainer(HUD, true);
			}

		}
		if (CurLevel < 2) 
		{
			SoundManager->SetAkSwitch(TEXT("Hit"),TEXT("Grade_1"));
		}
		else if (CurLevel < 6) 
		{
			SoundManager->SetAkSwitch(TEXT("Hit"), TEXT("Grade_2"));
		}
		else
		{
			SoundManager->SetAkSwitch(TEXT("Hit"), TEXT("Grade_3"));
		}
			SoundManager->PlayBulletCrashSound();

		//플레이어가 유저이고 자기장에 데미지를 입었을 때
	/*	auto ElectricField = Cast<AProjectOneGameState>(Causer);
		if (ElectricField && UGameplayStatics::GetPlayerPawn(GetWorld(), 0) == Causer)
		{
			ABLOG_S(Warning);
			FName CauserActor = "CausedActor";
			AHUD* HUD = Cast<AHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
			UObjectProperty* CausedAct = FindField<UObjectProperty>(HUD->GetClass(), CauserActor);

			if (CausedAct != NULL) {

				CausedAct->SetPropertyValue_InContainer(HUD, Causer);
			}


		}*/
	}
	

}

float AProjectOneCharacter::CalcDamage(float Damage, bool isHead)
{
	float reVal = Damage - (Damage * (CharacterStat->CurAP / 100));
	if (isHead)
		reVal *= 1.25f;
	return reVal;
}


void AProjectOneCharacter::PlayCShake(int Index) {
	GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(CShakeList[Index]);
}


void AProjectOneCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (ControlAllowed)
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()*ControlAllowed);
	else
		return;

}

void AProjectOneCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (ControlAllowed)
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()*ControlAllowed);
	else
		return;
}

void AProjectOneCharacter::MoveForward(float Value)
{
	if (isPause)
		return;

	if (Value) {
		//GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//GetCharacterMovement()->bOrientRotationToMovement = false;

	}
	//ABLOG(Warning,TEXT("Control:%d, val:%d, isroll:%d, "), (Controller != NULL), Value != 0.0f, !APAnim->isRoll)
	if ((Controller != NULL) && (Value != 0.0f) && !APAnim->isRoll)
	{

		InputVector.X = Value;
		APAnim->Forward = Value * 100.0f;
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

	}
}

void AProjectOneCharacter::MoveRight(float Value)
{
	if (isPause)
		return;

	if (Value) {
		//GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//GetCharacterMovement()->bOrientRotationToMovement = false;
		if (Value == 1.0f)
			rollDir = RollDir::RIGHT;
		else if (Value == -1.0f)
			rollDir = RollDir::LEFT;
	}
	if ((Controller != NULL) && (Value != 0.0f) && !APAnim->isRoll)
	{
		InputVector.Y = Value;

		APAnim->Right = Value * 100.0f;

		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AProjectOneCharacter::Dead()
{
	DeadTime--;

	if (DeadTime < 1)
	{
		GetWorldTimerManager().ClearTimer(DeadTimer);
		Destroy(this);
	}
}

void AProjectOneCharacter::Evolution()
{
	CharacterStat->CurHP = CharacterStat->GetCharacterStatData()->HP;

	if (CurLevel > 5)
		return;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EvolEffect->Template, GetMesh()->GetComponentLocation());
	switch (CurLevel)
	{
	case 0:
		GetMesh()->SetRelativeScale3D(FVector(1.0f));
		CameraBoom->SocketOffset = FVector(190.0f, 45.0f, 80.0f);
		GetCapsuleComponent()->SetCapsuleSize(25.0f, 45.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -48.0f));
		break;
	case 1:
		GetMesh()->SetRelativeScale3D(FVector(1.2f));
		CameraBoom->SocketOffset = FVector(160.0f, 50.0f, 80.0f);
		GetCapsuleComponent()->SetCapsuleSize(25.0f, 60.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -63.0f));
		break;
	case 2:
		GetMesh()->SetRelativeScale3D(FVector(1.5f));
		CameraBoom->SocketOffset = FVector(140.0f, 60.0f, 110.0f);

		GetMesh()->SetSkeletalMesh(SecondSkMesh);
		GetMesh()->SetAnimInstanceClass(SecondAnimIns);
		APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());

		GetCapsuleComponent()->SetCapsuleSize(45.0f, 90.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -93.0f));
		ChangeWeapon();

		break;
	case 3:
		GetMesh()->SetRelativeScale3D(FVector(1.6f));
		CameraBoom->SocketOffset = FVector(140.0f, 65.0f, 110.0f);
		GetCapsuleComponent()->SetCapsuleSize(45.0f, 95.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -98.0f));
		break;
	case 4:
		GetMesh()->SetRelativeScale3D(FVector(1.7f));
		CameraBoom->SocketOffset = FVector(140.0f, 75.0f, 110.0f);
		GetCapsuleComponent()->SetCapsuleSize(45.0f, 103.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -100.0f));
		break;
	case 5:
		GetMesh()->SetRelativeScale3D(FVector(1.0f));
		CameraBoom->SocketOffset = FVector(100.0f, 90.0f, 130.0f);
		GetMesh()->SetSkeletalMesh(ThirdMesh);
		GetMesh()->SetAnimInstanceClass(ThirdAnimIns);
		APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());
		GetMesh()->SetRelativeScale3D(FVector(0.15f));

		GetCapsuleComponent()->SetCapsuleSize(45.0f, 105.0f);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -100.0f));

		//APAnim->ShotMontageCurrent = APAnim->ShotMontageThird;
		break;
	default:
		break;
	}
	++CurLevel;
	SoundManager->PlayEvolutionSound();
	CharacterStat->SetCharacterID(CurLevel + (int)PlayerType);
	CharacterStat->SetWeaponeID(CurLevel + (int)PlayerType);
	CharacterStat->CurHP = CharacterStat->GetCharacterStatData()->HP;
	CharacterStat->CurAP = CharacterStat->GetCharacterStatData()->AP;
}
