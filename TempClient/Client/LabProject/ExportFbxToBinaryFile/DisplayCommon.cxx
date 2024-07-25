/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayCommon.h"
#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void WriteMetaDataConnections(FbxObject *pfbxObject)
{
	int nbMetaData = pfbxObject->GetSrcObjectCount<FbxObjectMetaData>();
    if (nbMetaData > 0)
        WriteString("    MetaData connections ");

    for (int i = 0; i < nbMetaData; i++)
    {
        FbxObjectMetaData* metaData = pfbxObject->GetSrcObject<FbxObjectMetaData>(i);
        WriteString("        Name:", (char*)metaData->GetName());
    }
}

FbxString gfbxString;

char *ReplaceBlank(const char *pstrValue, const char chReplace)
{
	int nLength = (int)strlen(pstrValue);
	if (nLength > 0)
	{
		gfbxString = pstrValue;
		gfbxString.ReplaceAll(' ', chReplace);
	}
	else
	{
		gfbxString = "Null";
	}
	return(gfbxString.Buffer());
}

void WriteString(const char *pstrValue)
{
	int nLength = (int)strlen(pstrValue);
	::fwrite(&nLength, sizeof(int), 1, gpBinaryFile);
	::fwrite(pstrValue, sizeof(char), nLength, gpBinaryFile);
}

void WriteString(char *pstrValue)
{
	int nLength = (int)strlen(pstrValue);
	::fwrite(&nLength, sizeof(int), 1, gpBinaryFile);
	::fwrite(pstrValue, sizeof(char), nLength, gpBinaryFile);
}

void WriteString(char *pszHeader, const char *pstrValue)
{
	WriteString(pszHeader);
	WriteString(pstrValue);
}

void WriteString(char *pszHeader, char *pstrValue1, char *pstrValue2, char *pstrValue3)
{
	WriteString(pszHeader);
	WriteString(pstrValue1);
	WriteString(pstrValue2);
	WriteString(pstrValue3);
}

void WriteIntString(int iValue, const char *pstrValue)
{
	::fwrite(&iValue, sizeof(int), 1, gpBinaryFile);
	WriteString(pstrValue);
}

void WriteIntString(char *pszHeader, int iValue, const char *pstrValue)
{
	WriteString(pszHeader);
	::fwrite(&iValue, sizeof(int), 1, gpBinaryFile);
	WriteString(pstrValue);
}

void WriteIntString(char *pszHeader, int iValue, const char *pstrValue1, const char *pstrValue2)
{
	WriteString(pszHeader);
	::fwrite(&iValue, sizeof(int), 1, gpBinaryFile);
	WriteString(pstrValue1);
	WriteString(pstrValue2);
}

void WriteIntString(char *pszHeader, int value1, int value2, int value3, char *pszString)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(int), 1, gpBinaryFile);
	WriteString(pszString);
}

void WriteIntString(char *pszHeader, int value1, int value2, const char *const pszString)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	WriteString(pszString);
}

void WriteBool(bool bValue)
{
    int nValue = (bValue) ? 1 :0;
	::fwrite(&nValue, sizeof(int), 1, gpBinaryFile);
}

void WriteBool(char *pszHeader, bool bValue)
{
	WriteString(pszHeader);
	WriteBool(bValue);
}

void WriteInt(int iValue)
{
	::fwrite(&iValue, sizeof(int), 1, gpBinaryFile);
}

void WriteInt(char *pszHeader, int iValue)
{
	WriteString(pszHeader);
	::fwrite(&iValue, sizeof(int), 1, gpBinaryFile);
}

void WriteInt(char *pszHeader, int value1, int value2)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
}

void WriteIntFloat(char *pszHeader, int value1, int value2, float value3)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void WriteInt(char *pszHeader, int value1, int value2, int value3)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(int), 1, gpBinaryFile);
}

void WriteInt(char *pszHeader, int value1, int value2, int value3, int value4)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(int), 1, gpBinaryFile);
}

void WriteInt(int value1, int value2, int value3, int value4)
{
	::fwrite(&value1, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(int), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(int), 1, gpBinaryFile);
}

void WriteDouble(double value)
{
	::fwrite(&value, sizeof(double), 1, gpBinaryFile);
}

void WriteFloat(float value)
{
	::fwrite(&value, sizeof(float), 1, gpBinaryFile);
}

