#ifndef _GAME_MOD_H
#define _GAME_MOD_H

namespace OpenNero
{

    /// @cond
    BOOST_PTR_DECL(SimContext);
    /// @endcond
    
    /// description structure for a game mod
    class Mod
    {
        std::vector<std::string>     mPath;     ///< Base Directory for the Mod
		std::string                  mUserPrefix; ///< directory with write access for storing user files

    public:
        SimContextPtr                context;  ///< Context for this mod
        std::string                  name;     ///< Name of the mod
        std::string                  mode;     ///< Mode of the mod
        
        /// default Mod constructor
        Mod();

        /// find resource by name (normally relative path)
        bool FindResource(const std::string& name, std::string& path);
        
        /// Get the location of the existing resource or a location where it can be saved
        bool FindUserResource(const std::string& name, std::string& path);
        
        /// set the path of this mod by parsing a colon-separated string
        void SetPath(std::string path);
        
        /// get the path of this mod (separated by ':')
        std::string GetPath();
    };

};

#endif /*_GAME_MOD_H_*/
