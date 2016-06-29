/*
* ���ܣ� �����դ�����ݽ��е�����ù������֣�����������֣�������������ֵ���߽�ʸ��SHP�ļ�
* ˵����
* ���������4��
*	��1���������դ�������ļ�������'#'�ָ
*	��2���������դ�������ļ�������'#'�ָ
*	��3�����������ʱդ�������ļ���������ʱ�ļ����ջ��Զ���ɾ����
*	��4�����������߽�SHP�����ļ�����
*
* ��������ֵ��
*	���� 0��  ��ʾ������ȷ����
*	���� 444����ʾ�ļ���ȡ���������������ļ�������������
*	���� 999����ʾ����Ĳ�����ʽ����ȷ
*	������    ��ʾ����ͼ��֮��û�н��������������ֵΪ��֮ǰͼ��û�н�����ͼ����ţ����磺����������ͼ����ǰ����ͼ���޽����������������ͼ������:2��
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "gdal_priv.h"
#include "ogrsf_frmts.h" //for ogr
#include "gdal_alg.h"	 //for GDALPolygonize
#include "cpl_conv.h"	 //for CPLMalloc()

using namespace std;

// �����������
void parseStr(string str, char c, vector<string>& tokens) {
	unsigned int posL = 0;
	unsigned int posR = 0;
	while(posR < str.length()-1) {
		posR = str.find_first_of(c, posL);
		string sub = str.substr(posL, posR-posL);
		tokens.push_back(sub);
		posL = posR + 1;
	}
}

// ��������ת���к�
bool Projection2ImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow)  
{  
	try  
	{  
		double dTemp = adfGeoTransform[1]*adfGeoTransform[5] - adfGeoTransform[2]*adfGeoTransform[4];  
		double dCol = 0.0, dRow = 0.0;  
		dCol = (adfGeoTransform[5]*(dProjX - adfGeoTransform[0]) -   
			adfGeoTransform[2]*(dProjY - adfGeoTransform[3])) / dTemp + 0.5;  
		dRow = (adfGeoTransform[1]*(dProjY - adfGeoTransform[3]) -   
			adfGeoTransform[4]*(dProjX - adfGeoTransform[0])) / dTemp + 0.5;  

		iCol = static_cast<int>(dCol);  
		iRow = static_cast<int>(dRow);  
		return true;  
	}  
	catch(...)  
	{  
		return false;  
	}  
}

// DEM���ݹ������ֲü�
int DEMCut(vector<string> inputRasterFileNames,vector<string> outputRasterFileNames)
{
	GDALAllRegister();

	// ��������
	vector<GDALDataset *> datasets;
	for(int i = 0; i < inputRasterFileNames.size(); i++)
	{
		GDALDataset *ds;
		ds = (GDALDataset*)GDALOpen(inputRasterFileNames[i].c_str(), GA_ReadOnly);
		if(ds != NULL)
		{
			datasets.push_back(ds);
		}
		else
		{
			return 444;		// �ļ���ȡ���󣬷���444
		}
	}

	// ��ȡ�������ֵķ�Χ left top right bottom
	// adfGeoTransform[6]  ����adfGeoTransform������Ƿ���任�е�һЩ�������ֱ������  
	// adfGeoTransform[0]  ���Ͻ�x����   
	// adfGeoTransform[1]  ��������ֱ���  
	// adfGeoTransform[2]  ��ת�Ƕ�, 0��ʾͼ�� "��������"  
	// adfGeoTransform[3]  ���Ͻ�y����   
	// adfGeoTransform[4]  ��ת�Ƕ�, 0��ʾͼ�� "��������"  
	// adfGeoTransform[5]  �ϱ�����ֱ���  
	double left = 0;
	double right = 0;
	double top = 0;
	double bottom = 0;
	for(int i = 0; i < datasets.size(); i++)
	{
		GDALDataset *ds;
		ds = datasets[i];
		double geoTransform[6];
		ds->GetGeoTransform(geoTransform);
		if(i == 0)
		{
			left = geoTransform[0];
			top = geoTransform[3];
			right = geoTransform[0] + geoTransform[1] * ds->GetRasterXSize() + geoTransform[2] * ds->GetRasterYSize();
			bottom = geoTransform[3] + geoTransform[4] * ds->GetRasterXSize() + geoTransform[5] * ds->GetRasterYSize();
		}
		else
		{
			double left_temp = geoTransform[0];
			double top_temp = geoTransform[3];
			double right_temp = geoTransform[0] + geoTransform[1] * ds->GetRasterXSize() + geoTransform[2] * ds->GetRasterYSize();
			double bottom_temp = geoTransform[3] + geoTransform[4] * ds->GetRasterXSize() + geoTransform[5] * ds->GetRasterYSize();

			// �ж��Ƿ��н���
			if(left_temp >= right || top_temp <= bottom || right_temp <= left || bottom_temp >= top)
			{
				return i;	// �����ú�������i��ͼ����֮ǰ��ͼ��û�н���
			}

			// �н���������ִ��
			if(left_temp > left)	// ȡleft���ֵ
			{
				left = left_temp;
			}
			if(right_temp < right)	// ȡright��Сֵ
			{
				right = right_temp;
			}
			if(top_temp < top)		// ȡtop��Сֵ
			{
				top = top_temp;
			}
			if(bottom_temp > bottom)// ȡbottom���ֵ
			{
				bottom = bottom_temp;
			}
		}
	}

	// ���ݱ߽��ȡÿ��դ���ļ���Ҫ�ü��������з�Χ��������ü����դ������
	vector<float*> pDataList;
	int pixelCount = 0;
	float noDataValue = datasets[0]->GetRasterBand(1)->GetNoDataValue();
	for(int i = 0; i < datasets.size(); i++)
	{
		GDALDataset *ds;
		ds = datasets[i];
		double geoTransform[6];
		ds->GetGeoTransform(geoTransform);
		int row_start, col_start, row_end, col_end, rowCount, colCount;
		Projection2ImageRowCol(geoTransform, left, top, col_start, row_start);	// ���ݵ�������ת���õ�������Ͻǵ����к�
		Projection2ImageRowCol(geoTransform, right, bottom, col_end, row_end);	// ���ݵ�������ת���õ��յ����½ǵ����к�
		rowCount = row_end - row_start;// + 1;
		colCount = col_end - col_start;// + 1;
		pixelCount = colCount*rowCount;

		// ��ȡ����
		float *pData = (float*)CPLMalloc(sizeof(float)*colCount*rowCount);
		GDALDataType dataType = ds->GetRasterBand(1)->GetRasterDataType();
		ds->RasterIO(GF_Read, col_start, row_start, colCount, rowCount, pData, colCount, rowCount, GDT_Float32, 1, 0, 0, 0, 0);

		pDataList.push_back(pData);
	}
	float *pBoundaryData = (float*)CPLMalloc(sizeof(float)*pixelCount);
	for(int i = 0; i < pixelCount; i++)
	{
		bool flag = true;
		for(int j = 0; j < pDataList.size(); j++)
		{
			float value = pDataList[j][i];
			if(value == noDataValue)
			{
				flag = false;
				break;
			}
		}
		if(flag == true)	// ������
		{
			pBoundaryData[i] = 1;
		}
		else	// û������,��դ��ֵΪnoDataValue
		{
			pBoundaryData[i] = 0;
		}
	}
	
	for(int i = 0; i < datasets.size(); i++)
	{
		GDALDataset *ds;
		ds = datasets[i];
		double geoTransform[6];
		ds->GetGeoTransform(geoTransform);
		int row_start, col_start, row_end, col_end, rowCount, colCount;
		Projection2ImageRowCol(geoTransform, left, top, col_start, row_start);	// ���ݵ�������ת���õ�������Ͻǵ����к�
		Projection2ImageRowCol(geoTransform, right, bottom, col_end, row_end);	// ���ݵ�������ת���õ��յ����½ǵ����к�
		rowCount = row_end - row_start;// + 1;
		colCount = col_end - col_start;// + 1;

		// �����������
		GDALDataset *output;
		GDALDriver  *pDriver;
		pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if(pDriver == NULL)
		{
			cout<<"GDAL DriverManager Error!\n";
			return 444;
		}
		output = pDriver->Create(outputRasterFileNames[i].c_str(), colCount, rowCount, 1, GDT_Float32, NULL);
		if(output == NULL)
		{
			cout<<"GDAL Create Error!\n";
			return 444;
		}

		for(int j = 0; j < pixelCount; j++)
		{
			if(pBoundaryData[j] == 0)
			{
				pDataList.at(i)[j] = noDataValue;
			}
		}

		double newGeotransform[6];
		newGeotransform[0] = left;
		newGeotransform[1] = geoTransform[1];
		newGeotransform[2] = geoTransform[2];
		newGeotransform[3] = top;
		newGeotransform[4] = geoTransform[4];
		newGeotransform[5] = geoTransform[5];
		output->SetGeoTransform(newGeotransform);
		output->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, colCount, rowCount, pDataList.at(i), colCount, rowCount, GDT_Float32, 0, 0);
		output->SetProjection(ds->GetProjectionRef());
		output->GetRasterBand(1)->SetNoDataValue(ds->GetRasterBand(1)->GetNoDataValue());

		GDALClose((GDALDatasetH)ds);		// �ر����ݼ�
		GDALClose((GDALDatasetH)output);	// �ر����ݼ�
	}
	return 0;
}

// ������������դ���ļ�
int CreateBoundaryRaster(vector<string> inputRasterFileNames, char *outputFileName)
{
	// ��������
	vector<GDALDataset *> datasets;
	for(int i = 0; i < inputRasterFileNames.size(); i++)
	{
		GDALDataset *ds;
		ds = (GDALDataset*)GDALOpen(inputRasterFileNames[i].c_str(), GA_ReadOnly);
		if(ds != NULL)
		{
			datasets.push_back(ds);
		}
		else
		{
			cout<<"�ļ���ȡ����\n";
			return 444;
		}
	}
	int colCount = datasets[0]->GetRasterXSize();
	int rowCount = datasets[0]->GetRasterYSize();
	GDALDataset *output;
	GDALDriver  *pDriver;
	pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if(pDriver == NULL)
	{
		cout<<"GDAL DriverManager Error!\n";
		return 444;
	}
	output = pDriver->Create(outputFileName, colCount, rowCount, 1, GDT_Float32, NULL);
	if(output == NULL)
	{
		cout<<"GDAL Create Error!\n";
		return 444;
	}
	
	float *pFinalData = (float*)CPLMalloc(sizeof(float)*colCount*rowCount);	// ����������դ�������
	vector<float *> pDataList;	// ���ÿ��դ��ͼ�������
	for(int i = 0; i < datasets.size(); i++)	// �洢��pDataList��
	{
		GDALDataset *ds;
		ds = datasets[i];
		float *pData = (float*)CPLMalloc(sizeof(float)*colCount*rowCount);
		ds->RasterIO(GF_Read, 0, 0, colCount, rowCount, pData, colCount, rowCount, GDT_Float32, 1, 0, 0, 0, 0);
		pDataList.push_back(pData);
	}

	float noDataValue = datasets[0]->GetRasterBand(1)->GetNoDataValue();
	for(int i = 0; i < colCount*rowCount; i++)
	{
		bool flag = true;	// �ж��Ƿ�������
		for(int j = 0; j < pDataList.size(); j++)
		{
			float value = pDataList[j][i];
			if(value == noDataValue)
			{
				flag = false;
				break;
			}
		}
		if(flag == true)	// ������
		{
			pFinalData[i] = 1;
		}
		else	// û������,��դ��ֵΪnoDataValue
		{
			pFinalData[i] = 0;
		}
	}
	
	GDALDataset *ds = datasets[0];
	output->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, colCount, rowCount, pFinalData, colCount, rowCount, GDT_Float32, 0, 0);
	double geoTransform[6];
	ds->GetGeoTransform(geoTransform);
	output->SetGeoTransform(geoTransform);
	output->SetProjection(ds->GetProjectionRef());
	output->GetRasterBand(1)->SetNoDataValue(0);

	for(int i = 0; i < datasets.size(); i++)	// �洢��pDataList��
	{
		GDALDataset *ds = datasets[i];
		GDALClose((GDALDatasetH)ds);		// �ر����ݼ�
	}
	
	GDALClose((GDALDatasetH)output);	// �ر����ݼ�
	return 0;
}

// ��������դ���ļ�ʸ����
int ImagePolygonize(char *inputFileName, char* outputFileName, const char* pszFormat)
{
	GDALAllRegister();
	OGRRegisterAll();	// �������ע��
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");

	GDALDataset* poSrcDS=(GDALDataset*)GDALOpen(inputFileName, GA_ReadOnly);
	if(poSrcDS==NULL)
	{
		return 444;
	}
	// �������ʸ���ļ�
	OGRSFDriver *poDriver;
	poDriver = (OGRSFDriver*)OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName( pszFormat );
	if (poDriver == NULL)
	{  
		GDALClose((GDALDatasetH)poSrcDS); 
		return 444;
	}
	poDriver->DeleteDataSource(outputFileName);	// �����ļ����ڣ�����ɾ�����е�shp�ļ�
	//�����ļ����������ʸ���ļ�
	OGRDataSource* poDstDS = poDriver->CreateDataSource(outputFileName, NULL);
	if (poDstDS==NULL)
	{
		GDALClose((GDALDatasetH)poSrcDS);
		return 444;
	}
	// ����ռ�ο���������ͼ����ͬ;
	OGRSpatialReference *poSpatialRef = new OGRSpatialReference(poSrcDS->GetProjectionRef());
	OGRLayer* poLayer = poDstDS->CreateLayer("boundary", poSpatialRef, wkbPolygon, NULL);
	if (poDstDS == NULL)
	{
		GDALClose((GDALDatasetH)poSrcDS); 
		OGRDataSource::DestroyDataSource(poDstDS); 
		delete poSpatialRef; 
		poSpatialRef = NULL; 
		return 444;
	}
	OGRFieldDefn ofieldDef("Segment", OFTInteger);	//�������Ա�ֻ��һ���ֶμ���Segment�������汣���Ӧ��դ�����Ԫֵ
	poLayer->CreateField(&ofieldDef);
	GDALRasterBandH hSrcBand = (GDALRasterBandH) poSrcDS->GetRasterBand(1);		//��ȡͼ��ĵ�һ������
	GDALPolygonize(hSrcBand, NULL, (OGRLayerH)poLayer, 0, NULL, NULL, NULL);	//����դ��ʸ����

	// ɾ��noDataValue���ֵ�Ҫ��
	OGRFeature *poFeature;
	poLayer->ResetReading();
	while( (poFeature = poLayer->GetNextFeature()) != NULL )	// ���Ҫ��
	{
		int value = poFeature->GetFieldAsInteger("Segment");
		if(value == 0)	// ��Ҫɾ����noDataҪ��
		{
			long fid = poFeature->GetFID();
			poLayer->DeleteFeature(fid);
		}
	}
	OGRFeatureDefn *pFeatureDefn = poLayer->GetLayerDefn();
	std::string strLayerName = pFeatureDefn->GetName();		// ��ȡ��ͼ�������
	std::string strSQL = "REPACK " + strLayerName;
	poDstDS->ExecuteSQL(strSQL.c_str(), NULL, "");
	
	GDALClose(poSrcDS); // �ر��ļ�
	OGRDataSource::DestroyDataSource(poDstDS);
	return 0;
}

// ɾ����ʱ�ļ�
void DeleteTempFile(char *filename)
{
	remove(filename);
}

// ��Ҫ�Ĵ�����
int DEMProcessing(vector<string> inputRasterFileNames, vector<string> outputRasterFileNames,
				   char *tempRasterFileName, char *shpFileName)
{
	int result = DEMCut(inputRasterFileNames, outputRasterFileNames);		// DEM���ݹ������ֲü�
	if(result != 0)
	{
		return result;		// ��������������ͼ��֮��û�н��������������ͼ�����
	}
	CreateBoundaryRaster(outputRasterFileNames, tempRasterFileName);		// ������������դ���ļ�
	ImagePolygonize(tempRasterFileName, shpFileName, "ESRI Shapefile");		// ��������դ���ļ�ʸ����
	DeleteTempFile(tempRasterFileName);										// ɾ����ʱ�ļ�
	return 0;
}


int main(int argc, char *argv[])
{
	// ******************* ����׼��--����������� ******************* //

	// ����ʾ��
	//argc = 5;
	//argv[1] = "D:/data/demcut/dem1.tif#D:/data/demcut/dem2.tif";
	//argv[2] = "D:/RasterCut/output/dem1.tif#D:/RasterCut/output/dem2.tif";
	//argv[3] = "D:/RasterCut/output/tempRaster.tif";
	//argv[4] = "D:/RasterCut/output/boundary.shp";
	//char *argument = "D:/data/demcut/twi.tif#D:/data/demcut/slope.tif#D:/data/demcut/plan.tif#D:/data/demcut/Elevation.asc D:/RasterCut/output/twi.tif#D:/RasterCut/output/slope.tif#D:/RasterCut/output/plan.tif#D:/RasterCut/output/Elevation.tif D:/RasterCut/output/tempRaster.tif D:/RasterCut/output/boundary.shp";
	//char *argument = "D:/data/demcut/dem1.tif#D:/data/demcut/dem2.tif D:/RasterCut/output/dem1.tif#D:/RasterCut/output/dem2.tif D:/RasterCut/output/tempRaster.tif D:/RasterCut/output/boundary.shp";

	if(argc-1 != 4) // �ж�������������Ƿ�Ϊ4
	{
		cout<<"������Ĳ�������\n";
		return 999;		// ����������󣬷���999
	}

	char *str_inputRasterFileNames = argv[1];
	char *str_outputRasterFileNames = argv[2];
	char *str_tempRasterFileName = argv[3];
	char *str_shpFileName = argv[4];

	vector<string> inputRasterFileNames;
	parseStr(str_inputRasterFileNames, '#', inputRasterFileNames);
	vector<string> outputRasterFileNames;
	parseStr(str_outputRasterFileNames, '#', outputRasterFileNames);
	char *tempRasterFileName = str_tempRasterFileName;
	char *shpFileName = str_shpFileName;

	// ******************* ����׼��--��� ******************* //

	// ��Ҫ�Ĵ�����
	int result = DEMProcessing(inputRasterFileNames, outputRasterFileNames, tempRasterFileName, shpFileName);
	if(result != 0)
	{
		if(result == 444)
		{
			cout<<"�ļ���ȡ����\n";
		}
		else
		{
			cout<<"������ĵ�"<<result+1<<"��ͼ����֮ǰ��ͼ�����ص����֣���������ȷ��ͼ�����ݡ�\n";	// ����ͼ��֮��û�н����������������֮ǰͼ��û�н�����ͼ�����
		}
		return result;
	}

	cout<<"\n--------DONE!--------\n";
	//system("pause");
	return 0;				// ��������û�д��󣬷���ֵΪ0
}