void WriteDouble(char *pszHeader, double value)
{
	WriteString(pszHeader);
	::fwrite(&value, sizeof(double), 1, gpBinaryFile);
}

void WriteFloat(char *pszHeader, float value)
{
	WriteString(pszHeader);
	::fwrite(&value, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(float value1, float value2)
{
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(double value)
{
	float fValue = (float)value;
	::fwrite(&fValue, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(double value1, double value2)
{
	float fValue1 = (float)value1;
	float fValue2 = (float)value2;
	::fwrite(&fValue1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue2, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(double value1, double value2, double value3)
{
	float fValue1 = (float)value1;
	float fValue2 = (float)value2;
	float fValue3 = (float)value3;
	::fwrite(&fValue1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue3, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(double value1, double value2, double value3, double value4)
{
	float fValue1 = (float)value1;
	float fValue2 = (float)value2;
	float fValue3 = (float)value3;
	float fValue4 = (float)value4;
	::fwrite(&fValue1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue3, sizeof(float), 1, gpBinaryFile);
	::fwrite(&fValue4, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(char *pszHeader, float value1, float value2)
{
	WriteString(pszHeader);
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void WriteFloat(float value1, float value2, float value3, float value4)
{
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(float), 1, gpBinaryFile);
}

void WriteUV2DVector(FbxVector2 value)
{
	float value1 = static_cast<float>(value.mData[0]);
	float value2 = 1.0 - static_cast<float>(value.mData[1]);
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void Write2DVector(FbxVector2 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void Write2DVector(char *pszHeader, FbxVector2 value)
{
	WriteString(pszHeader);
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void Write3DVector(FbxVector4 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void Write3DVector(char *pszHeader, FbxVector4 value)
{
	WriteString(pszHeader);
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void Write4DVector(FbxVector4 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	float value4 = (float)value[3];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(float), 1, gpBinaryFile);
}

void Write4DVector(char *pszHeader, FbxVector4 value)
{
	WriteString(pszHeader);
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	float value4 = (float)value[3];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(float), 1, gpBinaryFile);
}

void WriteMatrix(FbxAMatrix value)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) WriteFloat((float)value[i][j]);
	}
}

void WriteMatrix(char *pszHeader, FbxAMatrix value)
{
	WriteString(pszHeader);
	WriteMatrix(value);
}

void Write2DVector(FbxDouble2 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
}

void Write3DVector(FbxDouble3 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void Write4DVector(FbxDouble4 value)
{
	float value1 = (float)value[0];
	float value2 = (float)value[1];
	float value3 = (float)value[2];
	float value4 = (float)value[3];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
	::fwrite(&value4, sizeof(float), 1, gpBinaryFile);
}

void Write2DVector(char *pszHeader, FbxDouble2 value)
{
	WriteString(pszHeader);
	Write2DVector(value);
}

void Write3DVector(char *pszHeader, FbxDouble3 value)
{
	WriteString(pszHeader);
	Write3DVector(value);
}

void Write4DVector(char *pszHeader, FbxDouble4 value)
{
	WriteString(pszHeader);
	Write4DVector(value);
}

void WriteMatrix(char *pszHeader, FbxDouble4x4 value)
{
	WriteString(pszHeader);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) WriteFloat((float)value[i][j]);
	}
}

void WriteTransform(char *pszHeader, FbxAMatrix value0, FbxDouble3 value1, FbxDouble3 value2, FbxDouble3 value3)
{
	WriteString(pszHeader);
	WriteMatrix(value0);
	Write3DVector(value1);
	Write3DVector(value2);
	Write3DVector(value3);
}

void WriteColor(FbxPropertyT<FbxDouble3> value)
{
	float value1 = (float)value.Get()[0];
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	float value2 = (float)value.Get()[1];
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	float value3 = (float)value.Get()[2];
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void WriteColor(char *pszHeader, FbxPropertyT<FbxDouble3> value)
{
	WriteString(pszHeader);
	WriteColor(value);
}

void WriteColor(FbxColor value)
{
	float value1 = (float)value.mRed;
	::fwrite(&value1, sizeof(float), 1, gpBinaryFile);
	float value2 = (float)value.mGreen;
	::fwrite(&value2, sizeof(float), 1, gpBinaryFile);
	float value3 = (float)value.mBlue;
	::fwrite(&value3, sizeof(float), 1, gpBinaryFile);
}

void WriteColor(char *pszHeader, FbxColor value)
{
	WriteString(pszHeader);
	WriteColor(value);
}
