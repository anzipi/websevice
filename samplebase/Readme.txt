ע�����

1. ��������õ��Ʋ���������ֵ�Ĺ�ʽΪ��propertyVals[rowIdx][colIdx] = SumSimilarityValue/SumSimilarity;
Ҳ���Բ�����һ����ʽpropertyVals[rowIdx][colIdx] = MaxValue * MaxSimilarity + (1.0 - MaxSimilarity) * (SumSimilarityValue - MaxValue * MaxSimilarity) / (SumSimilarity - MaxSimilarity);
2. ʹ�ñ�����ʱ���뱣֤���л������ӵ�nodata��ֵ����һ�µģ�-9999��ã�
3. �������������Ҫ����openmpi���м��㻷����gdal������Linux��
	���뱾��������mpic++ *.cpp -o samplebase -lgdal ����samplebase��������
	���б���������mpirun -n ������ ����·�� ���� 
 
���磺

**version 1.1 2015-09-03 **

mpirun -n 4 samplebase "/home/admin/work/sampleWebservice/data2/geo.tif#/home/admin/work/sampleWebservice/data2/jiangshui.tif#/home/admin/work/sampleWebservice/data2/slope.tif#/home/admin/work/sampleWebservice/data2/dem.tif" "/home/admin/work/sampleWebservice/data2/training.csv" "Geology?Boolean#Climate?Gaussian#Terrain?Gaussian#Terrain?Gaussian" 0.5 "/home/admin/work/sampleWebservice/result/property" "/home/admin/work/sampleWebservice/result/uncertainty"

��ע��Gaussian�ɻ�ΪGower

**version 1.2 2016-01-25**


mpirun -n 4 samplebase "/home/admin/work/test/data/jiangshui.tif#/home/admin/work/test/data/geo.tif#/home/admin/work/test/data/dem.tif#/home/admin/work/test/data/slope.tif" "/home/admin/work/test/data/training.csv" "Climate?Gower#Geology?Boolean#Terrain?Gower#Terrain?Gower" 0.5 "/home/admin/work/test/result/propertyClay" "/home/admin/work/test/result/uncertaintyClay" "X" "Y" "Clay" "Limit" "Limit"

��ע��X��Y��Clay�ֱ��������ļ�������x,y�ʹ��Ʋ����Ե��������ۺϷ������Ի�ΪAverage

