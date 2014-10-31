#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <list>
#include <string>
#include <sstream>
#ifdef WIN32
	#include <windows.h>
	#include <io.h>
#endif


namespace FilesUtils
{
	std::string getExt(std::string path)
	{
		std::string ext;
		std::string::size_type i=path.find_last_of("./\\");
		if(i<path.length())
			if(path[i]=='.')
				ext.assign(path,i,path.size());
		return ext;
	}

	std::string remExt(std::string path)
	{
		std::string ext;
		std::string::size_type i=path.find_last_of("./\\");
		if(i<path.length())
		{
			if(path[i]=='.')
			{
				ext.assign(path,0,i);
				return ext;
			}
			else
				return path;
		}
		return path;
	}
	std::string getFileName(std::string path)
	{
		path = remExt(path);
		std::string filename;
		std::string::size_type i=path.find_last_of("/\\");
		if(i<path.length())
		{
			if(path[i]=='/' || path[i]=='\\')
			{
				filename.assign(path,i+1,path.size());
				return filename;
			}
			else
				return path;
		}
		return path;
	}
	std::string getFileNameWithExt(std::string path)
	{
		std::string filename;
		std::string::size_type i=path.find_last_of("/\\");
		if(i<path.length())
		{
			if(path[i]=='/' || path[i]=='\\')
			{
				filename.assign(path,i+1,path.size());
				return filename;
			}
			else
				return path;
		}
		return path;
	}
	std::string getDirectoryName(std::string path)
	{
		std::string filename;
		std::string::size_type i=path.find_last_of("/\\");
		if(i<path.length())
		{
			if(path[i]=='/' || path[i]=='\\')
			{
				filename.assign(path,0,i+1);
				return filename;
			}
			else
				return path;
		}
		return path;
	}
	std::string fixDir(std::string dir)
	{
		if(dir.length() == 0)
			return dir;

		if(dir.back() != '/' && dir.back() != '\\')
		{
			dir += '/';
		}
		return dir;
	}

	std::string fixWindowsDirectory(std::string filename)
	{
		for(unsigned int i=0; i<filename.length(); ++i)
		{
			if( filename[i] == '/' )
				filename[i] = '\\';
		}
		return filename;
	}

	int str2int(std::string str)
	{
		std::stringstream ss;
		ss << str;
		int res = 0;
		ss >> res;
		return res;
	}
	std::string int2str(int val)
	{
		std::stringstream ss;
		ss << val;
		return ss.str();
	}

#ifdef WIN32
	std::list<std::string> FilesInDir(const char* dir,const char* mask)
	{
		std::string s(dir);
		char c=dir[strlen(dir)-1];
		if(c!= '/' && c!='\\')
			s+='/';
		s+=mask;
		std::list<std::string> lst;
		struct _finddata_t c_file;
		long hFile;
		if( (hFile = _findfirst( s.c_str(), &c_file )) != -1L )
		{
			if(!(c_file.attrib & _A_SUBDIR))
				lst.push_back(c_file.name);
			while( _findnext( hFile, &c_file ) == 0 )
			{
				if(!(c_file.attrib & _A_SUBDIR))
					lst.push_back(c_file.name);
			}
			_findclose( hFile );
		}
		return lst;
	}
#endif
}

#endif