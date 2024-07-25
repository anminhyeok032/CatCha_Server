// ExportFbxToBinaryFile.cpp :이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "Common/Common.h"
#include "DisplayCommon.h"
#include "DisplayHierarchy.h"
#include "DisplayAnimation.h"


FILE *gpBinaryFile = NULL;
char pszBuffer[256];

void WriteStringToFile(char *pszString) { ::fwrite(pszString, sizeof(char), strlen(pszString), gpBinaryFile); }

void WriteGlobalTimeSettings(FbxGlobalSettings *pfbxGlobalSettings)
{
	char pstrTimeString[256];

	WriteString("<TimeMode>:", FbxGetTimeModeName(pfbxGlobalSettings->GetTimeMode()));

	FbxTimeSpan fbxTimeSpan;
	pfbxGlobalSettings->GetTimelineDefaultTimeSpan(fbxTimeSpan);
	FbxTime fbxTimeStart = fbxTimeSpan.GetStart();
	FbxTime fbxTimeEnd = fbxTimeSpan.GetStop();
	WriteString("<TimelineDefaultTimeSpan>:");
	WriteString(fbxTimeStart.GetTimeString(pstrTimeString, FbxUShort(256)));
	WriteString(fbxTimeEnd.GetTimeString(pstrTimeString, FbxUShort(256)));
}

