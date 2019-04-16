//#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <vtkSimplePointsReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkElevationFilter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAutoInit.h> 
#include "vtkDataSetMapper.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include <vtkInteractorStyleTrackballCamera.h>
#include "vtkSmartPointer.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include "iniparser.h"
#include "global_setting.h"


using namespace std;

VTK_MODULE_INIT(vtkRenderingOpenGL2);		//防止出现no override found for""错误
VTK_MODULE_INIT(vtkInteractionStyle);

double Position_x ;
double Position_y;
double Position_z;
double point_x;
double point_y;
double point_z;
double viewup_vx;
double viewup_vy;
double viewup_vz;
double viewangle;
double windowcenter_x;
double windowcenter_y;

void Load3Dfile(const char* filename);
void create_example_ini_file(void);
int parse_ini_file(const char* ini_name);

void Load3Dfile(const char* filename) {
	

	// 新建一个相机对象
	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();

	// 根据相机外矩阵，设置相机（光心）位置
	camera->SetPosition(Position_x,Position_y,Position_z);

	// 根据相机外矩阵，设置焦点位置。设置完这两项成像平面已经唯一确定。
	camera->SetFocalPoint(point_x, point_y, point_z);

	// 根据相机外矩阵，设置成像y正方向。是否有负号取决于原始的相机坐标系中，y是朝向相机上方（正）还是下方（负）。
	camera->SetViewUp(viewup_vx, viewup_vy, viewup_vz);

	// 计算视角。VTK中ViewAngle用角度表示。
	//viewAngle = -2 * atan((sensorSize.height / 2) / focus) * 180 / CV_PI;
	camera->SetViewAngle(viewangle);

	// 计算窗口中心。xh和yh分别是光学中心相对于传感器中心的偏移量。注意正方向规定的差异导致的负号。建议正负号都试一下。
	//windowCenter.x = -xh / (sensorSize.width / 2);
	//windowCenter.y = -yh / (sensorSize.height / 2);
	camera->SetWindowCenter(windowcenter_x, windowcenter_y);	//一般默认(0,0)
	
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	//ifstream ifs(filename.c_str());
	ifstream ifs(filename);
	string singleLine;
	double x, y, z, low_z = DBL_MAX, high_z = DBL_MIN;

	while (getline(ifs, singleLine))
	{
		stringstream linestream;
		linestream << singleLine;
		linestream >> x >> y >> z;
		if (z < low_z)
			low_z = z;
		else if (z > high_z)
			high_z = z;

		points->InsertNextPoint(x, y, z);
	}
	ifs.close();
	
	//vtkPolyData 可以用来保存点、线、面
	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);

	//vtkVertexGlyphFilter 用来加快点云处理速度
	vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	glyphFilter->SetInputData(polydata);
	glyphFilter->Update();
	
	//通过计算每个点到一条线上的投影来完成，计算完成后，每个点都含有一个标量值
	vtkElevationFilter *elevationFilter = vtkElevationFilter::New();	
	elevationFilter->SetInputConnection(glyphFilter->GetOutputPort());
	elevationFilter->SetLowPoint(0, 0, low_z);
	elevationFilter->SetHighPoint(0, 0, high_z);

	vtkPolyDataMapper *dataMapper = vtkPolyDataMapper::New();
	dataMapper->SetInputConnection(glyphFilter->GetOutputPort());

	vtkActor *actor = vtkActor::New();
	actor->SetMapper(dataMapper);


	vtkRenderer *renderer = vtkRenderer::New();
	renderer->AddActor(actor);
	renderer->SetBackground(0, 0, 0);		//设置背景色
	renderer->SetActiveCamera(camera);		//new camera
	
	vtkRenderWindow *renderwind = vtkRenderWindow::New();
	renderwind->SetOffScreenRendering(1);
	renderwind->AddRenderer(renderer);
	renderwind->Render();

	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();

	vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =vtkSmartPointer<vtkWindowToImageFilter>::New();
	windowToImageFilter->SetInput(renderwind);
	windowToImageFilter->Update();
	
	vtkSmartPointer<vtkPNGWriter> writer =vtkSmartPointer<vtkPNGWriter>::New();
	writer->SetFileName("screenshot.png");
	writer->SetInputConnection(windowToImageFilter->GetOutputPort());
	writer->Write();
}
void create_example_ini_file(void) {
	FILE* ini;
	if ((ini = fopen("configure.ini", "w")) == NULL) {
		cout << "create ini false" << endl;
		return;
	}
	//写配置文件
	fprintf(ini,

		"[CAMERA POSITION ]"			"\n"
		"camera_Position_x=0"			"\n"
		"camera_Position_y=0"			"\n"
		"camera_Position_z=1"			"\n\n"

		"[CAMERA FOCALPOINT]"			"\n"
		"camera_FocalPoint_x=0"			"\n"
		"camera_FocalPoint_y=0"			"\n\n"

		"[CAMERA VIEW]"					"\n"
		"camera_ViewUp_vx=0"			"\n"
		"camera_ViewUp_vy=1"			"\n"
		"camera_ViewUp_vz=0"			"\n"
		"camera_ViewAngle=30"			"\n\n"

		"[CAMREA WINDOW]"				"\n"
		"camera_WindowCenter_x=0"		"\n"
		"camera_WindowCenter_y=0"		"\n\n"
	);

	fclose(ini);
	cout << "create ini success" << endl;
}
int parse_ini_file(const char* ini_name) {
	dictionary* ini;

	ini = iniparser_load(ini_name);
	if (ini == NULL) {
		cout << "load config failed"<<endl;
		return -1;
	}
	//iniparser_dump(ini, stderr);
	
	//camera_position
	double camera_position_x = iniparser_getdouble(ini, "CAMERA POSITION:camera_Position_x",0.0);
	set_camera_position_x(camera_position_x);
	Position_x = get_camera_position_x();

	double camera_position_y = iniparser_getdouble(ini, "CAMERA POSITION:camera_Position_y", 0.0);
	set_camera_position_y(camera_position_y);
	Position_y = get_camera_position_y();

	double camera_position_z = iniparser_getdouble(ini, "CAMERA POSITION:camera_Position_z", 1.0);
	set_camera_position_z(camera_position_z);
	Position_z = get_camera_position_z();

	//camera_focuspoint
	double camera_focuspoint_x = iniparser_getdouble(ini, "CAMERA FOCALPOINT:camera_FocalPoint_x", 0.0);
	set_camera_point_x(camera_focuspoint_x);
	point_x =get_camera_point_x();

	double camera_focuspoint_y = iniparser_getdouble(ini, "CAMERA FOCALPOINT:camera_FocalPoint_y", 0.0);
	set_camera_point_y(camera_focuspoint_y);
	point_y = get_camera_point_y();

	double camera_focuspoint_z = iniparser_getdouble(ini, "CAMERA FOCALPOINT:camera_FocalPoint_z", 0.0);
	set_camera_point_z(camera_focuspoint_z);
	point_z = get_camera_point_z();

	//camera_view
	double camera_viewup_vx = iniparser_getdouble(ini, "CAMERA VIEW:camera_ViewUp_vx", 0.0);
	set_camera_viewup_vx(camera_viewup_vx);
	viewup_vx = get_camera_viewup_vx();

	double camera_viewup_vy = iniparser_getdouble(ini, "CAMERA VIEW:camera_ViewUp_vy", 0.0);
	set_camera_viewup_vy(camera_viewup_vy);
	viewup_vy = get_camera_viewup_vy();

	double camera_viewup_vz = iniparser_getdouble(ini, "CAMERA VIEW:camera_ViewUp_vz", 0.0);
	set_camera_viewup_vz(camera_viewup_vz);
	viewup_vz = get_camera_viewup_vz();

	double camera_viewangle = iniparser_getdouble(ini, "CAMERA VIEW:camera_ViewAngle", 30);
	set_camera_viewangle(camera_viewangle);
	viewangle = get_camera_viewangle();

	double camera_windowcenter_x = iniparser_getdouble(ini, "CAMREA WINDOW:camera_WindowCenter_x", 0.0);
	set_camera_WindowCenter_x(camera_windowcenter_x);
	windowcenter_x = get_camera_WindowCenter_x();

	double camera_windowcenter_y = iniparser_getdouble(ini, "CAMREA WINDOW:camera_WindowCenter_y", 0.0);
	set_camera_WindowCenter_y(camera_windowcenter_y);
	windowcenter_y = get_camera_WindowCenter_y();
	return 0;
}
int main(){
	create_example_ini_file();
	int ret=parse_ini_file("configure.ini");
	Load3Dfile("screw_01.txt");
	return ret;
}