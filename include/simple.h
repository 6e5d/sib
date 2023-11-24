#include <stdbool.h>
#include <stdint.h>

#include "../../vwdlayer/include/ifdraw.h"
#include "../../simpleimg/include/simpleimg.h"
#include "../../dmgrect/include/dmgrect.h"

typedef struct {
	Simpleimg *canvas;
	// may have multiple draw calls in 1 round
	// then taken manually and reset to empty
	Dmgrect *pending;

	// config
	float spacing;
	float alpha_k;
	float alpha_b;
	float size_k;
	float size_b;
	uint8_t color[3]; // BGR
} SibSimple;

void sib_simple_config(SibSimple *sib);
VwdlayerIfdraw sib_simple_ifdraw(SibSimple *sib);
