#include "TerminalServer.h"
#include "HttpServerModule.h"
#include "HttpPath.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h"
#include "Components/StaticMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"

#if WITH_EDITOR
#include "Editor.h"
#endif



DEFINE_LOG_CATEGORY_STATIC(LogTerminalServer, Log, All);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static TUniquePtr<FHttpServerResponse> JsonResponse(TSharedPtr<FJsonObject> Object)
{
	FString JsonStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
	FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
	TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(JsonStr, TEXT("application/json"));
	Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
	return Response;
}

static TUniquePtr<FHttpServerResponse> ErrorResponse(const FString& Message)
{
	TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject());
	Obj->SetStringField(TEXT("error"), Message);
	Obj->SetBoolField(TEXT("success"), false);
	return JsonResponse(Obj);
}

static TSharedPtr<FJsonObject> ParseBody(const FHttpServerRequest& Request)
{
	FString Body;
	FFileHelper::BufferToString(Body, Request.Body.GetData(), Request.Body.Num());
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		return Json;
	return nullptr;
}

static UWorld* GetEditorWorld()
{
#if WITH_EDITOR
	if (GEditor)
		return GEditor->GetEditorWorldContext().World();
#endif
	return nullptr;
}

// ---------------------------------------------------------------------------
// Route registration helper
// ---------------------------------------------------------------------------

static FHttpRouteHandle AddRoute(TSharedPtr<IHttpRouter> Router, TArray<FHttpRouteHandle>& OutHandles,
	const FString& Path, EHttpServerRequestVerbs Verbs,
	TFunction<TUniquePtr<FHttpServerResponse>(const FHttpServerRequest&)> Handler)
{
	FHttpRouteHandle Handle = Router->BindRoute(
		FHttpPath(Path), Verbs,
		FHttpRequestHandler::CreateLambda(
			[Handler = MoveTemp(Handler)](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
			{
				OnComplete(Handler(Request));
				return true;
			})
	);
	if (Handle.IsValid())
		OutHandles.Add(Handle);
	return Handle;
}

// ---------------------------------------------------------------------------
// Start / Stop
// ---------------------------------------------------------------------------

bool FTerminalServer::Start(int32 Port)
{
	ServerPort = Port;

	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpRouter = HttpServerModule.GetHttpRouter(ServerPort);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogTerminalServer, Error, TEXT("Failed to get HttpRouter for port %d"), ServerPort);
		return false;
	}

	// -- Status
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/status"), EHttpServerRequestVerbs::VERB_GET,
		[](const FHttpServerRequest&) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject());
			Obj->SetStringField(TEXT("status"), TEXT("running"));
			Obj->SetStringField(TEXT("plugin"), TEXT("UETerminalBridge"));
#if WITH_EDITOR
			Obj->SetBoolField(TEXT("editor"), true);
