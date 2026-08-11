#ifndef TEXTURE_H
#define TEXTURE_H

struct s_texture {
    unsigned char *data;
    unsigned int id;
    int width;
    int height;
    int channels;
} t_texture;



#endif