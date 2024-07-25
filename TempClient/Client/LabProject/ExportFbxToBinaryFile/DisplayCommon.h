/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_COMMON_H
#define _DISPLAY_COMMON_H

#include <fbxsdk.h>

extern FILE *gpBinaryFile;

char *ReplaceBlank(const char *value, const char chReplace);

void WriteMetaDataConnections(FbxObject *pfbxNode);
void WriteString(const char *pstrValue);
void WriteString(char *pszHeader, const char *pString);
void WriteString(char *pszHeader, char *pstrValue1, char *pstrValue2, char *pstrValue3);
void WriteIntString(int iValue, const char *pstrValue);
void WriteIntString(char *pszHeader, int iValue, const char *sValue);
void WriteIntString(char *pszHeader, int iValue, const char *pstrValue1, const char *pstrValue2);
void WriteIntString(char *pszHeader, int value1, int value2, const char *const pszString);
void WriteIntString(char *pszHeader, int value1, int value2, int value3, char *pString);
void WriteBool(bool bValue);
void WriteBool(char *pszHeader, bool value);
void WriteInt(int value);
void WriteInt(char *pszHeader, int value);
void WriteInt(char *pszHeader, int value1, int value2);
void WriteIntFloat(char *pszHeader, int value1, int value2, float value3);
void WriteInt(char *pszHeader, int value1, int value2, int value3);
void WriteInt(int value1, int value2, int value3, int value4);
void WriteInt(char *pszHeader, int value1, int value2, int value3, int value4);
void WriteDouble(double value);
void WriteDouble(char *pszHeader, double value);
void WriteFloat(float value);
void WriteFloat(double value);
void WriteFloat(char *pszHeader, float value);
void WriteFloat(float value1, float value2);
void WriteFloat(double value1, double value2);
void WriteFloat(double value1, double value2, double value3);
void WriteFloat(char *pszHeader, float value1, float value2);
void WriteFloat(float value1, float value2, float value3, float value4);
void WriteFloat(double value1, double value2, double value3, double value4);
void WriteUV2DVector(FbxVector2 value);
void Write2DVector(FbxVector2 value);
void Write3DVector(FbxVector4 value);
void Write4DVector(FbxVector4 value);
void WriteMatrix(FbxAMatrix value);
void Write2DVector(char *pszHeader, FbxVector2 value);
void Write3DVector(char *pszHeader, FbxVector4 value);
void Write4DVector(char *pszHeader, FbxVector4 value);
void WriteMatrix(char *pszHeader, FbxAMatrix value);
void Write2DVector(char *pszHeader, FbxDouble2 value);
void Write4DVector(char *pszHeader, FbxDouble4 value);
void Write3DVector(char *pszHeader, FbxDouble3 value);
void WriteMatrix(char *pszHeader, FbxDouble4x4 value);
void WriteTransform(char *pszHeader, FbxAMatrix value0, FbxDouble3 value1, FbxDouble3 value2, FbxDouble3 value3);

void WriteColor(FbxPropertyT<FbxDouble3> value);
void WriteColor(FbxColor value);
void WriteColor(char *pszHeader, FbxPropertyT<FbxDouble3> value);
void WriteColor(char *pszHeader, FbxColor value);

extern void WriteStringToFile(char *pszBuffer);

#endif // #ifndef _DISPLAY_COMMON_H


