#include "core/Common.h"
#include "core/File.h"
#include "game/Mod.h"
#include <sstream>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#if NERO_PLATFORM_WINDOWS
	#include <windows.h>
	#ifdef FindResource
	#undef FindResource
	#endif
	#ifdef CreateDirectory
	#undef CreateDirectory
	#endif
#endif

namespace OpenNero
{
    using namespace std;

    namespace fs = boost::filesystem;

    /// default Mod constructor
    Mod::Mod() : mPath(), mUserPrefix(), context(), name(), mode()
    {
		// here we want to get a path that is a user-specific place where we can save files
#if NERO_PLATFORM_WINDOWS
		TCHAR profilepath[MAX_PATH];
		ExpandEnvironmentStrings("%LOCALAPPDATA%\\OpenNERO", profilepath, MAX_PATH);
		mUserPrefix = profilepath;
		if (mUserPrefix.empty() || mUserPrefix == "%LOCALAPPDATA%\\OpenNERO") {
			ExpandEnvironmentStrings("%USERPROFILE%\\Local Settings\\Application Data\\OpenNERO", profilepath, MAX_PATH);
			mUserPrefix = profilepath;
		}
#else // NERO_PLATFORM_WINDOWS
        char* pHome = getenv("HOME");
        if (pHome != NULL) {
            string homeDir(pHome);
            mUserPrefix = homeDir + "/.opennero";
        } else {
            mUserPrefix = ".";
        }
#endif // NERO_PLATFORM_WINDOWS
		if (!FileExists(mUserPrefix))
		{
			CreateDirectory(mUserPrefix);
		}
        mPath.push_back("."); // working directory
		mPath.push_back(mUserPrefix);
    }
    
    /// Find resource by name (normally mod-relative path).
    /// Check if the resource is in the path of the current mod.
    /// If it is, set path to its filesystem path. Otherwise, return false.
    /// The order of the path elements matters - the first match is used.
    /// @param name name of the resource (usually relative path)
    /// @param path variable to store the filesystem path to
    /// @return true iff the resource is found and path is set
    bool Mod::FindResource(const string& name, string& path)
    {
		if (FileExists(name)) {
			path = name;
			return true;
		}
        vector<string>::const_iterator path_elts;
        boost::filesystem::path cwd(boost::filesystem::current_path());
        for (path_elts = mPath.begin(); path_elts != mPath.end(); ++path_elts)
        {
            string path_element = *path_elts;
            string location = FilePathJoin(path_element,name);
            if (FileExists(location))
            {
                path = FilePathJoin(path_element, name);
                return true;
            }
        }
        return false;
    }

    /// Get the location of the existing resource or a location where it can be saved
    /// @param name name of the resource (usually mod-relative path)
    /// @param path variable to store the filesystem path to
    /// @return 
    bool Mod::FindUserResource(const std::string& name, std::string& path)
    {
        // first, try to find the system resource by this name
        if (FindResource(name, path))
        {
            return true;
        }
        else
        {
            path = FilePathJoin(mUserPrefix, name);
            return true;
        }
    }

    /// set the path of this mod by parsing a colon-separated string
    void Mod::SetPath(string path)
    {
        mPath.clear();
        string element;
        istringstream iss(path);
        while ( getline(iss, element, ':') )
        {
#if NERO_DEBUG
            fs::path cwd = fs::current_path();
            AssertMsg(FileExists(element), "Invalid path " << path << ", when looking at: " << element << " in cwd: " << cwd);
#endif // NERO_DEBUG
            mPath.push_back(element);
        }
    }
    
    /// get the path of this mod (separated by ':')
    std::string Mod::GetPath()
    {
        // TODO: add to test
        ostringstream ss;
        vector<string>::const_iterator iter = mPath.begin();
        if (iter != mPath.end())
        {
            ss << *iter;
        }
        ++iter;
        for (; iter != mPath.end(); ++iter)
        {
            ss << ":" << *iter;
        }
        return ss.str();
    }
}
