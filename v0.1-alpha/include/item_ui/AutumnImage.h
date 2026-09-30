#ifndef IMG_H
#define IMG_H

typedef struct {
    int w, h;
    uint32_t* buffer; // Tüm pikseller burada tek parça (flat) duracak
} AutumnImage;


#endif
