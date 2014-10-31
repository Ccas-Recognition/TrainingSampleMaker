#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <time.h>

#include "FilesUtils.h"
#include "ImageNavigator.h"


using namespace std;
using namespace cv;

void DrawHelp();
void GenerateHelpText();
void GenerateNegativeSamples();
void CallBackMouseFunc(int event, int x, int y, int flags, void* userdata);
int CheckIntersections( const vector< Rect_<int> > &rects, const Rect_<int> &targetRect );
int GetIntersectionRectSquare( const Rect_<int> &rect1, const Rect_<int> &rect2 );
void SaveResult( string filename, string fg_dir, string bg_dir );
void LoadCache(string filename, string bg_dir, string fg_dir);

Mat src;
char window_name[] = "Training Sample Maker";

int w, h;
const int MOUSE_OFFSET_SIZE = 40;
const int MOUSE_OFFSET_SPEED = 10;

int cursor_pos_x;
int cursor_pos_y;

int current_x;
int current_y;
bool isDrag;

int dx, dy;

int rect_sizes[] = {24, 36, 48, 60};
int current_size;

auto_ptr<ImageTools::Navigator> navi;

Scalar deleteColor, addColor, positiveColor, negativeColor;
Scalar currentColor;
vector< Rect_<int> > positive;
vector< Rect_<int> > negative;
vector< Rect_<int> > currentRectTarget;

bool showBackgroundSamples;

int imageWidth, imageHeight;
bool removeAllFilesInDir;

vector<string> savedFiles;
vector<string> helpText;

void PrintUsage()
{
	std::cout <<
		"main image_name.jpg -fg imagelist_foreground_dir -bg imagelist_background_dir -rm" << endl << "-rm removes all files in dirs";
}


 int main( int argc, char** argv )
 {
	if( argc < 2 )
	{
		PrintUsage();
		return -1;
	}

	showBackgroundSamples = true;
	removeAllFilesInDir = false;
	string filename, fg_dir, bg_dir;
	filename = argv[1];

	for(int i=1; i<argc; ++i)
	{
		if( string( argv[i] ) == "-fg" )
		{
			++i;
			fg_dir = FilesUtils::fixDir( argv[i] );
		}
		else if( string( argv[i] ) == "-bg" )
		{
			++i;
			bg_dir = FilesUtils::fixDir( argv[i] );
		}else if( string( argv[i] ) == "-rm" )
		{
			removeAllFilesInDir = true;
		}
	}	

	src = imread( filename, 1 );
	if(src.data == 0)
	{
		cout << "Invalid Path" << endl;
		return -1;
	}

	GenerateHelpText();
	LoadCache(filename, bg_dir, fg_dir);

	imageWidth = src.cols;
	imageHeight = src.rows;

	dx = 0;
	dy = 0;

	current_size = 2;

	isDrag = false;
	cursor_pos_x = 0;
	cursor_pos_y = 0;

	w = 1024;
	h = 800;

	deleteColor = Scalar(0.0, 0.0, 255.0);
	addColor = Scalar(0.0, 255.0, 255.0);
	positiveColor = Scalar(0.0, 255.0, 0.0);
	negativeColor = Scalar(255.0, 0.0, 0.0);
	currentColor = addColor;

	navi = std::auto_ptr<ImageTools::Navigator> ( new ImageTools::Navigator(w, h, src) );
	navi->SetRectangles( positive, positiveColor );
	navi->SetRectangles( negative, negativeColor );
	navi->SetRectangles( currentRectTarget, currentColor);

	currentRectTarget.push_back( Rect_<int>(0, 0, rect_sizes[current_size], rect_sizes[current_size] ) );

#if 0
	positive.push_back( Rect_<int>(0, 0, 100, 100) );
	positive.push_back( Rect_<int>(210, 410, 100, 100) );
	positive.push_back( Rect_<int>(250, 120, 100, 100) );
	positive.push_back( Rect_<int>(1500, 1300, 100, 100) );
	negative.push_back( Rect_<int>(0, 0, 4080, 3072) );
	negative.push_back( Rect_<int>(1650, 2310, 100, 100) );
	negative.push_back( Rect_<int>(3350, 420, 100, 100) );
	negative.push_back( Rect_<int>(1100, 700, 100, 100) );
#endif

	navi->Update();
	namedWindow(window_name, 1);

	setMouseCallback(window_name, CallBackMouseFunc, 0);
	
	imshow(window_name, navi->Get());

	bool drawHelp = false;
	char c;	
	do
	{
		c = waitKey(10);
#if 0
		if(c != -1)
			cout << int(c) << endl;
#endif
		if( dx != 0 || dy != 0)
		{
			navi->Translate(dx, dy);
			//imshow(window_name, navi->Get());	
		}

		if( c == 6 ) // ctrl+f
		{
			GenerateNegativeSamples();
		}
		else if( c == 19 || c == 13 ) // ctrl+s
		{
			SaveResult( FilesUtils::getFileName( FilesUtils::remExt( filename ) ), fg_dir, bg_dir );
		}
		else if( c == 61 ||  c == 101) // =, e
		{
			navi->Scale(1.2f, 1.2f );
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());
		}
		else if( c == 45 ||  c == 113  ) //-, q
		{
			navi->Scale(1.0f/1.2f, 1.0f/1.2f );
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());
		}

		else if( c == 119) //w
		{
			navi->Translate(0, -MOUSE_OFFSET_SPEED*3/navi->GetScale() );
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());	
		}
		else if( c == 97) //a
		{
			navi->Translate(-MOUSE_OFFSET_SPEED*3/navi->GetScale(), 0 );
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());	
		}
		else if( c == 115) //s
		{
			navi->Translate(0, MOUSE_OFFSET_SPEED*3/navi->GetScale());
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());	
		}
		else if( c == 100) // d
		{
			navi->Translate(MOUSE_OFFSET_SPEED*3/navi->GetScale(), 0);
			CallBackMouseFunc( EVENT_MOUSEMOVE, current_x, current_y, 0, 0 );
			//imshow(window_name, navi->Get());	
		}

		else if( c >= 49 && c < (49 + sizeof(rect_sizes)/sizeof(int)) ) //1, 2, 3, 4 
		{
			current_size = c - 49;
			//cout << current_size <<endl;
			int sz = rect_sizes[current_size];
			currentRectTarget[0].width = sz;
			currentRectTarget[0].height = sz;
			currentRectTarget[0].x = current_x;
			currentRectTarget[0].y = current_y;
			navi->LocalToGlobal(currentRectTarget[0]);
			navi->DrawVectorGraphics();
			//imshow(window_name, navi->Get());
		}
		else if( c == 26 ) //ctrl+z
		{
			positive.pop_back();
			navi->DrawVectorGraphics();
			//imshow(window_name, navi->Get());
		}
		else if( c == 11 ) //ctrl+k
		{
			positive.clear();
			negative.clear();
			navi->DrawVectorGraphics();
			//imshow(window_name, navi->Get());
		}
		else if(c == 104) //h
		{
			drawHelp = !drawHelp;
			navi->Update();
			//imshow(window_name, navi->Get());
		}
		else if(c == 48) //0
		{
			showBackgroundSamples = !showBackgroundSamples;
			navi->SetShowBackgroundRectangles(showBackgroundSamples  );
		}
		if(drawHelp)
		{
			DrawHelp();
		}
		imshow(window_name, navi->Get());
	} while( c != 13 && c != 27 );

	return 0;
 }


