#include <vtkSmartPointer.h>
// For the rendering pipeline setup:
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
// For vtkBoxWidget:
#include <vtkBoxWidget.h>
#include <vtkCommand.h>
#include <vtkTransform.h>

#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPlane.h>

#include <vtkImageData.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <stdio.h>

//For dictionary
#include "iniparser.h"
#include "global_setting.h"

vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkPolyDataMapper> mapper ;
float rad=0.5f;
int   nimage=1;

//new camera add
double Position_x;
double Position_y;
double Position_z;
double point_x;
double point_y;
double point_z;

class vtkMyCallback : public vtkCommand
{
public:
	static vtkMyCallback *New()
	{
		return new vtkMyCallback;
	}
	virtual void Execute( vtkObject *caller, unsigned long, void* )
	{
    // Here we use the vtkBoxWidget to transform the underlying coneActor
    // (by manipulating its transformation matrix).
		vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
		vtkBoxWidget *widget = reinterpret_cast<vtkBoxWidget*>(caller);
		widget->GetTransform( t );
		widget->GetProp3D()->SetUserTransform( t );
		vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
		widget->GetPolyData(pd);
		for(int i=0;i<8;i++)
		{
			printf("point %d: %.4f,%.4f,%.4f;\n",i,pd->GetPoint(i)[0],pd->GetPoint(i)[1],pd->GetPoint(i)[2]);
		}
	}
};

class vtkTimerCallback : public vtkCommand
{
  public:
    static vtkTimerCallback *New()
    {
      vtkTimerCallback *cb = new vtkTimerCallback;
      cb->TimerCount = 0;
      return cb;
    }

    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,void *vtkNotUsed(callData))
    {
		if (vtkCommand::TimerEvent == eventId)
		{
			++this->TimerCount;
			//cout << this->TimerCount << endl;			
			
			
			vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
			sphereSource->SetCenter(0.0, 0.0, 0.0);
			sphereSource->SetRadius(rad);
			sphereSource->Update();
			mapper->SetInputConnection(sphereSource->GetOutputPort());
			
			vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
			windowToImageFilter->SetInput(renderWindow);
			windowToImageFilter->Update();
			windowToImageFilter->SetInputBufferTypeToRGB();
			//windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel

			#if VTK_MAJOR_VERSION > 8 || VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION >= 1
				windowToImageFilter->SetScale(2); //image quality
			#else
				windowToImageFilter->SetMagnification(1); //image quality
			#endif
			
			windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer

			vtkImageData *output=windowToImageFilter->GetOutput(); 

			int width = output->GetDimensions()[0];
			int height = output->GetDimensions()[1];
			unsigned char *colorsPtr = reinterpret_cast<unsigned char *>( output->GetScalarPointer() );

			printf("w:%d;h=%d\n",width,height); 
			cv::Mat testImg(height,width,CV_8UC3);
			uchar * data000 = testImg.ptr<uchar>(0);
			memcpy( data000,colorsPtr,width*height*3);
			cv::cvtColor(testImg, testImg, cv::COLOR_RGB2BGR);
			char buf[128];
			sprintf( buf, "test%.4d.jpg",nimage);
			cv::imwrite(buf,testImg);		
			nimage++;
			rad+=0.1f;
			if( rad > 20 ) rad=0.1f;
			
		}
        
    }
  private:
    int TimerCount;
    
};


//bool GetOutput(vtkRenderWindow *renWin，unsigned char* pdata, int maxlen)
bool GetOutput(vtkRenderWindow* renWin,unsigned char* pdata, int maxlen)
{
	
	
    int w = renWin->GetSize()[0];
    int h = renWin->GetSize()[1];

    if (maxlen<w*h * 4)
            return false;
	
    vtkSmartPointer<vtkUnsignedCharArray> pixels = vtkSmartPointer<vtkUnsignedCharArray>::New(); 
    pixels->SetArray(pdata, w*h * 4, 1); 

    renWin->GetRGBACharPixelData(0, 0, h - 1, w - 1, 1, pixels); 

    return true;
}


