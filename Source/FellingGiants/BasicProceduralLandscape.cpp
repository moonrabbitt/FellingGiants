// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicProceduralLandscape.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
ABasicProceduralLandscape::ABasicProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	ProceduralMesh->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void ABasicProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ABasicProceduralLandscape::OnConstruction(const FTransform& Transform)
{	//No longer need to run game, object loaded on construction script which is exectuted before game begins
	//Clear old variables on construction
	Vertices.Reset();
	Triangles.Reset();
	UV0.Reset();

	CreateVertices();
	CreateTriangles();

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, TArray<FColor>(), Tangents, true);
	ProceduralMesh->SetMaterial(0, Material);
	
}

// Called every frame
void ABasicProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABasicProceduralLandscape::CreateVertices()
{

	if (HeightmapTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Heightmap texture is null"));
		return;
	}

	//Adapt height map size
	float TextureSizeX = HeightmapTexture->GetSizeX();
	float TextureSizeY = HeightmapTexture->GetSizeY();

	// Fetch the underlying resource data of the texture. This is a low-level operation and should be done with care.
	FTexturePlatformData* TexData = HeightmapTexture->GetPlatformData();
	FTexture2DMipMap* MipMap = &TexData->Mips[0];
	FByteBulkData* RawImageData = &MipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
	int TextureWidth = MipMap->SizeX, TextureHeight = MipMap->SizeY;
	
	for (int X = 0; X <= XSize; ++X)
	{
		for (int Y = 0; Y <= YSize; ++Y)
		{
			int TextureX = FMath::Clamp(int(X * (TextureSizeX / XSize)), 0, TextureSizeX - 1);
			int TextureY = FMath::Clamp(int(Y * (TextureSizeY / YSize)), 0, TextureSizeY - 1);

			FColor PixelColor = FormatedImageData[TextureY * TextureWidth + TextureX];

			float Z = PixelColor.R / 255.0f * ZMultiplier;// using R channel from heightmap
			
			//GEngine->AddOnScreenDebugMessage(-1,999.0f,FColor::Yellow,FString::Printf(TEXT("Z %f"),Z));
			Vertices.Add(FVector(X * Scale, Y * Scale, Z ));
			UV0.Add(FVector2D(X * UVScale, Y * UVScale));
		}
	}
	// Unlock the mip level data now that we're done with it. It's important to always unlock any data you've locked as soon as you're done with it.
	RawImageData->Unlock();
}

void ABasicProceduralLandscape::CreateTriangles()
{
	int Vertex = 0;

	for (int X = 0; X < XSize; ++X)
	{
		for (int Y = 0; Y < YSize; ++Y)
		{
			Triangles.Add(Vertex);//Bottom left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + YSize + 2);//Top right corner
			Triangles.Add(Vertex + YSize + 1);//Top left corner

			++Vertex;
		}
		++Vertex;
	}
}