int main(int argc, char** argv)
{
	FbxManager *pfbxSdkManager = NULL;
    FbxScene *pfbxScene = NULL;

    InitializeSdkObjects(pfbxSdkManager);

//	FbxString pfbxstrModelFilePath("Eagle.fbx");
//	char *pszWriteFileName = "Eagle.bin";
//	FbxString pfbxstrModelFilePath("Lion.fbx");
//	char *pszWriteFileName = "Lion.bin";
	FbxString pfbxstrModelFilePath("../Resource/Model/Player_Model.fbx");
	char *pszWriteFileName = "../Resource/Model/Player_Model.bin";

	//FbxString pfbxstrModelFilePath("../Resource/Model/Weapons/Sphere_TXT_Pivot.fbx");
	//char* pszWriteFileName = "../Resource/Model/Weapons/Sphere_TXT_Pivot.bin";
	//FbxString pfbxstrModelFilePath("../Resource/Model/Building/Test_TXT.fbx");
	//char* pszWriteFileName = "../Resource/Model/Building/Test_TXT.bin";

//	FbxString pfbxstrModelFilePath("Angrybot.fbx");
//	char *pszWriteFileName = "Angrybot.bin";
//	FbxString pfbxstrModelFilePath("Elven_Witch.fbx");
//	char *pszWriteFileName = "Elven_Witch.bin";

    FbxScene *pfbxModelScene = FbxScene::Create(pfbxSdkManager, pfbxstrModelFilePath);
	bool bResult = LoadScene(pfbxSdkManager, pfbxModelScene, pfbxstrModelFilePath.Buffer());

#define _WITH_SEPARATED_ANIMATIONS

#ifdef _WITH_SEPARATED_ANIMATIONS
	const int nSeparatedAnimations = 21;
	FbxString pfbxstrAnimationFilePaths[nSeparatedAnimations];
	FbxScene *ppfbxAnimationScenes[nSeparatedAnimations];
	pfbxstrAnimationFilePaths[0] = "../Resource/Model/Player_Idle.fbx";
	pfbxstrAnimationFilePaths[1] = "../Resource/Model/Player_Roll.fbx";
	pfbxstrAnimationFilePaths[2] = "../Resource/Model/Player_Run.fbx";
	pfbxstrAnimationFilePaths[3] = "../Resource/Model/Player_Walk.fbx";

	pfbxstrAnimationFilePaths[4] = "../Resource/Model/Player_Sword_00.fbx";
	pfbxstrAnimationFilePaths[5] = "../Resource/Model/Player_Sword_11_Fix.fbx";
	pfbxstrAnimationFilePaths[6] = "../Resource/Model/Player_Sword_12_Fix.fbx";
	pfbxstrAnimationFilePaths[7] = "../Resource/Model/Player_Sword_13_Fix.fbx";
	pfbxstrAnimationFilePaths[8] = "../Resource/Model/Player_Sword_21_Fix.fbx";
	pfbxstrAnimationFilePaths[9] = "../Resource/Model/Player_Sword_22_Fix.fbx";
	pfbxstrAnimationFilePaths[10] = "../Resource/Model/Player_Sword_23_Fix.fbx";
	pfbxstrAnimationFilePaths[11] = "../Resource/Model/Player_Sword_30_Fix.fbx";
	pfbxstrAnimationFilePaths[12] = "../Resource/Model/Player_Sword_40_Fix.fbx";

	pfbxstrAnimationFilePaths[13] = "../Resource/Model/Player_Sphere_00_Fix.fbx";
	pfbxstrAnimationFilePaths[14] = "../Resource/Model/Player_Sphere_11_Fix.fbx";
	pfbxstrAnimationFilePaths[15] = "../Resource/Model/Player_Sphere_12_Fix.fbx";
	pfbxstrAnimationFilePaths[16] = "../Resource/Model/Player_Sphere_20_Fix.fbx";
	pfbxstrAnimationFilePaths[17] = "../Resource/Model/Player_Sphere_31_Fix.fbx";
	pfbxstrAnimationFilePaths[18] = "../Resource/Model/Player_Sphere_32_Fix.fbx";
	pfbxstrAnimationFilePaths[19] = "../Resource/Model/Player_Sphere_33_Fix.fbx";
	pfbxstrAnimationFilePaths[20] = "../Resource/Model/Player_Sphere_40_Fix.fbx";

	//const int nSeparatedAnimations = 3;
	//FbxString pfbxstrAnimationFilePaths[nSeparatedAnimations];
	//FbxScene *ppfbxAnimationScenes[nSeparatedAnimations];
	//pfbxstrAnimationFilePaths[0] = "Eagle@TakeOffGrounded.fbx";
	//pfbxstrAnimationFilePaths[1] = "Eagle@Fly.fbx";
	//pfbxstrAnimationFilePaths[2] = "Eagle@Glide.fbx";

	for(int i = 0; i < nSeparatedAnimations; i++)
	{
		ppfbxAnimationScenes[i] = FbxScene::Create(pfbxSdkManager, pfbxstrAnimationFilePaths[i]);
		LoadScene(pfbxSdkManager, ppfbxAnimationScenes[i], pfbxstrAnimationFilePaths[i].Buffer());
	}
#endif
	FbxGeometryConverter fbxGeomConverter(pfbxSdkManager);

	//FbxAxisSystem fbxSceneAxisSystem = pfbxModelScene->GetGlobalSettings().GetAxisSystem();
	//FbxAxisSystem fbxDirectXAxisSystem(FbxAxisSystem::eDirectX);
	//if (fbxSceneAxisSystem != fbxDirectXAxisSystem) fbxDirectXAxisSystem.ConvertScene(pfbxModelScene);

	//FbxSystemUnit fbxSceneSystemUnit = pfbxModelScene->GetGlobalSettings().GetSystemUnit();
	//if (fbxSceneSystemUnit.GetScaleFactor() != 1.0) FbxSystemUnit::cm.ConvertScene(pfbxModelScene);

	fbxGeomConverter.Triangulate(pfbxModelScene, true);
	//fbxGeomConverter.RemoveBadPolygonsFromMeshes(pfbxModelScene, NULL);
	//fbxGeomConverter.SplitMeshesPerMaterial(pfbxModelScene, true);


#ifdef _WITH_SEPARATED_ANIMATIONS
	//for(int i = 0; i < nSeparatedAnimations; i++)
	//{
	//	if (fbxSceneSystemUnit.GetScaleFactor() != 1.0) FbxSystemUnit::cm.ConvertScene(ppfbxAnimationScenes[i]);
	//}
#endif

	::fopen_s(&gpBinaryFile, pszWriteFileName, "wb");

	//WriteString("<GlobalTimeSettings>:");
  	//WriteGlobalTimeSettings(&pfbxScene->GetGlobalSettings());
	//WriteString("</GlobalTimeSettings>");

	WriteString("<Hierarchy>");
	WriteHierarchy(pfbxModelScene);
	WriteString("</Hierarchy>");

	WriteString("<Animation>");
#ifdef _WITH_SEPARATED_ANIMATIONS
	WriteAnimation(ppfbxAnimationScenes, nSeparatedAnimations);
#else
	WriteAnimation(pfbxModelScene);
#endif
	WriteString("</Animation>");

	::fclose(gpBinaryFile);
    DestroySdkObjects(pfbxSdkManager, bResult);

	return(0);
}

