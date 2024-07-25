/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include <fbxsdk.h>
#include "DisplayCommon.h"

#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
FbxAMatrix GetGeometricOffsetTransform(FbxNode *pfbxNode)
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return(FbxAMatrix(T, R, S));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void WritePolygonVertexColors(FbxMesh *pfbxMesh, int nPolygons)
{
	int nColorsPerVertex = pfbxMesh->GetElementVertexColorCount();
	if (nColorsPerVertex > 0)
	{
		int nColors = 0;
		for (int i = 0; i < nPolygons; i++) nColors += pfbxMesh->GetPolygonSize(i) * nColorsPerVertex;

		WriteInt("<Colors>:", nColors, nColorsPerVertex);
		for (int k = 0; k < nColorsPerVertex; k++)
		{
			WriteInt("<Color>:", k);
			FbxGeometryElementVertexColor *pfbxElementVertexColor = pfbxMesh->GetElementVertexColor(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i); //Triangle:3, Triangulate()
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					switch (pfbxElementVertexColor->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
							switch (pfbxElementVertexColor->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									WriteColor(pfbxElementVertexColor->GetDirectArray().GetAt(nControlPointIndex));
									break;
								case FbxGeometryElement::eIndexToDirect:
									WriteColor(pfbxElementVertexColor->GetDirectArray().GetAt(pfbxElementVertexColor->GetIndexArray().GetAt(nControlPointIndex)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							switch (pfbxElementVertexColor->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									WriteColor(pfbxElementVertexColor->GetDirectArray().GetAt(nVertexID));
									break;
								case FbxGeometryElement::eIndexToDirect:
									WriteColor(pfbxElementVertexColor->GetDirectArray().GetAt(pfbxElementVertexColor->GetIndexArray().GetAt(nVertexID)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygon:
						case FbxGeometryElement::eAllSame:
						case FbxGeometryElement::eNone:
							break;
						default:
							break;
					}
				}
			}
		}
	}
}

void WritePolygonVertexUVs(FbxMesh *pfbxMesh, int nPolygons)
{
	int nUVsPerVertex = pfbxMesh->GetElementUVCount(); //UVs Per Polygon's Vertex
	if (nUVsPerVertex > 0)
	{
		int nUVs = 0;
		for (int i = 0; i < nPolygons; i++) nUVs += pfbxMesh->GetPolygonSize(i) * nUVsPerVertex;

		WriteInt("<UVs>:", nUVs, nUVsPerVertex);
		for (int k = 0; k < nUVsPerVertex; k++)
		{
			WriteInt("<UV>:", k);
			for (int i = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i); //Triangle:3, Triangulate()
				for (int j = 0; j < nPolygonSize; j++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					FbxGeometryElementUV *pfbxElementUV = pfbxMesh->GetElementUV(k);
					switch (pfbxElementUV->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
							switch (pfbxElementUV->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									WriteUV2DVector(pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex));
									break;
								case FbxGeometryElement::eIndexToDirect:
									WriteUV2DVector(pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex)));
									break;
								default:
									break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							{
								int nTextureUVIndex = pfbxMesh->GetTextureUVIndex(i, j);
								switch (pfbxElementUV->GetReferenceMode())
								{
									case FbxGeometryElement::eDirect:
									case FbxGeometryElement::eIndexToDirect:
										WriteUV2DVector(pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex));
										break;
									default:
										break;
								}
							}
							break;
						case FbxGeometryElement::eByPolygon:
						case FbxGeometryElement::eAllSame:
						case FbxGeometryElement::eNone:
							break;
						default:
							break;
					}
				}
			}
		}
	}
}

