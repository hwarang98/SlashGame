// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "interface/HitInterface.h"
#include "NiagaraComponent.h"


AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	/* 충돌콜리전 활성화 */
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/* 모든채널에 대해 오버랩  */
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	/* 모든 채널에 오버랩 후 단일 채널에 충돌 설정 */
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
	BoxTraceStart = CreateDefaultSubobject<USphereComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	
	BoxTraceEnd = CreateDefaultSubobject<USphereComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::AttackMeshToSocket(USceneComponent* InParent, FName InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
	/* 사운드 재생 */
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	// 무기(Weapon)의 소유자(Owner)를 지정
	SetOwner(NewOwner);
	// 행위를 유발한 주체(Instigator)를 설정 즉, 공격자
	SetInstigator(NewInstigator);
	AttackMeshToSocket(InParent, InSocketName);
	ItemState = EItemState::EIS_Equipped;

	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (EmbersEffect)
	{
		EmbersEffect->Deactivate();
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	/**
	 * TODO
	 * 무기상자를 가져온다
	 * 구성요소에 엑세스 한다
	 * 중첩을 시작하고 동적 추가한다.
	 *
	 * 박스 컴포넌틔의 겹침 이벤트에 대한 응답으로 해당 콜백을 가져와야함
	 */
	
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

/* 박스 트레이스 수행 */
void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	for (AActor* Actor : ActorsToIgnore)
	{
		ActorsToIgnore.Remove(Actor);
	}
	
	FHitResult BoxHit;
	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		FVector(5.f, 5.f, 5.f),
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		BoxHit,
		true
	);
	/* 적중된 액터 반환 */
	if (BoxHit.GetActor())
	{
		UGameplayStatics::ApplyDamage(
			BoxHit.GetActor(),
			Damage,
			GetInstigator()->GetController(), // 공격을 유발한 주체의 컨트롤러 즉, 공격자
			this,
			UDamageType::StaticClass() // 기본 데미지 타입
			);
		/* 액터가 IHitInterface를 구현하고 있다면 캐스팅하여 인터페이스 포인터 반환 없으면 nullptr */
		IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
		if (HitInterface)
		{
			/* 인터페이스의 GetHit 함수 호출 (예: 피격 반응 처리 등) */
			HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint);
		}
		IgnoreActors.AddUnique(BoxHit.GetActor());

		CreateFields(BoxHit.ImpactPoint);

	}
}
