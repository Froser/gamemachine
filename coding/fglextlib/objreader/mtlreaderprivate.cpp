#include "stdafx.h"
#include "mtlreader.h"
#include "utilities/scanner.h"

#define KW_REMARK "#"
#define KW_NEWMATERIAL "newmtl"
#define KW_NS "Ns"
#define KW_D "d"
#define KW_TR "Tr"
#define KW_TF "Tf"
#define KW_ILLUM "illum"
#define KW_KA "Ka"
#define KW_KD "Kd"
#define KW_KS "Ks"

static bool strEqual(const char* str1, const char* str2)
{
	return !strcmp(str1, str2);
}

static bool isWhiteSpace(char c)
{
	return !!isspace(c);
}

Materials& MtlReaderPrivate::getMaterials()
{
	return m_materials;
}

void MtlReaderPrivate::parseLine(const char* line)
{
	Scanner scanner(line, isWhiteSpace);
	char command[LINE_MAX];
	scanner.next(command);

	if (strEqual(command, KW_REMARK))
		return;

	if (strEqual(command, KW_NEWMATERIAL))
	{
		char name[LINE_MAX];
		scanner.next(name);
		m_currentMaterial = &(m_materials[name]);
	}
	else if (strEqual(command, KW_NS))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ns = value;
	}
	else if (strEqual(command, KW_D))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->d = value;
	}
	else if (strEqual(command, KW_TR))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Tr = value;
	}
	else if (strEqual(command, KW_TF))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Tf_r = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Tf_g = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Tf_b = value;
	}
	else if (strEqual(command, KW_ILLUM))
	{
		Fint value;
		scanner.nextInt(&value);
		m_currentMaterial->illum = value;
	}
	else if (strEqual(command, KW_KA))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ka_r = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ka_g = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ka_b = value;
	}
	else if (strEqual(command, KW_KD))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Kd_r = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Kd_g = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Kd_b = value;
	}
	else if (strEqual(command, KW_KS))
	{
		Ffloat value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ks_r = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ks_g = value;
		scanner.nextFloat(&value);
		m_currentMaterial->Ks_b = value;
	}
}