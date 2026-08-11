#ifndef LEVEL_H
#define LEVEL_H

#include "asset/texture.h"
#include "asset/asset.h"
#include "asset/artgroup.h"

struct s_level_vis {


} t_level_vis;

struct s_level_tree {


} t_level_tree;

struct s_level {
    unsigned int id;
    char *name;
    char *path;

    t_level_tree tree;
    t_level_vis vis;
    
    t_artgroup artgroup;
    t_asset* assets;

} t_level;

#endif