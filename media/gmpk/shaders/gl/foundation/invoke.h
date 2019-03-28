// 子程序选择，对应gmglshaderprogram.cpp@toTechniqueEntranceId
uniform int GM_TechniqueEntrance;
const int GM_ModelType_Model2D = 0;
const int GM_ModelType_Model3D = 1;
const int GM_ModelType_Text = 2;
const int GM_ModelType_CubeMap = 3;
const int GM_ModelType_Particle = 6;
const int GM_ModelType_Custom = 7;
const int GM_ModelType_Shadow = 8;

void GM_InvokeTechniqueEntrance()
{
	// 与GMModelType的顺序一致
	switch (GM_TechniqueEntrance)
	{
		case GM_ModelType_Model2D:
			GM_Model2D();
			break;
		case GM_ModelType_Model3D:
		case GM_ModelType_Custom:
			GM_Model3D();
			break;
		case GM_ModelType_Text:
			GM_Text();
			break;
		case GM_ModelType_CubeMap:
			GM_CubeMap();
			break;
		case GM_ModelType_Particle:
			GM_Particle();
			break;
		case GM_ModelType_Shadow:
			GM_Shadow();
			break;
	}
}
