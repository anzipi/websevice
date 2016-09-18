#include "ogrsf_frmts.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h" 
#include <string>
#include <iostream>
#include <strstream>
#include <exception>
using namespace std;

void convertToShp(double longitude, double latitude, char *outshp)
{
	
   // double *TransferedLongLat = transfer(longitude, latitude);	
	
	//ʹ���Ա��ֶ�֧������
	CPLSetConfigOption("SHAPE_ENCODING","");
	OGRRegisterAll();//ע�����е�����
	//����ESRI shp�ļ�
	char *pszDriverName = "ESRI Shapefile";
	//���ö�Shape�ļ���д��Driver
	OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);
	if (poDriver == NULL)
	{
		cout<<pszDriverName<<"���������ã�"<<endl;
		return;
	}
	//��������Դ
	OGRDataSource *poDs = poDriver->CreateDataSource(outshp, NULL);
	if (poDs == NULL)
	{
		cout<<"DataSource Creation Error"<<endl;
		return;
	}
	//����ͼ��Layer
	string outShapName = outshp;
	string layerName = outShapName.substr(0, outShapName.length()-4);
	//layerName.c_str()��ʾ��stringתΪchar *����
	//����˵������ͼ�����ƣ�����ϵ��ͼ��ļ������ͣ�����ѡ��������й�
	OGRLayer *poLayer = poDs->CreateLayer(layerName.c_str(), NULL, wkbPoint, NULL);
	if (poLayer == NULL)
	{
		cout<<"Layer Creation Failed"<<endl;
		OGRDataSource::DestroyDataSource(poDs);
		return;
	}
	//���洴�����Ա����������Ա��д����������ݼ���
	//�ȴ���һ����ID����������
	OGRFieldDefn oFieldId("ID", OFTInteger);
	oFieldId.SetWidth(10);
	poLayer->CreateField(&oFieldId);
	//name
	OGRFieldDefn oFieldname("Dist_moved", OFTString);
	oFieldId.SetWidth(50);
	poLayer->CreateField(&oFieldname);
	//�ٴ���һ��"X"double����
	OGRFieldDefn oFieldX("X", OFTString);
	oFieldX.SetWidth(50);
	poLayer->CreateField(&oFieldX);
	//�ٴ���һ��"Y"double����
	OGRFieldDefn oFieldY("Y", OFTString);//OFTReal
	oFieldY.SetWidth(50);
	poLayer->CreateField(&oFieldY);
	//����һ��feature
	OGRFeature *poFeature; 	
	poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());//GetLayerDefn()��ȡ��ǰͼ������Ա�ṹ
	//�����Ա������Ǹմ������и�ֵ
	int i = 0;
	poFeature->SetField("ID", i);
	poFeature->SetField("Dist_moved", i);
	poFeature->SetField("X", longitude);
	poFeature->SetField("Y", latitude);
	i++;
	//����һ��OGRPoint����
	OGRPoint point;
	point.setX(longitude);
	point.setY(latitude);
	//point.setZ(0);
	
	poFeature->SetGeometry(&point);

	if(poLayer->CreateFeature(poFeature) != OGRERR_NONE )
	{
		printf( "Failed to create feature in shapefile.\n" );
		exit( 1 );
	}
	OGRFeature::DestroyFeature(poFeature);
	OGRDataSource::DestroyDataSource(poDs);
	
}
//======= ��γ��ת��ΪͶӰ����=============
double* transfer(double longitude, double latitude)
{
	OGRSpatialReference oSourceSRS;
	//EPSG code �����ض��������塢��λ����������ϵ��ͶӰ����ϵ����Ϣ
	//This method will initialize the spatial reference based on the passed in EPSG GCS or PCS code
	oSourceSRS.importFromEPSG(4326);//EPSG:4326�����������ϵWGS1984
	OGRSpatialReference oTargetSRS;
	oTargetSRS.importFromEPSG(2029);
	OGRCoordinateTransformation *poTransform;
	poTransform = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);
	if (poTransform == NULL)
	{
		cout<<"poTransform is null"<<endl;
		exit(1);
	}	
	if (!poTransform->Transform(1, &longitude, &latitude, NULL))
	{
		cout<<"transform failed"<<endl;
		exit(1);
	}
	//poTransform->Transform(1, &longitude, &latitude, NULL);
	double *inout = new double[2];
	inout[0] = longitude;
	inout[1] = latitude;
	return inout;
}
//��������������transfer�����Ĺ�����һ����
double* transfer2(double longitude, double latitude)
{
	OGRSpatialReference oSourceSRS;	
	oSourceSRS.SetWellKnownGeogCS( "WGS84" );
	OGRSpatialReference oTargetSRS;	
	oTargetSRS.SetWellKnownGeogCS("WGS84");
	oTargetSRS.SetUTM(17);	
	OGRCoordinateTransformation *poTransform;
	poTransform = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);
	if (poTransform == NULL)
	{
		cout<<"poTransform is null"<<endl;
		exit(1);
	}	
	if (!poTransform->Transform(1, &longitude, &latitude))
	{
		cout<<"transform failed"<<endl;
		exit(1);
	}
	//poTransform->Transform(1, &longitude, &latitude, NULL);
	double *inout = new double[2];
	inout[0] = longitude;
	inout[1] = latitude;
	return inout;
}
int main(int argc, char *argv[])
{		
	char *xCoordinate = argv[1];
	char *yCoordinate = argv[2];
	char *outShp = argv[3];
	double x = atof(xCoordinate);
	double y = atof(yCoordinate);
	convertToShp(x, y, outShp);
	cout<<"success! file is saved to "<<outShp<<endl;

	/*
	double *transferedLongLat = transfer(116.246742, 40.022211);
	double *transferedLongLat2 = transfer2(116.246742, 40.022211);
	cout<<"ת�����ͶӰ����Ϊ��"<<transferedLongLat[0]<<","<<transferedLongLat[1]<<endl;	
	cout<<"ʹ�õڶ�������ת����"<<transferedLongLat2[0]<<","<<transferedLongLat2[1]<<endl;	
	*/
	/*double x = 735847.112853;
	double y = 3428713.99719;
	const char *outShp = "E:\\Exercise\\test\\convertResult\\outshp.shp";*/			
	
	getchar();
}