#endif
			return JsonResponse(Obj);
		});

	// -- List actors in level
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/actors"), EHttpServerRequestVerbs::VERB_GET,
		[](const FHttpServerRequest&) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			TArray<TSharedPtr<FJsonValue>> ActorsArr;
			UWorld* World = GetEditorWorld();
			if (World)
			{
				for (TActorIterator<AActor> It(World); It; ++It)
				{
					AActor* Actor = *It;
					TSharedPtr<FJsonObject> A = MakeShareable(new FJsonObject());
					A->SetStringField(TEXT("name"), Actor->GetName());
					A->SetStringField(TEXT("label"), Actor->GetActorLabel());
					A->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
					FVector Loc = Actor->GetActorLocation();
					TArray<TSharedPtr<FJsonValue>> L;
					L.Add(MakeShareable(new FJsonValueNumber(Loc.X)));
					L.Add(MakeShareable(new FJsonValueNumber(Loc.Y)));
					L.Add(MakeShareable(new FJsonValueNumber(Loc.Z)));
					A->SetArrayField(TEXT("location"), L);
					ActorsArr.Add(MakeShareable(new FJsonValueObject(A)));
				}
			}
			Result->SetArrayField(TEXT("actors"), ActorsArr);
			Result->SetNumberField(TEXT("count"), ActorsArr.Num());
			return JsonResponse(Result);
		});

	// -- Spawn an actor by class
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/spawn"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid())
				return ErrorResponse(TEXT("Invalid JSON"));

			FString ClassName = Json->GetStringField(TEXT("class"));
			FString Label = Json->HasField(TEXT("label")) ? Json->GetStringField(TEXT("label")) : FString();

			UWorld* World = GetEditorWorld();
			if (!World)
				return ErrorResponse(TEXT("Editor world not available"));

			FString FullPath = ClassName.Contains(TEXT("/")) ? ClassName : FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
			UClass* ActorClass = LoadObject<UClass>(nullptr, *FullPath);
			if (!ActorClass)
				ActorClass = FindObject<UClass>(nullptr, *ClassName);
			if (!ActorClass)
				return ErrorResponse(FString::Printf(TEXT("Class not found: %s"), *ClassName));

			FVector Location(0, 0, 0);
			FRotator Rotation(0, 0, 0);
			const TArray<TSharedPtr<FJsonValue>>* LocArr = nullptr;
			if (Json->TryGetArrayField(TEXT("location"), LocArr) && LocArr && LocArr->Num() >= 3)
			{
				Location = FVector((*LocArr)[0]->AsNumber(), (*LocArr)[1]->AsNumber(), (*LocArr)[2]->AsNumber());
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* Spawned = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
			if (!Spawned)
				return ErrorResponse(TEXT("Failed to spawn actor"));

			if (!Label.IsEmpty())
				Spawned->SetActorLabel(Label);

			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			Result->SetStringField(TEXT("name"), Spawned->GetName());
			Result->SetStringField(TEXT("label"), Spawned->GetActorLabel());
			Result->SetStringField(TEXT("class"), Spawned->GetClass()->GetName());
			Result->SetBoolField(TEXT("success"), true);
			return JsonResponse(Result);
		});

	// -- Execute Python code (inline or file path)
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/exec"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid() || !Json->HasField(TEXT("code")))
				return ErrorResponse(TEXT("Missing 'code' field"));

			FString Code = Json->GetStringField(TEXT("code"));
			bool bIsFile = FPaths::FileExists(Code);

			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			FString Output;
			bool bSuccess = false;

			if (GEngine)
			{
				FString PyCmd;
				if (bIsFile)
				{
					PyCmd = FString::Printf(TEXT("py \"%s\""), *Code);
				}
				else
				{
					// Write inline code to temp file
					FString TempFile = FPaths::ProjectSavedDir() / TEXT("Python") / TEXT("_exec_temp.py");
					FFileHelper::SaveStringToFile(Code, *TempFile);
					PyCmd = FString::Printf(TEXT("py \"%s\""), *TempFile);
				}

				GEngine->Exec(nullptr, *PyCmd, *GLog);
				bSuccess = true;
				Output = FString::Printf(TEXT("Executed: py %s"), bIsFile ? *Code : TEXT("<inline>"));
			}
			else
			{
				Output = TEXT("GEngine not available");
			}

			Result->SetStringField(TEXT("output"), Output);
			Result->SetBoolField(TEXT("success"), bSuccess);
			return JsonResponse(Result);
		});

	// -- Get property
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/property"), EHttpServerRequestVerbs::VERB_GET,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			const FString* ActorPtr = Request.QueryParams.Find(TEXT("actor"));
			const FString* PropPtr = Request.QueryParams.Find(TEXT("property"));
			if (!ActorPtr || !PropPtr)
				return ErrorResponse(TEXT("Missing 'actor' or 'property'"));

			UWorld* World = GetEditorWorld();
			if (!World)
				return ErrorResponse(TEXT("Editor world not available"));

			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* Actor = *It;
				if (Actor->GetName() != *ActorPtr && Actor->GetActorLabel() != *ActorPtr)
					continue;

				for (TFieldIterator<FProperty> PropIt(Actor->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Prop = *PropIt;
					if (Prop->GetName() != *PropPtr)
						continue;

					void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
					FString ValueStr;
					Prop->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);
					TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
					Result->SetStringField(TEXT("actor"), *ActorPtr);
					Result->SetStringField(TEXT("property"), *PropPtr);
					Result->SetStringField(TEXT("value"), ValueStr);
					Result->SetStringField(TEXT("type"), Prop->GetClass()->GetName());
					return JsonResponse(Result);
				}
				return ErrorResponse(FString::Printf(TEXT("Property not found: %s"), **PropPtr));
			}
			return ErrorResponse(FString::Printf(TEXT("Actor not found: %s"), **ActorPtr));
		});

	// -- Set property
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/property"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid())
				return ErrorResponse(TEXT("Invalid JSON"));

			FString ActorName = Json->GetStringField(TEXT("actor"));
			FString PropertyName = Json->GetStringField(TEXT("property"));
			FString ValueStr = Json->GetStringField(TEXT("value"));

			UWorld* World = GetEditorWorld();
			if (!World)
				return ErrorResponse(TEXT("Editor world not available"));

			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* Actor = *It;
				if (Actor->GetName() != ActorName && Actor->GetActorLabel() != ActorName)
					continue;

				for (TFieldIterator<FProperty> PropIt(Actor->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Prop = *PropIt;
					if (Prop->GetName() != PropertyName)
						continue;

					void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Actor);
					Prop->ImportText_Direct(*ValueStr, ValuePtr, Actor, PPF_None);
					TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
					Result->SetStringField(TEXT("property"), PropertyName);
					Result->SetStringField(TEXT("value"), ValueStr);
					Result->SetBoolField(TEXT("success"), true);
					return JsonResponse(Result);
				}
				return ErrorResponse(FString::Printf(TEXT("Property not found: %s"), *PropertyName));
			}
			return ErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
		});

	// -----------------------------------------------------------------------
	// ASSET ENDPOINTS
	// -----------------------------------------------------------------------

	// -- Import a file (FBX, OBJ, GLTF, etc.) into the content browser
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/asset/import"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid())
				return ErrorResponse(TEXT("Invalid JSON"));

			FString SourceFile = Json->GetStringField(TEXT("file"));
			FString DestPath = Json->HasField(TEXT("destination")) ? Json->GetStringField(TEXT("destination")) : TEXT("/Game/Imports");

			if (SourceFile.IsEmpty())
				return ErrorResponse(TEXT("Missing 'file' field"));

			if (!FPaths::FileExists(SourceFile))
				return ErrorResponse(FString::Printf(TEXT("File not found: %s"), *SourceFile));

			// Run import on game thread
			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			bool bSuccess = false;
			TArray<FString> ImportedPaths;

			UAssetImportTask* Task = NewObject<UAssetImportTask>();
			Task->Filename = SourceFile;
			Task->DestinationPath = DestPath;
			Task->bAutomated = true;
			Task->bReplaceExisting = true;
			Task->bSave = true;

			IAssetTools::Get().ImportAssetTasks({ Task });

			for (UObject* Obj : Task->GetObjects())
			{
				if (Obj)
				{
					ImportedPaths.Add(Obj->GetPathName());
					bSuccess = true;
				}
			}

			Result->SetBoolField(TEXT("success"), bSuccess);
			TArray<TSharedPtr<FJsonValue>> PathsArr;
			for (const FString& P : ImportedPaths)
				PathsArr.Add(MakeShareable(new FJsonValueString(P)));
			Result->SetArrayField(TEXT("imported"), PathsArr);
			return JsonResponse(Result);
		});

	// -- List assets in a content folder
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/assets"), EHttpServerRequestVerbs::VERB_GET,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			FString Folder = TEXT("/Game");
			if (Request.QueryParams.Contains(TEXT("folder")))
				Folder = Request.QueryParams[TEXT("folder")];

			IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
			TArray<FAssetData> AssetList;
			Registry.GetAssetsByPath(FName(*Folder), AssetList, true);

			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			TArray<TSharedPtr<FJsonValue>> AssetsArr;
			for (const FAssetData& Asset : AssetList)
			{
				TSharedPtr<FJsonObject> A = MakeShareable(new FJsonObject());
				A->SetStringField(TEXT("name"), Asset.AssetName.ToString());
				A->SetStringField(TEXT("path"), Asset.PackagePath.ToString());
				A->SetStringField(TEXT("class"), Asset.AssetClassPath.ToString());
				AssetsArr.Add(MakeShareable(new FJsonValueObject(A)));
			}
			Result->SetArrayField(TEXT("assets"), AssetsArr);
			Result->SetNumberField(TEXT("count"), AssetsArr.Num());
			return JsonResponse(Result);
		});

	// -- Export an asset to a file
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/asset/export"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid())
				return ErrorResponse(TEXT("Invalid JSON"));

			FString AssetPath = Json->GetStringField(TEXT("asset"));
			FString OutputFile = Json->HasField(TEXT("output")) ? Json->GetStringField(TEXT("output")) : FString();
			FString Format = Json->HasField(TEXT("format")) ? Json->GetStringField(TEXT("format")) : TEXT("fbx");

			if (AssetPath.IsEmpty())
				return ErrorResponse(TEXT("Missing 'asset' field"));

			UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
			if (!Asset)
				return ErrorResponse(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));

			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			IAssetTools::Get().ExportAssets({ AssetPath }, { OutputFile });
			Result->SetBoolField(TEXT("success"), true);
			Result->SetStringField(TEXT("exported"), OutputFile);
			return JsonResponse(Result);
		});

	// -- Place an asset from content browser into the level as an actor
	AddRoute(HttpRouter, RouteHandles, TEXT("/api/asset/place"), EHttpServerRequestVerbs::VERB_POST,
		[](const FHttpServerRequest& Request) -> TUniquePtr<FHttpServerResponse>
		{
			TSharedPtr<FJsonObject> Json = ParseBody(Request);
			if (!Json.IsValid())
				return ErrorResponse(TEXT("Invalid JSON"));

			FString AssetRef = Json->GetStringField(TEXT("asset"));
			if (AssetRef.IsEmpty())
				return ErrorResponse(TEXT("Missing 'asset' field"));

			UWorld* World = GetEditorWorld();
			if (!World)
				return ErrorResponse(TEXT("Editor world not available"));

			// Try to load as StaticMesh
			UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *AssetRef));
			if (!Mesh)
				return ErrorResponse(FString::Printf(TEXT("Could not load StaticMesh: %s. Asset must be a StaticMesh."), *AssetRef));

			FVector Location(0, 0, 0);
			FRotator Rotation(0, 0, 0);
			const TArray<TSharedPtr<FJsonValue>>* LocArr = nullptr;
			if (Json->TryGetArrayField(TEXT("location"), LocArr) && LocArr && LocArr->Num() >= 3)
			{
				Location = FVector((*LocArr)[0]->AsNumber(), (*LocArr)[1]->AsNumber(), (*LocArr)[2]->AsNumber());
			}
			if (Json->HasField(TEXT("rotation")))
			{
				// comma-separated or array
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AStaticMeshActor* SMActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParams);
			if (!SMActor)
				return ErrorResponse(TEXT("Failed to spawn StaticMeshActor"));

			SMActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			SMActor->RegisterAllComponents();

			if (Json->HasField(TEXT("label")))
				SMActor->SetActorLabel(Json->GetStringField(TEXT("label")));

			// Save the level so the change persists
			UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

			TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
			Result->SetStringField(TEXT("name"), SMActor->GetName());
			Result->SetStringField(TEXT("label"), SMActor->GetActorLabel());
			Result->SetStringField(TEXT("mesh"), Mesh->GetPathName());
			Result->SetStringField(TEXT("location"), Location.ToString());
			Result->SetBoolField(TEXT("success"), true);
			return JsonResponse(Result);
		});

	HttpServerModule.StartAllListeners();
	bRunning = true;
	UE_LOG(LogTerminalServer, Log, TEXT("TerminalServer started on port %d (asset import/export enabled)"), ServerPort);
	return true;
}

void FTerminalServer::Stop()
{
	if (HttpRouter.IsValid())
	{
		for (auto& Handle : RouteHandles)
		{
			HttpRouter->UnbindRoute(Handle);
		}
		RouteHandles.Empty();
	}
	bRunning = false;
	UE_LOG(LogTerminalServer, Log, TEXT("TerminalServer stopped"));
}
