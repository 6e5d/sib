#ifndef INCLUDEGUARD_SIB
#define INCLUDEGUARD_SIB

#include "../../vwdlayer/include/vwdlayer.h"
#include "../../simpleimg/include/simpleimg.h"
#include "../../dmgrect/include/dmgrect.h"

typedef struct {
	void (*callback)(void *data);
	void *data;
	Simpleimg *canvas;
	// pending: 1 frame may have multiple segments
	// accum: 1 stroke may have many segments
	Dmgrect pending;
	// config
	float spacing;
	float alpha_k;
	float alpha_b;
	float size_k;
	float size_b;
	uint8_t color[3]; // BGR
} SibSimple;

void sib_simple_config(SibSimple *sib);
VwdlayerIfdraw sib_simple_ifdraw(void);

#endif
