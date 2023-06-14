// Fill out your copyright notice in the Description page of Project Settings.
//https://github.com/Fetiska/Unreal-Procedural-Terrain-Generation-CPP/blob/main/%232%20Landscape%20generation/Source/DiamondSquareCPP/Public/DiamondSquare.h

#pragma once

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "BasicProceduralLandscape.generated.h"


class UProceduralMeshComponent;
class UMaterialInterface;
class UTextureRenderTarget2D;

UCLASS()
class FELLINGGIANTS_API ABasicProceduralLandscape : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasicProceduralLandscape();

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	int XSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	int YSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	float ZMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
	float NoiseScale = 0.1f;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.000001))
	float Scale = 100.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.000001))
	float UVScale = 0.0f;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1))
	float MinGrassHeight;
	
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 1))
	float MaxGrassHeight;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	
	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* SceneCaptureComponent;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, Category = "Components")
	UInstancedStaticMeshComponent* GrassMeshComponent;

	UPROPERTY(EditAnywhere)
	int32 GrassDensity = 1;  // number of grass instances per triangle

	UPROPERTY(EditAnywhere)
	float GrassSize = 1.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UProceduralMeshComponent* ProceduralMesh;
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> Normals;
	TArray<struct  FProcMeshTangent> Tangents;

	void CreateVertices();
	void CreateVerticesWithoutHeightMap();
	void CreateTriangles();
	void SpawnGrass();
	void UpdateGrassZPosition();
};