void WritePolygonVertexNormals(FbxMesh *pfbxMesh, int nPolygons)
{
	int nNormalsPerVertex = pfbxMesh->GetElementNormalCount();
	if (nNormalsPerVertex > 0)
	{
		int nNormals = 0;
		for (int i = 0; i < nPolygons; i++) nNormals += pfbxMesh->GetPolygonSize(i) * nNormalsPerVertex;

		WriteInt("<Normals>:", nNormals, nNormalsPerVertex);
		for (int k = 0; k < nNormalsPerVertex; k++)
		{
			FbxGeometryElementNormal *pfbxElementNormal = pfbxMesh->GetElementNormal(k);
			WriteInt("<Normal>:", k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					if (pfbxElementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementNormal->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Write3DVector(pfbxElementNormal->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Write3DVector(pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}
}

void WritePolygonVertexTangents(FbxMesh *pfbxMesh, int nPolygons)
{
	int nTangentsPerVertex = pfbxMesh->GetElementTangentCount();
	if (nTangentsPerVertex > 0)
	{
		int nTangents = 0;
		for (int i = 0; i < nPolygons; i++) nTangents += pfbxMesh->GetPolygonSize(i) * nTangentsPerVertex;

		WriteInt("<Tangents>:", nTangents, nTangentsPerVertex);
		for (int k = 0; k < nTangentsPerVertex; k++)
		{
			WriteInt("<Tangent>:", k);
			FbxGeometryElementTangent *pfbxElementTangent = pfbxMesh->GetElementTangent(k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					if (pfbxElementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Write3DVector(pfbxElementTangent->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Write3DVector(pfbxElementTangent->GetDirectArray().GetAt(pfbxElementTangent->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}
}

void WritePolygonVertexBinormals(FbxMesh *pfbxMesh, int nPolygons)
{
	int nBinormalsPerVertex = pfbxMesh->GetElementTangentCount();
	if (nBinormalsPerVertex > 0)
	{
		int nBinormals = 0;
		for (int i = 0; i < nPolygons; i++) nBinormals += pfbxMesh->GetPolygonSize(i) * nBinormalsPerVertex;

		WriteInt("<BiTangents>:", nBinormals, nBinormalsPerVertex);
		for (int k = 0; k < nBinormalsPerVertex; k++)
		{
			FbxGeometryElementBinormal *pfbxElementBinormal = pfbxMesh->GetElementBinormal(k);
			WriteInt("<BiTangent>:", k);
			for (int i = 0, nVertexID = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i);
				for (int j = 0; j < nPolygonSize; j++, nVertexID++)
				{
					if (pfbxElementBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (pfbxElementBinormal->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								Write3DVector(pfbxElementBinormal->GetDirectArray().GetAt(nVertexID));
								break;
							case FbxGeometryElement::eIndexToDirect:
								Write3DVector(pfbxElementBinormal->GetDirectArray().GetAt(pfbxElementBinormal->GetIndexArray().GetAt(nVertexID)));
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}
}

void WritePolygonVertexIndices(FbxMesh *pfbxMesh, int nPolygons)
{
	int nPolygonIndices = nPolygons * 3; //Triangle: 3, Triangulate(), nIndices = nPolygons * 3

	int *pnPolygonIndices = new int[nPolygonIndices];
	for (int i = 0, k = 0; i < nPolygons; i++)
	{
		for (int j = 0; j < 3; j++) pnPolygonIndices[k++] = pfbxMesh->GetPolygonVertex(i, j);
	}

	FbxNode *pfbxNode = pfbxMesh->GetNode();
    int nMaterials = pfbxNode->GetMaterialCount();
	WriteInt("<SubIndices>:", nPolygonIndices, nMaterials);

	if (nMaterials > 1)
	{
		int nElementMaterials = pfbxMesh->GetElementMaterialCount();
		for (int i = 0; i < nElementMaterials; i++)
		{
			FbxGeometryElementMaterial *pfbxElementMaterial = pfbxMesh->GetElementMaterial(i);
			FbxGeometryElement::EReferenceMode nReferenceMode = pfbxElementMaterial->GetReferenceMode();
			switch (nReferenceMode)
			{
				case FbxGeometryElement::eDirect:
					break;
				case FbxGeometryElement::eIndex:
				case FbxGeometryElement::eIndexToDirect:
				{
					int *pnSubIndices = new int[nMaterials];
					memset(pnSubIndices, 0, sizeof(int) * nMaterials);

					int nSubIndices = pfbxElementMaterial->GetIndexArray().GetCount();
					for (int j = 0; j < nSubIndices; j++) pnSubIndices[pfbxElementMaterial->GetIndexArray().GetAt(j)]++;

					int **ppnSubIndices = new int*[nMaterials];
					for (int k = 0; k < nMaterials; k++)
					{
						pnSubIndices[k] *= 3;
						ppnSubIndices[k] = new int[pnSubIndices[k]];
					}

					int *pnToAppends = new int[nMaterials];
					memset(pnToAppends, 0, sizeof(int) * nMaterials);
					for (int j = 0, k = 0; j < nSubIndices; j++)
					{
						int nMaterial = pfbxElementMaterial->GetIndexArray().GetAt(j);
						for (int i = 0; i < 3; i++) ppnSubIndices[nMaterial][pnToAppends[nMaterial]++] = pnPolygonIndices[k++];
					}

					for (int k = 0; k < nMaterials; k++)
					{
						WriteInt("<SubIndex>:", k, pnSubIndices[k]);
						for (int j = 0; j < pnSubIndices[k]; j++) WriteInt(ppnSubIndices[k][j]);
					}

					if (pnSubIndices) delete[] pnSubIndices;
					for (int k = 0; k < nMaterials; k++) if (ppnSubIndices[k]) delete[] ppnSubIndices[k];
					if (ppnSubIndices) delete[] ppnSubIndices;
					if (pnToAppends) delete[] pnToAppends;

					break;
				}
			}
		}
	}
	else
	{
		WriteInt("<SubIndex>:", 0, nPolygonIndices);
		for (int i = 0; i < nPolygonIndices; i++) WriteInt(pnPolygonIndices[i]);
	}

	if (pnPolygonIndices) delete[] pnPolygonIndices;
}

void WritePolygons(FbxMesh *pfbxMesh, int nPolygons)
{
	WriteInt("<Polygons>:", nPolygons);

	WritePolygonVertexIndices(pfbxMesh, nPolygons);

	//WritePolygonVertexColors(pfbxMesh, nPolygons);
	//WritePolygonVertexUVs(pfbxMesh, nPolygons);
	//WritePolygonVertexNormals(pfbxMesh, nPolygons);
	//WritePolygonVertexTangents(pfbxMesh, nPolygons);
	//WritePolygonVertexBinormals(pfbxMesh, nPolygons);

	WriteString("</Polygons>");
}

void WriteTextureInfo(FbxTexture *pfbxTexture, int nBlendMode)
{
	WriteString(ReplaceBlank(pfbxTexture->GetName(), '_'));

	FbxFileTexture *pfbxFileTexture = FbxCast<FbxFileTexture>(pfbxTexture);
	if (pfbxFileTexture)
	{
		FbxString fbxPathName = pfbxFileTexture->GetRelativeFileName();
		FbxString fbxFileName = fbxPathName.Right(fbxPathName.GetLen() - fbxPathName.ReverseFind('\\') - 1);
		WriteIntString(0, ReplaceBlank(fbxFileName.Buffer(), '_')); //0:"File"
	}
	else 
	{
		FbxProceduralTexture *pfbxProceduralTexture = FbxCast<FbxProceduralTexture>(pfbxTexture);
		if (pfbxProceduralTexture) WriteInt(1); //1:"Procedural"
	}

    WriteFloat(pfbxTexture->GetScaleU(), pfbxTexture->GetScaleV());
    WriteFloat(pfbxTexture->GetTranslationU(), pfbxTexture->GetTranslationV());
    WriteBool(pfbxTexture->GetSwapUV());
    WriteFloat(pfbxTexture->GetRotationU(), pfbxTexture->GetRotationV(), pfbxTexture->GetRotationW());

    WriteInt(pfbxTexture->GetAlphaSource()); //Alpha Source: 0:"None", 1:"Intensity", 2:"Black"
    WriteInt(pfbxTexture->GetCroppingLeft(), pfbxTexture->GetCroppingTop(), pfbxTexture->GetCroppingRight(), pfbxTexture->GetCroppingBottom());

    WriteInt(pfbxTexture->GetMappingType()); //Mapping Type: 0:"Null", 1:"Planar", 2:"Spherical", 3:"Cylindrical", 4:"Box", 5:"Face", 6:"UV", 7:"Environment"
    if (pfbxTexture->GetMappingType() == FbxTexture::ePlanar) WriteInt(pfbxTexture->GetPlanarMappingNormal()); //PlanarMappingNormal: 0:"X", 1:"Y", 2:"Z"
   
    WriteInt(nBlendMode); //Blend Mode: -1:"None", 0:"Translucent", 1:"Additive", 2:"Modulate", 3:"Modulate2", 4:"Over", 5:"Normal", 6:"Dissolve", 7:"Darken", 8:"ColorBurn", 9:"LinearBurn", 10:"DarkerColor", "Lighten", "Screen", "ColorDodge", "LinearDodge", "LighterColor", "SoftLight", "HardLight", "VividLight", "LinearLight", "PinLight", "HardMix", "Difference", "Exclusion", "Substract", "Divide", "Hue", "Saturation", "Color", "Luminosity", "Overlay"

	WriteFloat(pfbxTexture->GetDefaultAlpha()); //Default Alpha

	WriteInt((pfbxFileTexture) ? pfbxFileTexture->GetMaterialUse() : -1); //Material Use: 0: "Model Material", 1:"Default Material"

    WriteInt(pfbxTexture->GetTextureUse()); //Texture Use: 0:"Standard", 1:"Shadow Map", 2:"Light Map", 3:"Spherical Reflexion Map", 4:"Sphere Reflexion Map", 5:"Bump Normal Map"
}

void FindAndWriteTextureInfoByProperty(FbxSurfaceMaterial *pfbxMaterial, FbxProperty *pfbxProperty)
{
    if (pfbxProperty->IsValid())
    {
		int nLayeredTextures = pfbxProperty->GetSrcObjectCount<FbxLayeredTexture>();
		if (nLayeredTextures > 0)
		{
			WriteInt("<LayeredTextures>:", nLayeredTextures);
			for (int i = 0; i < nLayeredTextures; i++)
			{
				FbxLayeredTexture *pfbxLayeredTexture = pfbxProperty->GetSrcObject<FbxLayeredTexture>(i);
				int nTextures = pfbxLayeredTexture->GetSrcObjectCount<FbxTexture>();
				WriteInt("<LayeredTexture>:", i, nTextures);
				for (int j = 0; j < nTextures; j++)
				{
					FbxTexture *pfbxTexture = pfbxLayeredTexture->GetSrcObject<FbxTexture>(j);
					if (pfbxTexture)
					{
						WriteInt("<Texture>:", j);
						FbxLayeredTexture::EBlendMode nBlendMode;
						pfbxLayeredTexture->GetTextureBlendMode(j, nBlendMode);
						WriteTextureInfo(pfbxTexture, (int)nBlendMode);
					}
				}
			}
		}
		else
		{
			int nTextures = pfbxProperty->GetSrcObjectCount<FbxTexture>();
			WriteInt("<Textures>:", nTextures);
			if (nTextures > 0)
			{
				for (int j = 0; j < nTextures; j++)
				{
					FbxTexture *pfbxTexture = pfbxProperty->GetSrcObject<FbxTexture>(j);
					if (pfbxTexture)
					{
						WriteInt("<Texture>:", j);
						WriteTextureInfo(pfbxTexture, -1);
					}
				}
			}
		}
    }
}

void WriteHardwareShaderImplementation(FbxSurfaceMaterial *pfbxMaterial, const FbxImplementation *pfbxImplementation, FbxString fbxstrImplemenationType)
{
    if (pfbxImplementation)
    {
        const FbxBindingTable *pfbxRootTable = pfbxImplementation->GetRootTable();
        FbxString fbxstrFileName = "\"" + pfbxRootTable->DescAbsoluteURL.Get() + "\"";
        FbxString fbxstrTechniqueName = "\"" + pfbxRootTable->DescTAG.Get() + "\""; 

		WriteString("<HardwareShader>:", ReplaceBlank(fbxstrImplemenationType.Buffer(), '_'), fbxstrFileName.Buffer(), fbxstrTechniqueName.Buffer());

        const FbxBindingTable *pfbxTable = pfbxImplementation->GetRootTable();
        int nEntries = (int)pfbxTable->GetEntryCount();

		WriteInt("<Entries>:", nEntries);
        for (int i = 0; i < nEntries; i++)
        {
            const FbxBindingTableEntry& fbxEntry = pfbxTable->GetEntry(i);
            const char *pstrEntrySrcType = fbxEntry.GetEntryType(true); 

			WriteIntString("<Entry>:", i, fbxEntry.GetSource());

            FbxProperty fbxProperty;
            if (!strcmp(FbxPropertyEntryView::sEntryType, pstrEntrySrcType))
            {   
                fbxProperty = pfbxMaterial->FindPropertyHierarchical(fbxEntry.GetSource()); 
                if (!fbxProperty.IsValid())
                {
                    fbxProperty = pfbxMaterial->RootProperty.FindHierarchical(fbxEntry.GetSource());
                }
            }
            else if (!strcmp(FbxConstantEntryView::sEntryType, pstrEntrySrcType))
            {
                fbxProperty = pfbxImplementation->GetConstants().FindHierarchical(fbxEntry.GetSource());
            }

            if (fbxProperty.IsValid())
            {
				int nTextures = fbxProperty.GetSrcObjectCount<FbxTexture>();
                if (nTextures > 0)
                {
					WriteInt("<Textures>:", nTextures);

					int nFileTextures = fbxProperty.GetSrcObjectCount<FbxFileTexture>();
					WriteInt("<FileTextures>:", nFileTextures);
                    for (int j = 0; j < nFileTextures; j++)
                    {
                        FbxFileTexture *pfbxFileTexture = fbxProperty.GetSrcObject<FbxFileTexture>(j);
                        WriteString(pfbxFileTexture->GetFileName());
                    }
					WriteString("\n");

					int nLayeredTextures = fbxProperty.GetSrcObjectCount<FbxLayeredTexture>();
					WriteInt("<LayeredTextures>:", nLayeredTextures);
                    for (int j = 0; j < nLayeredTextures; j++)
                    {
                        FbxLayeredTexture *pfbxLayeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(j);
                        WriteString(pfbxLayeredTexture->GetName());
                    }
					WriteString("\n");

					int nProceduralTextures = fbxProperty.GetSrcObjectCount<FbxProceduralTexture>();
					WriteInt("<ProceduralTextures>:", nProceduralTextures);
                    for(int j = 0; j < nProceduralTextures; ++j)
                    {
                        FbxProceduralTexture *pfbxProceduralTexture = fbxProperty.GetSrcObject<FbxProceduralTexture>(j);
                        WriteString(pfbxProceduralTexture->GetName());
                    }
					WriteString("</Textures>");
                }
                else
                {
					FbxDataType lFbxType = fbxProperty.GetPropertyDataType();
                    if (FbxBoolDT == lFbxType)
                    {
                        WriteBool("<Bool>:", fbxProperty.Get<FbxBool>());
                    }
                    else if ((FbxIntDT == lFbxType) || (FbxEnumDT == lFbxType))
                    {
                        WriteInt("<Int>:", fbxProperty.Get<FbxInt>());
                    }
                    else if (FbxFloatDT == lFbxType)
                    {
                        WriteFloat("<Float>:", fbxProperty.Get<FbxFloat>());
                    }
                    else if (FbxDoubleDT == lFbxType)
                    {
                        WriteDouble("<Double>:", fbxProperty.Get<FbxDouble>());
                    }
                    else if ((FbxStringDT == lFbxType) || (FbxUrlDT == lFbxType) || (FbxXRefUrlDT == lFbxType))
                    {
                        WriteString("<String>:", fbxProperty.Get<FbxString>().Buffer());
                    }
                    else if (FbxDouble2DT == lFbxType)
                    {
                        Write2DVector("<Vector2D>:", fbxProperty.Get<FbxDouble2>());
                    }
                    else if ((FbxDouble3DT == lFbxType) || (FbxColor3DT == lFbxType))
                    {
                        Write3DVector("<Vector3D>:", fbxProperty.Get<FbxDouble3>());
                    }
                    else if ((FbxDouble4DT == lFbxType) || (FbxColor4DT == lFbxType))
                    {
                        Write4DVector("<Vector4D>:", fbxProperty.Get<FbxDouble4>());
                    }
                    else if (FbxDouble4x4DT == lFbxType)
                    {
						WriteMatrix("<Matrix>:", fbxProperty.Get<FbxDouble4x4>());
                    }
					else
					{
						WriteString("<Unknown>");
					}
                }
            }   
        }
		WriteString("</Entries>");
    }
}

void WriteMaterials(FbxGeometry *pfbxGeometry)
{
	FbxNode *pfbxNode = pfbxGeometry->GetNode();
	if (pfbxNode)
	{
		int nMaterials = pfbxNode->GetMaterialCount();
		WriteInt("<Materials>:", nMaterials);

		for (int i = 0; i < nMaterials; i++)
		{
            FbxSurfaceMaterial *pfbxMaterial = pfbxNode->GetMaterial(i);
			if (pfbxMaterial)
			{
				WriteIntString("<Material>:", i, ReplaceBlank(pfbxMaterial->GetName(), '_'));

				int nProperties = FbxLayerElement::sTypeTextureCount;		
				WriteInt("<TextureProperties>:", nProperties);
				for (int j = 0; j < nProperties; j++)
				{
					FbxProperty fbxProperty = pfbxMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[j]);
					WriteIntString("<Property>:", j, FbxLayerElement::sTextureChannelNames[j], ReplaceBlank(fbxProperty.GetName(), '_'));
//0:DiffuseColor(FbxSurfaceMaterial::sDiffuse), 1:DiffuseFactor, 2:EmissiveColor, 3:EmissiveFactor, 4:AmbientColor, 5:AmbientFactor, 6:SpecularColor, 7:SpecularFactor, 8:ShininessExponent
//9:NormalMap, 10:Bump, 11:TransparentColor, 12:TransparencyFactor, 13:ReflectionColor, 14:ReflectionFactor, 15:DisplacementColor, 16:VectorDisplacementColor
					FindAndWriteTextureInfoByProperty(pfbxMaterial, &fbxProperty);
				}
				WriteString("</TextureProperties>");

				WriteString("<ShadingModel>:", ReplaceBlank((char *)pfbxMaterial->ShadingModel.Get().Buffer(), '_'));

				FbxString fbxstrImplemenationType = "HLSL";
				const FbxImplementation *pfbxImplementation = GetImplementation(pfbxMaterial, FBXSDK_IMPLEMENTATION_HLSL);
				if (!pfbxImplementation)
				{
					pfbxImplementation = GetImplementation(pfbxMaterial, FBXSDK_IMPLEMENTATION_CGFX);
					fbxstrImplemenationType = "CGFX";
				}
				if (pfbxImplementation)
				{
					WriteHardwareShaderImplementation(pfbxMaterial, pfbxImplementation, fbxstrImplemenationType);
				}
				else if(pfbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
				{
					WriteString("<Phong>:");

					FbxSurfacePhong *pfbxSurfacePhong = (FbxSurfacePhong *)pfbxMaterial;
					WriteColor(pfbxSurfacePhong->Ambient);
					WriteColor(pfbxSurfacePhong->Diffuse);
					WriteColor(pfbxSurfacePhong->Specular);
					WriteColor(pfbxSurfacePhong->Emissive);
					WriteFloat((float)pfbxSurfacePhong->TransparencyFactor);
					WriteFloat((float)pfbxSurfacePhong->Shininess);
					WriteFloat((float)pfbxSurfacePhong->ReflectionFactor);
				}
				else if(pfbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
				{
					WriteString("<Lambert>:");

					FbxSurfaceLambert *pfbxSurfaceLambert = (FbxSurfaceLambert *)pfbxMaterial;
					WriteColor(pfbxSurfaceLambert->Ambient);
					WriteColor(pfbxSurfaceLambert->Diffuse);
					WriteColor(pfbxSurfaceLambert->Emissive);
					WriteFloat((float)pfbxSurfaceLambert->TransparencyFactor);
				}
				else
				{
					WriteString("<Unknown>");
				}
			}
		}
		WriteString("</Materials>");
	}
}

void WriteControlPointUVs(FbxMesh *pfbxMesh, int nControlPoints)
{
	FbxVector2 *pControlPointUVs = new FbxVector2[nControlPoints];

	int nUVsPerVertex = pfbxMesh->GetElementUVCount(); //UVs Per Polygon's Vertex
	WriteInt("<UVs>:", nControlPoints, nUVsPerVertex);

	if (nUVsPerVertex > 0)
	{
		int nPolygons = pfbxMesh->GetPolygonCount();

		for (int k = 0; k < nUVsPerVertex; k++)
		{
			WriteInt("<UV>:", k);
			FbxGeometryElementUV *pfbxElementUV = pfbxMesh->GetElementUV(k);
			for (int i = 0; i < nPolygons; i++)
			{
				int nPolygonSize = pfbxMesh->GetPolygonSize(i); //Triangle: 3, Triangulate()
				for (int j = 0; j < nPolygonSize; j++)
				{
					int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
					switch (pfbxElementUV->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
						{
							switch (pfbxElementUV->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									pControlPointUVs[nControlPointIndex] = pfbxElementUV->GetDirectArray().GetAt(nControlPointIndex);
									break;
								case FbxGeometryElement::eIndexToDirect:
									pControlPointUVs[nControlPointIndex] = pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(nControlPointIndex));
									break;
								default:
									break;
							}
							break;
						}
						case FbxGeometryElement::eByPolygonVertex:
						{
							int nTextureUVIndex = pfbxMesh->GetTextureUVIndex(i, j);
							switch (pfbxElementUV->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
								case FbxGeometryElement::eIndexToDirect:
									pControlPointUVs[nControlPointIndex] = pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex);
									break;
								default:
									break;
							}
							break;
						}
						case FbxGeometryElement::eByPolygon:
						case FbxGeometryElement::eAllSame:
						case FbxGeometryElement::eNone:
							break;
						default:
							break;
					}
				}
			}

			for (int i = 0; i < nControlPoints; i++) WriteUV2DVector(pControlPointUVs[i]);
		}
	}
}

void WriteControlPointNormals(FbxMesh *pfbxMesh, int nControlPoints)
{
	FbxVector4 *pControlPointNormals = new FbxVector4[nControlPoints];

	int nNormalsPerVertex = pfbxMesh->GetElementNormalCount();
	WriteInt("<Normals>:", nControlPoints, nNormalsPerVertex);

	if (nNormalsPerVertex > 0)
	{
		int nPolygons = pfbxMesh->GetPolygonCount();

		for (int k = 0; k < nNormalsPerVertex; k++)
		{
			WriteInt("<Normal>:", k);

			FbxGeometryElementNormal *pfbxElementNormal = pfbxMesh->GetElementNormal(k);
			if (pfbxElementNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				if (pfbxElementNormal->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					for (int i = 0; i < nControlPoints; i++) pControlPointNormals[i] = pfbxElementNormal->GetDirectArray().GetAt(i);
				}
			}
			else if (pfbxElementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				for (int i = 0, nVertexID = 0; i < nPolygons; i++)
				{
					int nPolygonSize = pfbxMesh->GetPolygonSize(i);
					for (int j = 0; j < nPolygonSize; j++, nVertexID++)
					{
						int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
						switch (pfbxElementNormal->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								pControlPointNormals[nControlPointIndex] = pfbxElementNormal->GetDirectArray().GetAt(nVertexID);
								break;
							case FbxGeometryElement::eIndexToDirect:
								pControlPointNormals[nControlPointIndex] = pfbxElementNormal->GetDirectArray().GetAt(pfbxElementNormal->GetIndexArray().GetAt(nVertexID));
								break;
							default:
								break;
						}
					}
				}
			}

			for (int i = 0; i < nControlPoints; i++) Write3DVector(pControlPointNormals[i]);
		}
	}
}

void WriteControlPointTangents(FbxMesh *pfbxMesh, int nControlPoints)
{
	FbxVector4 *pControlPointTangents = new FbxVector4[nControlPoints];

	int nTangentsPerVertex = pfbxMesh->GetElementTangentCount();
	WriteInt("<Tangents>:", nControlPoints, nTangentsPerVertex);

	if (nTangentsPerVertex > 0)
	{
		int nPolygons = pfbxMesh->GetPolygonCount();

		for (int k = 0; k < nTangentsPerVertex; k++)
		{
			WriteInt("<Tangent>:", k);

			FbxGeometryElementTangent *pfbxElementTangent = pfbxMesh->GetElementTangent(k);
			if (pfbxElementTangent->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				if (pfbxElementTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					for (int i = 0; i < nControlPoints; i++) pControlPointTangents[i] = pfbxElementTangent->GetDirectArray().GetAt(i);
				}
			}
			else if (pfbxElementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				for (int i = 0, nVertexID = 0; i < nPolygons; i++)
				{
					int nPolygonSize = pfbxMesh->GetPolygonSize(i);
					for (int j = 0; j < nPolygonSize; j++, nVertexID++)
					{
						int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
						switch (pfbxElementTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								pControlPointTangents[nControlPointIndex] = pfbxElementTangent->GetDirectArray().GetAt(nVertexID);
								break;
							case FbxGeometryElement::eIndexToDirect:
								pControlPointTangents[nControlPointIndex] = pfbxElementTangent->GetDirectArray().GetAt(pfbxElementTangent->GetIndexArray().GetAt(nVertexID));
								break;
							default:
								break;
						}
					}
				}
			}

			for (int i = 0; i < nControlPoints; i++) Write3DVector(pControlPointTangents[i]);
		}
	}
}

void WriteControlPointBiTangents(FbxMesh *pfbxMesh, int nControlPoints)
{
	FbxVector4 *pControlPointBiTangents = new FbxVector4[nControlPoints];

	int nBiTangentsPerVertex = pfbxMesh->GetElementBinormalCount();
	WriteInt("<BiTangents>:", nControlPoints, nBiTangentsPerVertex);

	if (nBiTangentsPerVertex > 0)
	{
		int nPolygons = pfbxMesh->GetPolygonCount();

		for (int k = 0; k < nBiTangentsPerVertex; k++)
		{
			WriteInt("<BiTangent>:", k);

			FbxGeometryElementBinormal *pfbxElementBiTangent = pfbxMesh->GetElementBinormal(k);
			if (pfbxElementBiTangent->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				if (pfbxElementBiTangent->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					for (int i = 0; i < nControlPoints; i++) pControlPointBiTangents[i] = pfbxElementBiTangent->GetDirectArray().GetAt(i);
				}
			}
			else if (pfbxElementBiTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				for (int i = 0, nVertexID = 0; i < nPolygons; i++)
				{
					int nPolygonSize = pfbxMesh->GetPolygonSize(i);
					for (int j = 0; j < nPolygonSize; j++, nVertexID++)
					{
						int nControlPointIndex = pfbxMesh->GetPolygonVertex(i, j);
						switch (pfbxElementBiTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
								pControlPointBiTangents[nControlPointIndex] = pfbxElementBiTangent->GetDirectArray().GetAt(nVertexID);
								break;
							case FbxGeometryElement::eIndexToDirect:
								pControlPointBiTangents[nControlPointIndex] = pfbxElementBiTangent->GetDirectArray().GetAt(pfbxElementBiTangent->GetIndexArray().GetAt(nVertexID));
								break;
							default:
								break;
						}
					}
				}
			}

			for (int i = 0; i < nControlPoints; i++) Write3DVector(pControlPointBiTangents[i]);
		}
	}
}

void WriteControlPoints(FbxMesh *pfbxMesh, int nControlPoints)
{
	WriteInt("<ControlPoints>:", nControlPoints);
	FbxVector4 *pfbxvControlPoints = pfbxMesh->GetControlPoints();
	for (int i = 0; i < nControlPoints; i++) Write3DVector(pfbxvControlPoints[i]);
}

void WriteMesh(FbxMesh *pfbxMesh)
{
	int nControlPoints = pfbxMesh->GetControlPointsCount();
	if (nControlPoints > 0)
	{
		WriteControlPoints(pfbxMesh, nControlPoints);

		WriteControlPointUVs(pfbxMesh, nControlPoints);
		//WriteControlPointNormals(pfbxMesh, nControlPoints);
		//WriteControlPointTangents(pfbxMesh, nControlPoints);
		//WriteControlPointBiTangents(pfbxMesh, nControlPoints);
	}

	int nPolygons = pfbxMesh->GetPolygonCount();
	if (nPolygons > 0) WritePolygons(pfbxMesh, nPolygons);
}

void WriteSkinDeformations(FbxMesh *pfbxMesh)
{
	FbxSkin *pfbxSkinDeformer = (FbxSkin *)pfbxMesh->GetDeformer(0, FbxDeformer::eSkin);
	int nClusters = pfbxSkinDeformer->GetClusterCount();

	WriteInt("<BoneNames>:", nClusters);

	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		FbxNode *pfbxClusterLinkNode = pfbxCluster->GetLink();
		WriteString(ReplaceBlank((char *)pfbxClusterLinkNode->GetName(), '_'));
	}

	WriteInt("<BoneOffsets>:", nClusters);

	FbxAMatrix fbxmtxGeometryOffset = GetGeometricOffsetTransform(pfbxMesh->GetNode());
	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		FbxAMatrix fbxmtxBindPoseMeshToRoot; //Cluster Transform
		pfbxCluster->GetTransformMatrix(fbxmtxBindPoseMeshToRoot);
		FbxAMatrix fbxmtxBindPoseBoneToRoot; //Cluster Link Transform
		pfbxCluster->GetTransformLinkMatrix(fbxmtxBindPoseBoneToRoot);

		FbxAMatrix fbxmtxVertextToLinkNode = fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot * fbxmtxGeometryOffset;

		WriteMatrix(fbxmtxVertextToLinkNode);
	}

	int nControlPoints = pfbxMesh->GetControlPointsCount();

	int *pnBonesPerVertex = new int[nControlPoints];
	::memset(pnBonesPerVertex, 0, nControlPoints * sizeof(int));

	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();
		int *pnControlPointIndices = pfbxCluster->GetControlPointIndices();
		for (int k = 0; k < nControlPointIndices; k++) pnBonesPerVertex[pnControlPointIndices[k]] += 1;
	}
//	int nMaxBonesPerVertex = 0;
//	for (int i = 0; i < nControlPoints; i++) if (pnBonesPerVertex[i] > nMaxBonesPerVertex) nMaxBonesPerVertex = pnBonesPerVertex[i];

	int **ppnBoneIDs = new int*[nControlPoints];
	float **ppnBoneWeights = new float*[nControlPoints];
	for (int i = 0; i < nControlPoints; i++)
	{
		ppnBoneIDs[i] = new int[pnBonesPerVertex[i]];
		ppnBoneWeights[i] = new float[pnBonesPerVertex[i]];
		::memset(ppnBoneIDs[i], 0, pnBonesPerVertex[i] * sizeof(int));
		::memset(ppnBoneWeights[i], 0, pnBonesPerVertex[i] * sizeof(float));
	}

	int *pnBones = new int[nControlPoints];
	::memset(pnBones, 0, nControlPoints * sizeof(int));

	for (int j = 0; j < nClusters; j++)
	{
		FbxCluster *pfbxCluster = pfbxSkinDeformer->GetCluster(j);

		int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();
		int *pnControlPointIndices = pfbxCluster->GetControlPointIndices();
		double *pfControlPointWeights = pfbxCluster->GetControlPointWeights();

		for (int k = 0; k < nControlPointIndices; k++)
		{
			int nVertex = pnControlPointIndices[k];
			ppnBoneIDs[nVertex][pnBones[nVertex]] = j;
			ppnBoneWeights[nVertex][pnBones[nVertex]] = (float)pfControlPointWeights[k];
			pnBones[nVertex] += 1;
		}
	}
					
	for (int i = 0; i < nControlPoints; i++)
	{
		for (int j = 0; j < pnBonesPerVertex[i] - 1; j++)
		{
			for (int k = j + 1; k < pnBonesPerVertex[i]; k++)
			{
				if (ppnBoneWeights[i][j] < ppnBoneWeights[i][k])
				{
					float fTemp = ppnBoneWeights[i][j];
					ppnBoneWeights[i][j] = ppnBoneWeights[i][k];
					ppnBoneWeights[i][k] = fTemp;
					int nTemp = ppnBoneIDs[i][j];
					ppnBoneIDs[i][j] = ppnBoneIDs[i][k];
					ppnBoneIDs[i][k] = nTemp;
				}
			}
		}
	}

	int (*pnSkinningIndices)[4] = new int[nControlPoints][4];
	float (*pfSkinningWeights)[4] = new float[nControlPoints][4];

	for (int i = 0; i < nControlPoints; i++)
	{
		::memset(pnSkinningIndices[i], 0, 4 * sizeof(int));
		::memset(pfSkinningWeights[i], 0, 4 * sizeof(float));

		for (int j = 0; j < pnBonesPerVertex[i]; j++)
		{
			if (j < 4)
			{
				pnSkinningIndices[i][j] = ppnBoneIDs[i][j];
				pfSkinningWeights[i][j] = ppnBoneWeights[i][j];
			}
		}
	}

	for (int i = 0; i < nControlPoints; i++)
	{
		float nSumOfBoneWeights = 0.0f;
		for (int j = 0; j < 4; j++) nSumOfBoneWeights += pfSkinningWeights[i][j];
		for (int j = 0; j < 4; j++) pfSkinningWeights[i][j] /= nSumOfBoneWeights;
	}

	WriteInt("<BoneIndices>:", nControlPoints);
	for (int i = 0; i < nControlPoints; i++)
	{
		WriteInt(pnSkinningIndices[i][0], pnSkinningIndices[i][1], pnSkinningIndices[i][2], pnSkinningIndices[i][3]);
	}
	WriteInt("<BoneWeights>:", nControlPoints);
	for (int i = 0; i < nControlPoints; i++)
	{
		WriteFloat(pfSkinningWeights[i][0], pfSkinningWeights[i][1], pfSkinningWeights[i][2], pfSkinningWeights[i][3]);
	}

	for (int i = 0; i < nControlPoints; i++)
	{
		if (ppnBoneIDs[i]) delete[] ppnBoneIDs[i];
		if (ppnBoneWeights[i]) delete[] ppnBoneWeights[i];
	}
	if (ppnBoneIDs) delete[] ppnBoneIDs;
	if (ppnBoneWeights) delete[] ppnBoneWeights;

	if (pnBones) delete[] pnBones;
	if (pnBonesPerVertex) delete[] pnBonesPerVertex;

	if (pnSkinningIndices) delete[] pnSkinningIndices;
	if (pfSkinningWeights) delete[] pfSkinningWeights;
}

void WriteHierarchy(int *pnFrame, FbxNode *pfbxNode)
{
	WriteIntString("<Frame>:", *pnFrame, ReplaceBlank(pfbxNode->GetName(), '_'));

	FbxAMatrix fbxmtxLocalTransform = pfbxNode->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);

	FbxDouble3 T = fbxmtxLocalTransform.GetT();
	FbxDouble3 R = fbxmtxLocalTransform.GetR();
	FbxDouble3 S = fbxmtxLocalTransform.GetS();

	//FbxDouble3 T = pfbxNode->LclTranslation.Get();
	//FbxDouble3 R = pfbxNode->LclRotation.Get();
	//FbxDouble3 S = pfbxNode->LclScaling.Get();

	R[0] = R[0] * (3.1415926f / 180.f);
	R[1] = R[1] * (3.1415926f / 180.f);
	R[2] = R[2] * (3.1415926f / 180.f);

	WriteTransform("<Transform>:", fbxmtxLocalTransform, S, R, T);

	FbxNodeAttribute *pfbxNodeAttribute = pfbxNode->GetNodeAttribute();
	if (pfbxNodeAttribute)
	{
		FbxNodeAttribute::EType nAttributeType = pfbxNodeAttribute->GetAttributeType();
		if (nAttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh *pfbxMesh = (FbxMesh *)pfbxNode->GetNodeAttribute();
			int nSkinDeformers = pfbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (nSkinDeformers > 0)
			{
				WriteString("<SkinDeformations>:");
				WriteSkinDeformations(pfbxMesh);
				WriteString("</SkinDeformations>");
			}

			WriteString("<Mesh>:", ReplaceBlank((char*)pfbxMesh->GetName(), '_'));
			WriteMesh(pfbxMesh);
			WriteString("</Mesh>");

			WriteMaterials(pfbxMesh);
			//WriteTexture(pfbxMesh);
		}
	}

	int nChilds = pfbxNode->GetChildCount();
	WriteInt("<Children>:", nChilds);

    for (int i = 0; i < nChilds; i++)
    {
		(*pnFrame)++;
        WriteHierarchy(pnFrame, pfbxNode->GetChild(i));
    }

	WriteString("</Frame>");
}

void WriteHierarchy(FbxScene *pfbxScene)
{
    FbxNode *pfbxRootNode = pfbxScene->GetRootNode();

	int nFrame = 0;
    for (int i = 0; i < pfbxRootNode->GetChildCount(); i++, nFrame++)
    {
        WriteHierarchy(&nFrame, pfbxRootNode->GetChild(i));
    }
}


