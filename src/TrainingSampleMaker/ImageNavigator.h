#ifndef IMAGE_NAVIGATOR_H
#define IMAGE_NAVIGATOR_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

namespace ImageTools
{
	using namespace std;
	using namespace cv;

	template<typename T>
	T clamp(T val, T begin, T end)
	{
		if(val > end)
			return end;
		else if(val < begin)
			return begin;
		return val;
	}

	class Navigator
	{
		int w, h;
		int fullResolutionWidth, fullResolutionHeight;
		Mat source, image, resizedImage, buffer1, buffer2;
		int dragX, dragY;
		int BORDER_SIZE;

		//Point2i leftTopPosition;
		Point2i dragCenterPosition;
		Point2i centerPosition;
		Point2f scale;

		vector< vector< Rect_<int> >* > drawRectangles;
		vector< Scalar* > colors;

	public:
		Navigator(int _w, int _h, Mat _source )
		{
			BORDER_SIZE = _w/2;

			//leftTopPosition.x = BORDER_SIZE;
			//leftTopPosition.y = BORDER_SIZE;
			
			scale.x = 0.25f;
			scale.y = 0.25f;

			w = _w;
			h = _h;
			//source = Mat( source.rows + BORDER_SIZE*2, source.cols + BORDER_SIZE*2, source.type(), 0 );
			//_source.copyTo( source( Range( BORDER_SIZE, BORDER_SIZE + _source.rows - 1 ), Range( BORDER_SIZE, BORDER_SIZE + _source.cols - 1 ) ) );
			copyMakeBorder( _source, source, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, BORDER_CONSTANT, 0 );

			centerPosition.x = source.cols/2;
			centerPosition.y = source.rows/2;

			fullResolutionHeight = source.rows;
			fullResolutionWidth = source.cols;

			image = Mat( h, w, source.type() );
			resizedImage = Mat( h, w, source.type() );
			Update();
		}

		void SetRectangles( vector< Rect_<int> > &_rectangles, Scalar &color )
		{
			drawRectangles.push_back(&_rectangles);
			colors.push_back(&color);
		}

		Mat& Get()
		{
			return image;
		}

		void StartDrag(int _dragX, int _dragY )
		{
			dragX = _dragX;
			dragY = _dragY;
			dragCenterPosition = centerPosition;
		}

		void Drag(int x, int y)
		{
			centerPosition.x = dragCenterPosition.x + int((dragX-x)/scale.x);
			centerPosition.y = dragCenterPosition.y + int((dragY-y)/scale.y);
			Update();
		}

		void Translate(int dx, int dy)
		{
			centerPosition.x += dx;
			centerPosition.y += dy;

			Update();
		}
		void Scale(float dx, float dy)
		{
			//float center_x = leftTopPosition.x + w/2*scale.x;
			//float center_y = leftTopPosition.y + h/2*scale.y;

			scale.x = clamp(scale.x*dx, 0.25f, 10.0f);
			scale.y = clamp(scale.y*dy, 0.25f, 10.0f);

			//float current_center_x = leftTopPosition.x + w/2*scale.x;
			//float current_center_y = leftTopPosition.y + h/2*scale.y;

			//leftTopPosition.x += current_center_x - center_x;
			//leftTopPosition.y += current_center_y - center_y;

			Update();
		}
		void Update()
		{
			centerPosition.x = clamp( centerPosition.x, int(w/2*1.0f/scale.x), fullResolutionWidth - int(w/2*1.0f/scale.x) );
			centerPosition.y = clamp( centerPosition.y, int(h/2*1.0f/scale.y), fullResolutionHeight - int(h/2*1.0f/scale.y) );

			source( Range(centerPosition.y - int(h/2*1.0f/scale.y), centerPosition.y + int(h/2*1.0f/scale.y)), 
				Range(centerPosition.x - int(w/2*1.0f/scale.y), centerPosition.x + int(w/2*1.0f/scale.y) ) ).copyTo(buffer1);
			resize( buffer1, resizedImage, resizedImage.size(), scale.x, scale.y, INTER_NEAREST );

			DrawVectorGraphics();
		}
		void LocalToGlobal( Rect_<int> &rect )
		{
			rect.x = int( ( rect.x )/scale.x );
			rect.y = int( ( rect.y )/scale.y );
			rect.x += ( centerPosition.x - int(w/2*1.0f/scale.x) ) - BORDER_SIZE;
			rect.y += ( centerPosition.y - int(h/2*1.0f/scale.y) ) - BORDER_SIZE;
			rect.x -= rect.width/2;
			rect.y -= rect.height/2;
		}
		void DrawVectorGraphics()
		{
			if(drawRectangles.size() == 0)
				return;
			//buffer2 = Mat(image.rows, image.cols, image.type(), Scalar(0, 0, 0));
			buffer2 = resizedImage.clone();
			
			DrawRectangles(1, buffer2);//crap
			addWeighted(resizedImage, 0.8, buffer2, 0.2, 0.0, image); //crap
			DrawRectangles(0, image);//crap
			DrawRectangles(2, image);//crap
		}
		private:
		void DrawRectangles(int index, Mat &buffer)
		{
			int thickness =  min( max( int(scale.x), 1), 3);
			vector< Rect_<int> > &rects = *(drawRectangles[index]);
			for(unsigned int j=0; j<rects.size(); ++j )
			{
				Rect_<int> rect = rects[j];
				rect.x += BORDER_SIZE - ( centerPosition.x - int(w/2*1.0f/scale.x) );
				rect.y += BORDER_SIZE - ( centerPosition.y - int(h/2*1.0f/scale.y) );
				rect.x = int( rect.x*scale.x );
				rect.y = int( rect.y*scale.y );
				rect.width = int(rect.width*scale.x);
				rect.height = int(rect.height*scale.y);
				rectangle( buffer, rect, *(colors[index]), thickness);
			}
		}
	};
}

#endif