void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), 
								void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
  //std::cout << "Keypress callback" << std::endl;
  
  vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
  std::cout << "Pressed: " << iren->GetKeySym() << std::endl;
  
  
  if( strcmp( iren->GetKeySym(), "d") == 0 )		//按d保存图片
  {
		//std::cout << "DPressed: " <<std::endl;
		vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
		windowToImageFilter->SetInput(renderWindow);
		windowToImageFilter->Update();
		windowToImageFilter->SetInputBufferTypeToRGB();
		//windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel

		#if VTK_MAJOR_VERSION > 8 || VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION >= 1
			windowToImageFilter->SetScale(2); //image quality
		#else
			windowToImageFilter->SetMagnification(1); //image quality
		#endif
		
		windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer

		vtkImageData *output=windowToImageFilter->GetOutput(); 

		int width = output->GetDimensions()[0];
		int height = output->GetDimensions()[1];
		unsigned char *colorsPtr = reinterpret_cast<unsigned char *>( output->GetScalarPointer() );

		printf("w:%d;h=%d\n",width,height); 
		cv::Mat testImg(height,width,CV_8UC3);
		uchar * data000 = testImg.ptr<uchar>(0);
		memcpy( data000,colorsPtr,width*height*3);
		cv::cvtColor(testImg, testImg, cv::COLOR_RGB2BGR);
		char buf[128];
		sprintf( buf, "test%.4d.jpg",nimage);
		cv::imwrite(buf,testImg);		
		nimage++;
  }
  else if( strcmp( iren->GetKeySym(), "x") == 0 )	//按x重新加载数据
  {
		std::cout << "xPressed: " <<std::endl;

		// rad+=0.5f;
		// vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
		// sphereSource->SetCenter(0.0, 0.0, 0.0);
		// sphereSource->SetRadius(rad);
		// sphereSource->Update();

		// mapper->SetInputConnection(sphereSource->GetOutputPort());
  }
}

void loaddata()
{
	/*vtkPoints * points = vtkPoints::New();
    int n=0;
    while(!feof(fp))//首先读取点云数据到点表points同时指定点对应的id:
    {
        int ret=fscanf(fp,"%lf %lf %lf",&arr[0],&arr[1],&arr[2]);
        if(ret!=3)
            break;     
        points->InsertPoint(n,arr[0],arr[1],arr[2]);
        n++;
    }
    printf("%d\n", n);
    fclose(fp);

    vtkPolyVertex * polyvertex = vtkPolyVertex::New();
    polyvertex->GetPointIds()->SetNumberOfIds(n);
    int i=0;
    for(i=0;i<n;i++)//建立拓扑关系
    {
        polyvertex->GetPointIds()->SetId(i,i);
    }

    vtkUnstructuredGrid * grid=vtkUnstructuredGrid::New();
    grid->SetPoints(points);
    grid->InsertNextCell(polyvertex->GetCellType(),
            polyvertex->GetPointIds());

    vtkDataSetMapper *map1 = vtkDataSetMapper::New();
    map1->SetInput(grid);*/
	
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
		"camera_FocalPoint_y=0"			"\n"
		"camera_FocalPoint_z=0"			"\n\n"
	);

	fclose(ini);
	cout << "create ini success" << endl;
}

int parse_ini_file(const char* ini_name) {
	dictionary* ini;

	ini = iniparser_load(ini_name);
	if (ini == NULL) {
		cout << "load config failed" << endl;
		return -1;
	}
	//iniparser_dump(ini, stderr);
	
	//camera_position
	double camera_position_x = iniparser_getdouble(ini, "CAMERA POSITION:camera_Position_x", 0.0);
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
	point_x = get_camera_point_x();

	double camera_focuspoint_y = iniparser_getdouble(ini, "CAMERA FOCALPOINT:camera_FocalPoint_y", 0.0);
	set_camera_point_y(camera_focuspoint_y);
	point_y = get_camera_point_y();

	double camera_focuspoint_z = iniparser_getdouble(ini, "CAMERA FOCALPOINT:camera_FocalPoint_z", 0.0);
	set_camera_point_z(camera_focuspoint_z);
	point_z = get_camera_point_z();

	return 0;
}
int main( int vtkNotUsed( argc ), char* vtkNotUsed( argv )[] )
{
	

	// Create a sphere
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(0.0, 0.0, 0.0);
	sphereSource->SetRadius(1.0);
	sphereSource->Update();

	// Create a mapper and actor
	mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	//camera
	vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
	camera->SetPosition(Position_x, Position_y, Position_z);
	camera->SetFocalPoint(0, 0, 0);

	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();  
	renderer->SetActiveCamera(camera);
	  
	renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize( 300, 300 );
	//renderWindow->SetOffScreenRendering(1);

	// Add the actor to the scene
	renderer->AddActor(actor);
	renderer->SetBackground(0.0,0.0,0.4); // Background color white
	renderer->ResetCameraClippingRange(); //不加这一行可能会成像不完整  

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	
	// vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	// keypressCallback->SetCallback ( KeypressCallbackFunction );
	// renderWindowInteractor->AddObserver ( vtkCommand::KeyPressEvent, keypressCallback ); 

	renderWindowInteractor->Initialize();
	
	vtkSmartPointer<vtkTimerCallback> cb = vtkSmartPointer<vtkTimerCallback>::New();
	renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, cb);
  
	int timerId = renderWindowInteractor->CreateRepeatingTimer(20);
  
	// Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}

