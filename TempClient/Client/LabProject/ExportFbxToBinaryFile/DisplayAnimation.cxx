/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayCommon.h"
#include "DisplayAnimation.h"

#if defined (FBXSDK_ENV_MAC)
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

int GetAnimationCurves(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode)
{
	int nAnimationCurves = 0;

	FbxAnimCurve *pfbxAnimationCurve = NULL;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;
    if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;
    if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y)) nAnimationCurves++;
	if (pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z)) nAnimationCurves++;

	return(nAnimationCurves);
}

int GetAnimationLayerCurveNodes(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode)
{
	int nAnimationCurveNodes = 0;
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0) nAnimationCurveNodes = 1;

	for (int i = 0; i < pfbxNode->GetChildCount(); i++)
    {
        nAnimationCurveNodes += GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode->GetChild(i));
    }

	return(nAnimationCurveNodes);
}

void WriteCurveKeys(char *pszHeader, FbxAnimCurve *pfbxAnimationCurve, bool bRotationAngle)
{
    int nKeys = pfbxAnimationCurve->KeyGetCount();
	WriteInt(pszHeader, nKeys);

    for (int i = 0; i < nKeys; i++)
    {
		FbxTime fbxKeyTime = pfbxAnimationCurve->KeyGetTime(i);
		float fkeyTime = (float)fbxKeyTime.GetSecondDouble();
		WriteFloat(fkeyTime);
    }

    for (int i = 0; i < nKeys; i++)
    {
		float fKeyValue = static_cast<float>(pfbxAnimationCurve->KeyGetValue(i));
		if (bRotationAngle) fKeyValue = (3.1415926f / 180.f) * fKeyValue;
		WriteFloat(fKeyValue);
    }
}

void WriteChannels(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode)
{
	if (GetAnimationCurves(pfbxAnimationLayer, pfbxNode) > 0)
	{
		WriteIntString("<AnimationCurve>:", (*pnCurveNode)++, ReplaceBlank(pfbxNode->GetName(), '_'));

		FbxAnimCurve *pfbxAnimationCurve = NULL;

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if(pfbxAnimationCurve) WriteCurveKeys("<TX>:", pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve)
			WriteCurveKeys("<TY>:", pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclTranslation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) WriteCurveKeys("<TZ>:", pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if(pfbxAnimationCurve) WriteCurveKeys("<RX>:", pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve) WriteCurveKeys("<RY>:", pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclRotation.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) WriteCurveKeys("<RZ>:", pfbxAnimationCurve, true);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if(pfbxAnimationCurve) WriteCurveKeys("<SX>:", pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if(pfbxAnimationCurve) WriteCurveKeys("<SY>:", pfbxAnimationCurve, false);

		pfbxAnimationCurve = pfbxNode->LclScaling.GetCurve(pfbxAnimationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if(pfbxAnimationCurve) WriteCurveKeys("<SZ>:", pfbxAnimationCurve, false);

		WriteString("</AnimationCurve>");
	}
}

void WriteAnimation(FbxAnimLayer *pfbxAnimationLayer, FbxNode *pfbxNode, int *pnCurveNode)
{
    WriteChannels(pfbxAnimationLayer, pfbxNode, pnCurveNode);

    for (int i = 0; i < pfbxNode->GetChildCount(); i++)
    {
        WriteAnimation(pfbxAnimationLayer, pfbxNode->GetChild(i), pnCurveNode);
    }
}

void WriteAnimation(FbxAnimStack *pfbxAnimStack, FbxNode *pfbxNode)
{
    int nAnimationLayers = pfbxAnimStack->GetMemberCount<FbxAnimLayer>();
	WriteInt("<AnimationLayers>:", nAnimationLayers);

    for (int i = 0; i < nAnimationLayers; i++)
    {
        FbxAnimLayer *pfbxAnimationLayer = pfbxAnimStack->GetMember<FbxAnimLayer>(i);
		int nLayerCurveNodes = GetAnimationLayerCurveNodes(pfbxAnimationLayer, pfbxNode);

		int nCurveNode = 0;
		WriteIntFloat("<AnimationLayer>:", i, nLayerCurveNodes, float(pfbxAnimationLayer->Weight) / 100.0f);
        WriteAnimation(pfbxAnimationLayer, pfbxNode, &nCurveNode);
		WriteString("</AnimationLayer>");
	}

	WriteString("</AnimationLayers>");
}

void WriteAnimation(FbxScene *pfbxScene)
{
	int nAnimationStacks = pfbxScene->GetSrcObjectCount<FbxAnimStack>();
	WriteInt("<AnimationSets>:", nAnimationStacks);

    for (int i = 0; i < nAnimationStacks; i++)
    {
		FbxAnimStack *pfbxAnimStack = pfbxScene->GetSrcObject<FbxAnimStack>(i);

		WriteIntString("<AnimationSet>:", i, ReplaceBlank(pfbxAnimStack->GetName(), '_'));
		FbxTime fbxTimeStart = pfbxAnimStack->LocalStart;
		FbxTime fbxTimeStop = pfbxAnimStack->LocalStop;
		WriteFloat((float)fbxTimeStart.GetSecondDouble(), (float)fbxTimeStop.GetSecondDouble());

        WriteAnimation(pfbxAnimStack, pfbxScene->GetRootNode());
		WriteString("</AnimationSet>");
    }

	WriteString("</AnimationSets>");
}

void WriteAnimation(FbxScene **ppfbxScenes, int nSeparatedAnimations)
{
	int nAnimationStacks = 0;
	int *pnAnimationStacks = new int[nSeparatedAnimations];

	for (int i = 0; i < nSeparatedAnimations; i++)
	{
		 pnAnimationStacks[i] = ppfbxScenes[i]->GetSrcObjectCount<FbxAnimStack>();
		 nAnimationStacks += pnAnimationStacks[i];
	}
	
	WriteInt("<AnimationSets>:", nAnimationStacks);

	for (int i = 0, k = 0; i < nSeparatedAnimations; i++)
	{
		for (int j = 0; j < pnAnimationStacks[i]; j++, k++)
		{
			FbxAnimStack *pfbxAnimStack = ppfbxScenes[i]->GetSrcObject<FbxAnimStack>(j);
			WriteIntString("<AnimationSet>:", k, ReplaceBlank(pfbxAnimStack->GetName(), '_'));
			FbxTime fbxTimeStart = pfbxAnimStack->LocalStart;
			FbxTime fbxTimeStop = pfbxAnimStack->LocalStop;
			WriteFloat((float)fbxTimeStart.GetSecondDouble(), (float)fbxTimeStop.GetSecondDouble());

			WriteAnimation(pfbxAnimStack, ppfbxScenes[i]->GetRootNode());
			WriteString("</AnimationSet>");
		}
	}

	WriteString("</AnimationSets>");

	delete[] pnAnimationStacks;
}