void CallBackMouseFunc(int event, int x, int y, int flags, void* userdata)
{
	//cout << event << endl;
	if  ( event == EVENT_RBUTTONDOWN )
	{
		isDrag = true;
		cursor_pos_x = x;
		cursor_pos_y = y;
		navi->StartDrag(cursor_pos_x, cursor_pos_y);
		dx = 0;
		dy = 0;
		//cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	else if  ( event == EVENT_RBUTTONUP )
	{
		isDrag = false;
		//cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	else if  ( event == EVENT_LBUTTONDOWN )
	{
		//cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		int index = CheckIntersections(positive, currentRectTarget[0]);
		if( index == -1 )
		{
			positive.push_back( currentRectTarget[0] );
		}
		else
		{
			for(unsigned int i=index; i<positive.size()-1; ++i)
				positive[i] = positive[i+1];
			positive.pop_back();
		}
		navi->DrawVectorGraphics();
		//imshow(window_name, navi->Get());
	}
	else if  ( event == EVENT_LBUTTONDOWN )
	{
		//cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	}
	else if ( event == EVENT_MOUSEMOVE )
	{
		current_x = x;
		current_y = y;

		if(isDrag)
		{
			navi->Drag(x, y);
			//imshow(window_name, navi->Get());
		}
	else
	{
		if( x < MOUSE_OFFSET_SIZE )
			dx = -MOUSE_OFFSET_SPEED/navi->GetScale();
		else if( x > w - MOUSE_OFFSET_SIZE )
			dx =  MOUSE_OFFSET_SPEED/navi->GetScale();
		else
			dx = 0;

		if( y < MOUSE_OFFSET_SIZE )
		dy = -MOUSE_OFFSET_SPEED/navi->GetScale();
		else if( y > h - MOUSE_OFFSET_SIZE )
			dy = MOUSE_OFFSET_SPEED/navi->GetScale();
		else
			dy = 0;
	}

	int sz = rect_sizes[current_size];
	currentRectTarget[0].width = sz;
	currentRectTarget[0].height = sz;
	currentRectTarget[0].x = current_x;
	currentRectTarget[0].y = current_y;
	navi->LocalToGlobal( currentRectTarget[0] );
	if( CheckIntersections(positive, currentRectTarget[0]) != -1 )
		currentColor = deleteColor;
	else
		currentColor = addColor;
	navi->DrawVectorGraphics();
	//imshow(window_name, navi->Get());	
	}
}


int GetIntersectionRectSquare( const Rect_<int> &rect1, const Rect_<int> &rect2 )
{
	int minX = max( rect1.x, rect2.x );
	int minY = max( rect1.y, rect2.y );

	int maxX = min( rect1.x + rect1.width, rect2.x + rect2.width );
	int maxY = min( rect1.y + rect1.height, rect2.y + rect2.height );

	int width = max( maxX - minX, 0 );
	int height = max( maxY - minY, 0 );

	return width*height;
}

int CheckIntersections( const vector< Rect_<int> > &rects, const Rect_<int> &targetRect )
{
	float square = float( targetRect.width*targetRect.height );
	int index = 0;
	for(auto it = rects.begin(); it != rects.end(); ++it, ++index )
	{
		float currentSquare = min(square, float( it->height*it->width ) );
		if( GetIntersectionRectSquare( *it, targetRect )/square > 0.35f )
			return index;
	}
	return -1;
}

void GenerateNegativeSamples()
{
	unsigned int ITERATIONS = positive.size()*2;
	//const int ITERATIONS = 4000;
	unsigned int sizes_totall = sizeof(rect_sizes)/sizeof(int);

	negative.clear();
	for(unsigned int i=0; i<ITERATIONS; ++i)
	{
		int randSize = rect_sizes[ i%sizes_totall ];
		int max_x = imageWidth - randSize;
		int max_y = imageHeight - randSize;
		int x = int(rand()/float(RAND_MAX)*max_x);
		int y = int(rand()/float(RAND_MAX)*max_y);
		
		Rect_<int> rect(x, y, randSize, randSize);
		if( CheckIntersections(positive, rect) == -1)
			negative.push_back(rect);
	}

	for(unsigned int i=0; i<positive.size(); ++i)
	{
		for(int dx=-1; dx <= 1; ++dx)
		{
			for(int dy=-1; dy <= 1; ++dy)
			{
				if(dx == 0 && dy == 0)
					continue;
				Rect_<int> rect = positive[i];
				rect.x -= dx*rect.width;
				rect.y -= dy*rect.height;
				if( CheckIntersections(positive, rect) == -1)
					negative.push_back(rect);
			}
		}
	}
}

void SaveRectangles( string xml_filename, const vector< Rect_<int> > &rects )
{
	string gen_filename = FilesUtils::remExt(xml_filename);

	FileStorage storage( xml_filename, FileStorage::WRITE );
	savedFiles.push_back(xml_filename);

	storage << "images" << "[";
	for(unsigned int i=0; i<rects.size(); ++i)
	{
		const Rect_<int> &rect = rects[i];
		if( rect.x < 0 || rect.y < 0 || (rect.x + rect.width) >= src.cols || (rect.y + rect.height) >= src.rows )
			continue;

		stringstream filename;
		
		filename << gen_filename << "_" << rect.x << "_" << rect.y << "_" << rect.width << "_" << rect.height << ".jpg";
		Mat ImagePart = src( Range( rect.y, rect.y + rect.height ), Range( rect.x, rect.x + rect.width) );
		imwrite(filename.str(), ImagePart);
		savedFiles.push_back( filename.str() );

		string part_name = FilesUtils::getFileNameWithExt(filename.str());
		storage << ("\"" + part_name + "\"");
	}
	storage << "]";
	storage.release();
}

void RemoveFiles(string dirname)
{
	dirname.pop_back();
#ifdef WIN32
	string command = (string("del /Q ") + FilesUtils::fixWindowsDirectory(dirname) );
	system( command.c_str() );
#else
	string command = (string("rm -r ") + dirname);
	system( command.c_str() );
	command = (string("mkdir ") + dirname);
	system( command.c_str() );
#endif
}

void RemoveFile(string filename)
{
#ifdef WIN32
	string command = (string("del /Q ") + FilesUtils::fixWindowsDirectory(filename));
	system( command.c_str() );
#else
	string command = (string("rm ") + filename);
	system( command.c_str() );
#endif
}


void SaveResult( string filename, string fg_dir, string bg_dir )
{
	GenerateNegativeSamples();
	if( removeAllFilesInDir )
	{
		RemoveFiles(fg_dir);
		RemoveFiles(bg_dir);
	}
	if( savedFiles.size() != 0 )
	{
		for(unsigned int i=0; i<savedFiles.size(); ++i)
			RemoveFile( savedFiles[i] );
		savedFiles.clear();
		cout << "revert previous save" << endl;
	}
	if(negative.size() == 0)
		GenerateNegativeSamples();

	srand( unsigned int( time(0) ) );
	string rectanglesId = "rnd" + FilesUtils::int2str(rand()%1000);

	SaveRectangles( fg_dir + filename + "_" + rectanglesId + "_fg.xml", positive );
	SaveRectangles( bg_dir + filename + "_" + rectanglesId + "_bg.xml", negative );

	cout << "saved: " << ( fg_dir + filename ) << ", " << ( bg_dir + filename ) << endl;
}

void LoadRects(string filename, string dir, vector< Rect_<int> > &rects, const list<string> &files )
{
	for( auto it = files.begin(); it != files.end(); ++it )
	{
		stringstream partname( FilesUtils::remExt(*it) );
		string item;
		array<string, 7> tokens;
		for(int i=0; i<7; ++i)
		{
			getline(partname, item, '_');
			tokens[i] = item;
		}
		if( tokens[0] == filename )
		{
			savedFiles.push_back( dir + (*it) );

			Rect_<int> part;
			part.x = FilesUtils::str2int(tokens[3]);
			part.y = FilesUtils::str2int(tokens[4]);
			part.width = FilesUtils::str2int(tokens[5]);
			part.height = FilesUtils::str2int(tokens[6]);

			rects.push_back(part);
		}
	}
}

void LoadCache(string filename, string bg_dir, string fg_dir)
{
	filename = FilesUtils::getFileName(filename);
	string xml_mask_fg = filename + "*_fg.xml";
	string xml_mask_bg = filename + "*_bg.xml";
	list<string> fg_files, bg_files, bg_xml, fg_xml;

#if WIN32
	fg_files = FilesUtils::FilesInDir(fg_dir.c_str(), "*.jpg");
	bg_files = FilesUtils::FilesInDir(bg_dir.c_str(), "*.jpg");
	fg_xml = FilesUtils::FilesInDir(fg_dir.c_str(), xml_mask_fg.c_str());
	bg_xml = FilesUtils::FilesInDir(bg_dir.c_str(), xml_mask_bg.c_str());
#endif
	LoadRects(filename, fg_dir, positive, fg_files);
	LoadRects(filename, bg_dir, negative, bg_files);

	for(auto it = fg_xml.begin(); it != fg_xml.end(); ++it)
		savedFiles.push_back(fg_dir + (*it) );
	for(auto it = bg_xml.begin(); it != bg_xml.end(); ++it)
		savedFiles.push_back(bg_dir + (*it) );
}		

void GenerateHelpText()
{
	helpText.push_back( "h - show/hide help" );
	helpText.push_back( "w, a, s, d -- up, left, down, right navigation" );
	helpText.push_back( "e or = -- zoom in" );
	helpText.push_back( "q or - -- zoom out" );
	helpText.push_back( "ctrl+s -- save and generate training samples" );
	helpText.push_back( "ctrl+f -- generate background negative samples" );
	helpText.push_back( "ctrl+z -- remove last part" );
	helpText.push_back( "ctrl+k -- remove all parts" );
	helpText.push_back( "0 -- show/hide background samples" );
	helpText.push_back( "1, 2, 3, 4 -- change sample part size" );
	helpText.push_back( "enter -- exit with saving" );
	helpText.push_back( "exit -- exit without saving" );
	helpText.push_back( "right mouse -- image navigation" );
	helpText.push_back( "left mouse -- put new positive sample part" );
	helpText.push_back( "      or remove existing on mouse position" );
}
void DrawHelp()
{
	const int line_height = 20;
	const int helpdesk_x = 600;

	Rect_<int> helpdesk(helpdesk_x, 10, src.cols - helpdesk_x - 20,  10 + line_height*helpText.size() );

	rectangle(navi->Get(), helpdesk, Scalar(0, 0, 0), CV_FILLED );
	
	for(unsigned int i=0; i<helpText.size(); ++i)
	{
		putText(navi->Get(), helpText[i], Point(helpdesk.x + 10, helpdesk.y+i*line_height + line_height), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255.0, 255.0, 255.0) );

	}
}