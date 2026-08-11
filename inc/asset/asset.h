#ifndef ASSET_H
#define ASSET_H

typedef enum e_asset_type {
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_LEVEL,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_MUSIC,
    ASSET_TYPE_FONT,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_MODEL,
    ASSET_TYPE_ANIMATION,
    ASSET_TYPE_SCRIPT,
    ASSET_TYPE_UI
} t_asset_type;

typedef struct s_asset {
    unsigned int id;
    char *name;
    char *path;

    t_asset_type type;
} t_asset;

#endif