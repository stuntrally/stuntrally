#ifndef SH_GROUP_H
#define SH_GROUP_H

#include "MaterialInstance.hpp"

namespace sh
{
    /*
     * A group can contain any number of \a MaterialInstance objects. \n
     * This allows instances to be organized conveniently in groups (for example one group per game level or map). \n
     * You don't have to use groups at all. If you don't use them, all instances will be in a default group with an empty name.
     */
    struct Group
    {
        std::map <std::string, MaterialInstance> mInstances;
    };
}

#endif
